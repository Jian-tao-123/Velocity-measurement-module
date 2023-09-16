#include "bsp_can.h"
#include "referee.h"
#include "chassis_move.h"

static int16_t* read_motor_data(uint8_t *rxdata);
static void get_motor_data(MOTOR_t *motor, uint16_t angle, int16_t speed, int16_t current);
static void record_chassis_callback(MOTOR_t *motor, uint16_t angle, int16_t current,int16_t speed);
static void canTX_gimbal(int8_t commd_keyboard, int16_t heat0, int16_t heat0_limit,uint8_t level,uint16_t enemy_outpost_blood);
void canTX_gimbal_2(int16_t angle,int16_t speed,int16_t actual_vx);
void canTX_gimbal_3(uint8_t game_process,uint16_t time,uint16_t outpost_blood,uint16_t self_blood,uint8_t shot_flag);
void canTX_gimbal_4(float robot_x,float robot_y,float target_position_x,float target_position_y);
void canTX_gimbal_5(float target_position_x,float target_position_y);

uint16_t enemy_outpost_blood;
uint16_t heat0,heat0_limit;
float target_position_x,target_position_y;
uint8_t commd_keyboard;
uint8_t level,foul_robot_id;
uint16_t time,outpost_blood,self_blood;
uint8_t game_process,shot_flag;
float robot_x, robot_y,robot_z,robot_yaw;
uint8_t bullet_freq;
float	bullet_speed;
uint8_t	bullet_speedlimit;


int8_t turn_flag; //控制小陀螺旋转标志位，0为快，1为慢

//--------------------3.21-------------//
//static void canTX_gimbal(int8_t game_process, int16_t time, int16_t outpost_blood ,int16_t self_blood, uint8_t shot_flag,uint8_t heat0,uint8_t heat0_limit);
//--------------------3.21-------------//
/**
  * @breif         can通信初始化
  * @param[in]     none
	* @param[out]    none
  * @retval        none     
  */
uint8_t bsp_can_init(void)
{
	uint8_t status=0;
	CAN_FilterTypeDef canFilter;
	
	
	canFilter.FilterBank=1;    																//筛选器组1
	canFilter.FilterIdHigh=0;
	canFilter.FilterIdLow=0;
	canFilter.FilterMaskIdHigh=0;
	canFilter.FilterMaskIdLow=0;
	canFilter.FilterMode=CAN_FILTERMODE_IDMASK;  							//掩码模式
	canFilter.FilterActivation=CAN_FILTER_ENABLE;							//开启
	canFilter.FilterScale=CAN_FILTERSCALE_32BIT; 							//32位模式
	canFilter.FilterFIFOAssignment=CAN_FILTER_FIFO0; 					//链接到fifo0
	canFilter.SlaveStartFilterBank=14;												//can2筛选组起始编号
	
	status=HAL_CAN_ConfigFilter(&hcan1,&canFilter);					//配置过滤器
	
	canFilter.FilterBank=15;    															//筛选器组15
	status=HAL_CAN_ConfigFilter(&hcan2,&canFilter);					//配置过滤器
	
	/*离开初始模式*/
	HAL_CAN_Start(&hcan1);				
	HAL_CAN_Start(&hcan2);
	
	
	/*开中断*/
	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);       //can1 接收fifo 0不为空中断
	HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);       //can2 接收fifo 0不为空中断
	return status;
}


/**
  * @breif         can发送函数
  * @param[in]     hcan：can的句柄结构体
	* @param[in]     mdata：需要发送的数组
	* @param[out]    none
  * @retval        status：发送的状态    
  */
uint8_t Can_Tx_Message(CAN_HandleTypeDef *hcan,uint8_t *mdata)
{
	uint8_t status;
	CAN_TxHeaderTypeDef CAN_TxHeaderStruct;
	uint32_t  pTxMailbox;
	if(hcan==&hcan1)
	{
		CAN_TxHeaderStruct.StdId=0x200;
		CAN_TxHeaderStruct.ExtId=0;
		CAN_TxHeaderStruct.DLC=8;
		CAN_TxHeaderStruct.IDE=CAN_ID_STD;
		CAN_TxHeaderStruct.RTR=CAN_RTR_DATA;
		CAN_TxHeaderStruct.TransmitGlobalTime=DISABLE;
	}
	else if(hcan==&hcan2)
	{
		CAN_TxHeaderStruct.StdId=0x006;
		CAN_TxHeaderStruct.ExtId=0;
		CAN_TxHeaderStruct.DLC=8;
		CAN_TxHeaderStruct.IDE=CAN_ID_STD;
		CAN_TxHeaderStruct.RTR=CAN_RTR_DATA;
		CAN_TxHeaderStruct.TransmitGlobalTime=DISABLE;
	}
	status=HAL_CAN_AddTxMessage(hcan,&CAN_TxHeaderStruct,mdata,&pTxMailbox);
	return status;
}
//CAN_GIMBAL_Y_ID
uint8_t canTX_gimbal_y(int16_t yaw)
{
	CAN_TxHeaderTypeDef canFrame;
	uint8_t data[8]={0};
	uint32_t temp=0;
	
	canFrame.IDE=CAN_ID_STD;
	canFrame.StdId=0x2ff;
	canFrame.RTR=CAN_RTR_DATA;
	canFrame.DLC=8;
	canFrame.TransmitGlobalTime=DISABLE;
	data[6]=0;
	data[7]=0;
	data[2]=0;
	data[3]=0;
	data[0]=yaw>>8;
	data[1]=yaw&0xff;
	data[4]=0;
	data[5]=0;
	HAL_CAN_AddTxMessage(&hcan1, &canFrame, data,&temp);
	
	return temp;
}

/**
  * @breif         can接收中断函数
  * @param[in]     hcan：can的句柄结构体
	* @param[out]    can1接收四个电机返回的电流值，can2接收上板传来的数据
  * @retval        none   
  */
int8_t x_start_flag=0;
int8_t y_start_flag=0;
int YAW_SPEED;
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef CAN_RxHeaderStruct;
	uint8_t rxdata[8];
	int16_t speed,speed2,*gdata,current;
	float angle;
	if(hcan==&hcan1)
	{
		HAL_CAN_GetRxMessage(&hcan1,CAN_RX_FIFO0,&CAN_RxHeaderStruct,rxdata);
		gdata=read_motor_data(rxdata);
		angle=gdata[0];
		speed=gdata[1];
		current=gdata[2];
		speed2=gdata[1];
		switch(CAN_RxHeaderStruct.StdId)
		{
			case CAN_3508Motor1_ID:
				get_motor_data(&chassis_motor1,angle,speed,current);
			break;
			case CAN_3508Motor2_ID:
				get_motor_data(&chassis_motor2,angle,speed,current);
			break;
			case CAN_3508Motor3_ID:
				get_motor_data(&chassis_motor3,angle,speed,current);
			break;
			case CAN_3508Motor4_ID:
				get_motor_data(&chassis_motor4,angle,speed,current);
			break;
			case CAN_GIMBAL_Y_ID:
				record_chassis_callback(&chassis_center,angle,current,speed2);
				YAW_SPEED=gdata[1];
			break;
			case LOOP_BACK_ID:
				get_motor_data(&chassis_motor1,angle,speed,current);
			default:break;
		}
	}
	else if(hcan==&hcan2)
	{
		if(HAL_CAN_GetRxMessage(&hcan2,CAN_RX_FIFO0,&CAN_RxHeaderStruct,rxdata)!=HAL_OK)
			chassis_control_order.chassis_mode=CHASSIS_NO_FORCE;
		gdata=read_motor_data(rxdata);
		if(CAN_RxHeaderStruct.StdId==GIMBAL_CONTROL_ID_1)
		{
			chassis_data_analyze();
			chassis_control_order.vx_set*=-1;
			chassis_control_order.vy_set*=-1;
		}
		if(CAN_RxHeaderStruct.StdId==GIMBAL_CONTROL_ID_2)	
		{
			chassis_control_order.chassis_mode=gdata[0];//mode
			turn_flag=(int8_t)rxdata[4];
			xrun_flag=(int8_t)rxdata[5];
			yrun_flag=(int8_t)rxdata[6];
			if((chassis_control_order.chassis_mode==CHASSIS_FOLLOW)&&(chassis_control_order.last_chassis_mode==CHASSIS_SPIN))     
				chassis_center.switch_mode_flag=SPIN_TO_FOLLOW; 
			chassis_control_order.last_chassis_mode=chassis_control_order.chassis_mode; 
			if(chassis_control_order.chassis_mode==CHASSIS_SPIN)
			{
				if((chassis_center.actual_angle>5)&&(chassis_center.actual_angle<360))
				{
					if(chassis_center.actual_angle-chassis_control_order.last_gimbal_6020_angle>0)
						chassis_center.spin_dirt=CLOCKWISE;
					else if(chassis_center.actual_angle-chassis_control_order.last_gimbal_6020_angle<0)
						chassis_center.spin_dirt=ANTICLOCKWISE;
				}
			}
			if(xrun_flag==1 || yrun_flag==1)
			{
				
				if(xrun_flag )
				{
					if(x_start_flag==0)
					{
					chassis_motor1.start_angle_x=chassis_motor1.total_angle;
					chassis_motor2.start_angle_x=chassis_motor2.total_angle;
					chassis_motor3.start_angle_x=chassis_motor3.total_angle;
					chassis_motor4.start_angle_x=chassis_motor4.total_angle;
						x_start_flag++;
					}
				}
				else if(yrun_flag)
				{
					if(y_start_flag==0)
					{
					chassis_motor1.start_angle_y=chassis_motor1.total_angle;
					chassis_motor2.start_angle_y=chassis_motor2.total_angle;
					chassis_motor3.start_angle_y=chassis_motor3.total_angle;
					chassis_motor4.start_angle_y=chassis_motor4.total_angle;
						y_start_flag++;
					}
				}
//				chassis_control_order.chassis_mode=SENTRY_CONTROL;
				chassis_control();
		}
		
		}
		
	}
	
}

/**
  * @breif         can整合电机数据函数
  * @param[in]     rxdata：电机返回的数组
	* @param[out]    整合好的数组
  * @retval        adata：整合好的数组  
  */
int16_t adata[4];
static int16_t* read_motor_data(uint8_t *rxdata)
{
		adata[0]=(int16_t)((rxdata[0]<<8)|rxdata[1]);
		adata[1]=(int16_t)((rxdata[2]<<8)|rxdata[3]);
		adata[2]=(int16_t)((rxdata[4]<<8)|rxdata[5]);
		adata[3]=(int16_t)((rxdata[6]<<8)|rxdata[7]);
	return adata;
}

/**
  * @breif         can解析电机数据函数
  * @param[in]     motor：电机参数结构体
  * @param[in]     angle：电机角度值
  * @param[in]     speed：电机速度值
  * @param[in]     current：电机电流值
	* @param[out]    将输入的值赋值给电机的结构体成员
  * @retval        none   
  */
static void get_motor_data(MOTOR_t *motor, uint16_t angle, int16_t speed, int16_t current)
{
	motor->last_angle = motor->actual_angle;
	motor->actual_angle = angle;
	motor->pid.position_loop.apid.actual_angle=angle;
	motor->actual_speed = 0.5*(speed + motor->last_speed);
	motor->pid.position_loop.vpid.actual_speed=motor->actual_speed;
	motor->pid.speed_loop.vpid.actual_speed=motor->actual_speed;
	//motor->actual_speed = KalmanFilter(speed,Q,R);
	motor->last_speed = speed;
	motor->actual_current = current;
	//motor1.temp = temp;
	if(motor->start_angle_flag==0)
	{
		motor->start_angle = angle;
		motor->start_angle_flag++;	//只在启动时记录一次初始角度
	}
	
	if(motor->actual_angle - motor->last_angle > 4096)
		motor->round_cnt--;
	else if (motor->actual_angle - motor->last_angle < -4096)
		motor->round_cnt++;
	motor->total_angle = motor->round_cnt * 8192 + motor->actual_angle;// - motor->start_angle;
}

static void record_chassis_callback(MOTOR_t *motor, uint16_t angle, int16_t current,int16_t speed)
{
	motor->last_angle = motor->actual_angle;
	motor->actual_angle = (float)angle/8191.0f;
	motor->actual_angle*=360.0f;
	motor->actual_current = current;
	motor->actual_speed=0.5*(speed + motor->last_speed);;
	if(motor->start_angle_flag==0)
	{
		motor->start_angle = (float)angle/8191.0f*360.0f;
		motor->start_angle_flag++;	//只在启动时记录一次初始角度
	}
	motor->last_speed = speed;
}

//------------3.21----------------//

/**
  * @breif         can发送上板函数
  * @param[in]     game_process：当前比赛阶段
  * @param[in]     time：比赛剩余时间
  * @param[in]     heat0_limit：前哨站血量
  * @param[in]     bullet_speed：自身血量
  * @param[in]     bullet_speedlimit：是否被击打
	* @param[out]    none
  * @retval        none   
  */
	//----------------3.15-------------//
static void canTX_gimbal(int8_t commd_keyboard, int16_t heat0, int16_t heat0_limit,uint8_t level,uint16_t enemy_outpost_blood)
{
	uint8_t data[8];
	uint8_t status;
	CAN_TxHeaderTypeDef CAN_TxHeaderStruct;
	uint32_t  pTxMailbox;

	CAN_TxHeaderStruct.StdId=GIMBAL_CONTROL_ID_1;
	CAN_TxHeaderStruct.ExtId=0;
	CAN_TxHeaderStruct.DLC=8;
	CAN_TxHeaderStruct.IDE=CAN_ID_STD;
	CAN_TxHeaderStruct.RTR=CAN_RTR_DATA;
	CAN_TxHeaderStruct.TransmitGlobalTime=DISABLE;
	
	data[0]= commd_keyboard;
	data[1]=heat0>>8;
	data[2]=heat0&0xff;
	data[3]=heat0_limit>>8;
	data[4]=heat0_limit&0xff;
	data[5]=level; 
	data[6]=enemy_outpost_blood>>8;
	data[7]=enemy_outpost_blood&0xff;
	
	status=HAL_CAN_AddTxMessage(&hcan2,&CAN_TxHeaderStruct,data,&pTxMailbox);
}
//----------------3.15-------------//

void canTX_gimbal_2(int16_t angle,int16_t speed,int16_t actual_vx)
{
	uint8_t data[8];
	uint8_t status;
	CAN_TxHeaderTypeDef CAN_TxHeaderStruct;
	uint32_t  pTxMailbox;
	
	CAN_TxHeaderStruct.StdId=CAN_GIMBAL_Y_ID;
	CAN_TxHeaderStruct.ExtId=0;
	CAN_TxHeaderStruct.DLC=8;
	CAN_TxHeaderStruct.IDE=CAN_ID_STD;
	CAN_TxHeaderStruct.RTR=CAN_RTR_DATA;
	CAN_TxHeaderStruct.TransmitGlobalTime=DISABLE;
	
	data[0]=angle>>8;
	data[1]=angle&0xff;
	data[2]=speed>>8;
	data[3]=speed&0xff;
	data[4]=actual_vx>>8;
	data[5]=actual_vx&0xff;
	status=HAL_CAN_AddTxMessage(&hcan2,&CAN_TxHeaderStruct,data,&pTxMailbox);
}

void canTX_gimbal_3(uint8_t game_process,uint16_t time,uint16_t outpost_blood,uint16_t self_blood,uint8_t shot_flag)
{
	uint8_t data[8];
	uint8_t status;
	CAN_TxHeaderTypeDef CAN_TxHeaderStruct;
	uint32_t  pTxMailbox;
	
	CAN_TxHeaderStruct.StdId=GIMBAL_CONTROL_ID_3;
	CAN_TxHeaderStruct.ExtId=0;
	CAN_TxHeaderStruct.DLC=8;
	CAN_TxHeaderStruct.IDE=CAN_ID_STD;
	CAN_TxHeaderStruct.RTR=CAN_RTR_DATA;
	CAN_TxHeaderStruct.TransmitGlobalTime=DISABLE;
	
	data[0]=game_process;
	data[1]=time>>8;
	data[2]=time&0xff;
	data[3]=outpost_blood>>8;
	data[4]=outpost_blood&0xff;
	data[5]=self_blood>>8;
	data[6]=self_blood&0xff;
	data[7]=shot_flag;
	status=HAL_CAN_AddTxMessage(&hcan2,&CAN_TxHeaderStruct,data,&pTxMailbox);
}

void canTX_gimbal_4(float robot_x,float robot_y,float target_position_x,float target_position_y)
{
	uint8_t data[8];
	uint8_t status;
	CAN_TxHeaderTypeDef CAN_TxHeaderStruct;
	uint32_t  pTxMailbox;
	
	CAN_TxHeaderStruct.StdId=GIMBAL_CONTROL_ID_4;
	CAN_TxHeaderStruct.ExtId=0;
	CAN_TxHeaderStruct.DLC=8;
	CAN_TxHeaderStruct.IDE=CAN_ID_STD;
	CAN_TxHeaderStruct.RTR=CAN_RTR_DATA;
	CAN_TxHeaderStruct.TransmitGlobalTime=DISABLE;
	
	data[0]=((uint16_t)(robot_x*100.0f))>>8;
	data[1]=((uint16_t)(robot_x*100.0f))&0xff;
	data[2]=((uint16_t)(robot_y*100.0f))>>8;
	data[3]=((uint16_t)(robot_y*100.0f))&0xff;
	data[4]=((uint16_t)(target_position_x*100.0f))>>8;
	data[5]=((uint16_t)(target_position_x*100.0f))&0xff;
	data[6]=((uint16_t)(target_position_y*100.0f))>>8;
	data[7]=((uint16_t)(target_position_y*100.0f))&0xff;
	status=HAL_CAN_AddTxMessage(&hcan2,&CAN_TxHeaderStruct,data,&pTxMailbox);
}

void canTX_gimbal_5(float bullet_speed,float bullet_speedlimit)
{
	uint8_t data[8];
	uint8_t status;
	CAN_TxHeaderTypeDef CAN_TxHeaderStruct;
	uint32_t  pTxMailbox;
	
	CAN_TxHeaderStruct.StdId=GIMBAL_CONTROL_ID_5;
	CAN_TxHeaderStruct.ExtId=0;
	CAN_TxHeaderStruct.DLC=8;
	CAN_TxHeaderStruct.IDE=CAN_ID_STD;
	CAN_TxHeaderStruct.RTR=CAN_RTR_DATA;
	CAN_TxHeaderStruct.TransmitGlobalTime=DISABLE;
	
	data[0]=((uint16_t)(bullet_speed*10.0f))>>8;
	data[1]=((uint16_t)(bullet_speed*10.0f))&0xff;
	data[2]=bullet_speedlimit;
	status=HAL_CAN_AddTxMessage(&hcan2,&CAN_TxHeaderStruct,data,&pTxMailbox);

}


void send_gimbal_data(void)
{
	get_self_own_side_blood(&outpost_blood,&self_blood,&enemy_outpost_blood);
	get_referee_warning(&level,&foul_robot_id);
	get_shoot_heat0_limit_and_heat0(&heat0_limit, &heat0);
	get_robot_target_position(&target_position_x,&target_position_y,&commd_keyboard);
	canTX_gimbal(commd_keyboard,heat0, heat0_limit,level,enemy_outpost_blood);
}

void send_gimbal_data_2(void)
{
	int16_t angle,speed;
	angle=(int16_t)chassis_center.actual_angle;
	speed=(int16_t)chassis_center.actual_speed;

	canTX_gimbal_2(angle,speed,(int16_t)Kinematics.actual_velocities.linear_x);
}

void send_gimbal_data_3(void)
{
	get_game_process_get_time(&game_process,	&time);
	get_self_own_side_blood(&outpost_blood,&self_blood,&enemy_outpost_blood);
	get_robot_hurt(&shot_flag);
	
	canTX_gimbal_3(game_process,time,outpost_blood,self_blood,shot_flag);
}


void send_gimbal_data_4(void)
{
	get_robot_target_position(&target_position_x,&target_position_y,&commd_keyboard);
	get_robot_position(&robot_x,&robot_y,&robot_z,&robot_yaw);
	canTX_gimbal_4(robot_x,robot_y,target_position_x,target_position_y);
}
void send_gimbal_data_5(void)
{
	get_shoot_data(&bullet_freq,	&bullet_speed, &bullet_speedlimit);
	canTX_gimbal_5(bullet_speed,bullet_speedlimit);	
}


void canTX_gimbal_referee(int16_t vx,int16_t vy)
{
	uint8_t data[8];
	uint8_t status;
	CAN_TxHeaderTypeDef CAN_TxHeaderStruct;
	uint32_t  pTxMailbox;
	
	CAN_TxHeaderStruct.StdId=CAN_GIMBAL_Y_ID;
	CAN_TxHeaderStruct.ExtId=0;
	CAN_TxHeaderStruct.DLC=8;
	CAN_TxHeaderStruct.IDE=CAN_ID_STD;
	CAN_TxHeaderStruct.RTR=CAN_RTR_DATA;
	CAN_TxHeaderStruct.TransmitGlobalTime=DISABLE;
	
	data[0]=vx>>8;
	data[1]=vx&0xff;
	data[0]=vy>>8;
	data[1]=vy&0xff;
	status=HAL_CAN_AddTxMessage(&hcan2,&CAN_TxHeaderStruct,data,&pTxMailbox);
}
