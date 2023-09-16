#include "chassis_move.h"
#include "fuzzy_pid.h"
#include "bsp_can.h"

static float chassis_follow(void);
static void chassis_speed_control(float speed_x, float speed_y, float speed_r);
static float chassis_power_loop(uint16_t target_power,float actual_power,float last_power);
static void chassis_move_mode(void);
static void can_send_chassis_current(void);
static void power_limitation_jugement(void);
static float chassis_buffer_loop(uint16_t buffer);
static void chassis_fly(uint16_t buffer);
static void speed_optimize(void);
static float Get_chassis_theta(void);

//static float Limit_Rhn(float num , float X);
CHASSIS_CONTROL_ORDER_t chassis_control_order;
MOTOR_t chassis_motor1,chassis_motor2,chassis_motor3,chassis_motor4,chassis_center;
POWER_PID_t p_pid;
BUFFER_PID_t b_pid;
REAl_CHASSIS_SPEED_t real_chassis_speed;
uint8_t fly_flag;
float max_d_speed_x=3.0f;
float max_d_speed_y=3.0f;
float max_d_speed_x_stop=7.0f;
float max_d_speed_y_stop=7.0f;
float avx,avy,awz;
float last_vx,last_vy;

//�µĹ��������õ��Ĳ���   Rhn
static float abs_f(float a);
static void Power_limit(void);
float Power_out_1=0;
float Power_out_2=0;
float Power_out_3=0;
float Power_out_4=0;
float Power_out=0;//Power_out֮��
float Power_out_square_sum=0;//Power_out��ƽ����
float Power_in_1=0;
float Power_in_2=0;
float Power_in_3=0;
float Power_in_4=0;
float Power_in=0;//Power_in֮��
float Target_current_square_sum=0;//Ŀ�������ƽ����
float Actual_speed_square_sum=0;//ʵ�ʽ��ٶȵ�ƽ����
float K_limit=0;//Ŀ�������Сϵ��

//float POWER_K2=0.0045;
float POWER_K1=2.8;
float POWER_K2=0.01;
float offset=0;

float Power_out_1_actual=0;//���ڵ���POWER_K1��POWER_K2
float Power_out_2_actual=0;
float Power_out_3_actual=0;
float Power_out_4_actual=0;
float Power_out_actual=0;
float Power_in_1_actual=0;
float Power_in_2_actual=0;
float Power_in_3_actual=0;
float Power_in_4_actual=0;
float Power_in_actual=0;

float reduction_ratio=19.203f;//���ӵ�����ٱ�
float chassis_radius=7.6f;//���Ӱ뾶    cm

extern uint16_t supercap_volt;  //�������ݵ�ѹ

float nrk=0;
/**
  * @breif         �����˶�����
  * @param[in]     none 
	* @param[out]    none
  * @retval        none     
  */
void chassis_move(void)
{
	//�Ż��ٶ�
//	speed_optimize();
	//ģʽѡ��
	chassis_move_mode();
		

	//pid����
	vpid_chassis_realize();
	chassis_motor1.target_current=chassis_motor1.pid.speed_loop.vpid.PID_OUT;
	chassis_motor2.target_current=chassis_motor2.pid.speed_loop.vpid.PID_OUT;
	chassis_motor3.target_current=chassis_motor3.pid.speed_loop.vpid.PID_OUT;
	chassis_motor4.target_current=chassis_motor4.pid.speed_loop.vpid.PID_OUT;
	//��������
	Power_limit();
	
	//���͵���
	can_send_chassis_current();
}
/**
  * @breif         ��ȡ��̨�����֮��ļн�
  * @param[in]     none
	* @param[out]    ��̨�����֮��ļн�(������)
  * @retval        none     
  */
static float Get_chassis_theta(void)
{
	float temp,temp2,angle;
	if(chassis_center.actual_angle<GIMBAL_HEAD_ANGLE)
		temp=chassis_center.actual_angle+360.0f;
	else temp=chassis_center.actual_angle;
	temp2=temp-GIMBAL_HEAD_ANGLE;	
	angle=temp2/360.0f*2*PI;
	return angle;
}
float theta; 

void chassis_spin(float *vx,float *vy) 
{					
	   
	theta=Get_chassis_theta(); 
	*vx = (float)(avy*sin(theta) + avx*cos(theta)); 
	*vy = (float)(avy*cos(theta) - avx*sin(theta));   
}
  

/**
  * @breif         ���̹�������
  * @param[in]     none 
	* @param[out]    ������ƺ���ĸ��������ֵ
  * @retval        none     
  */
float current_scale,BUFFER_MAX=60.0f,POWER_TOTAL_CURxRENT_LIMIT=9000.0f;
float temp3,temp1,temp2,speed1,speed2,speed3,speed4,total_current_limit,total_current,power,last_power;
float power_scale,buffer_scale;
uint16_t max_power,buffer;

static float abs_f(float a)
 {
	if(a<0)a=-1.0f*a;
	return a;
 }

static void Power_limit(void)//�µĹ�������
{
	get_chassis_power_and_buffer_and_max(&power,&buffer,&max_power);
	max_power=120;//����ϵͳ��ȡ��������ʱ�Լ���ֵ,����ʹ��ʱҪע�͵�
	Power_out_1=abs_f(((chassis_motor1.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*(K_M*chassis_motor1.target_current*20.0f/16384.0f));
	Power_out_2=abs_f(((chassis_motor2.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*(K_M*chassis_motor2.target_current*20.0f/16384.0f));
	Power_out_3=abs_f(((chassis_motor3.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*(K_M*chassis_motor3.target_current*20.0f/16384.0f));
	Power_out_4=abs_f(((chassis_motor4.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*(K_M*chassis_motor4.target_current*20.0f/16384.0f));
	Power_out=Power_out_1+Power_out_2+Power_out_3+Power_out_4;
	Power_out_square_sum=Power_out_1*Power_out_1+Power_out_2*Power_out_2+Power_out_3*Power_out_3+Power_out_4*Power_out_4;
	Power_in_1=Power_out_1+POWER_K1*(K_M*chassis_motor1.target_current*20.0f/16384.0f)*(K_M*chassis_motor1.target_current*20.0f/16384.0f)+POWER_K2*((chassis_motor1.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor1.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f);
	Power_in_2=Power_out_2+POWER_K1*(K_M*chassis_motor2.target_current*20.0f/16384.0f)*(K_M*chassis_motor2.target_current*20.0f/16384.0f)+POWER_K2*((chassis_motor2.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor2.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f);
	Power_in_3=Power_out_3+POWER_K1*(K_M*chassis_motor3.target_current*20.0f/16384.0f)*(K_M*chassis_motor3.target_current*20.0f/16384.0f)+POWER_K2*((chassis_motor3.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor3.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f);
	Power_in_4=Power_out_4+POWER_K1*(K_M*chassis_motor4.target_current*20.0f/16384.0f)*(K_M*chassis_motor4.target_current*20.0f/16384.0f)+POWER_K2*((chassis_motor4.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor4.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f);
	Target_current_square_sum=(K_M*chassis_motor1.target_current*20.0f/16384.0f)*(K_M*chassis_motor1.target_current*20.0f/16384.0f)+(K_M*chassis_motor2.target_current*20.0f/16384.0f)*(K_M*chassis_motor2.target_current*20.0f/16384.0f)+(K_M*chassis_motor3.target_current*20.0f/16384.0f)*(K_M*chassis_motor3.target_current*20.0f/16384.0f)+(K_M*chassis_motor4.target_current*20.0f/16384.0f)*(K_M*chassis_motor4.target_current*20.0f/16384.0f);
	Actual_speed_square_sum=((chassis_motor1.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor1.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)+((chassis_motor2.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor2.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)+((chassis_motor3.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor3.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)+((chassis_motor4.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor4.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f);
	Power_in=Power_in_1+Power_in_2+Power_in_3+Power_in_4;
	
	if(max_power<50)offset=10;
	else if(max_power>=50&&max_power<60)offset=10;
	else if(max_power>=60&&max_power<70)offset=10;
	else if(max_power>=70&&max_power<80)offset=10;
	else if(max_power>=80&&max_power<100)offset=13;
	else if(max_power>=80&&max_power<100)offset=13;
	else if(max_power>=100&&max_power<120)offset=15;
	else if(max_power>=120)offset=20;
	
if(Power_in>max_power && supercap_volt<=15) //���������ݵ�ѹ�����˾����ƹ������
	{
		if((Power_out*Power_out-4.0f*POWER_K1*(Target_current_square_sum)*(POWER_K2*(Actual_speed_square_sum)-max_power+offset))>=0)
		{
			K_limit=(-1.0f*Power_out+sqrt(Power_out*Power_out-4.0f*POWER_K1*(Target_current_square_sum)*(POWER_K2*(Actual_speed_square_sum)-max_power+offset)))/(2.0f*POWER_K1*Target_current_square_sum);
			if(K_limit>1)K_limit=1;
		}
		else 
		{
			K_limit=0;
		}
	}
	else 
	{
		K_limit=1.0f;
	}
		chassis_motor1.target_current*=K_limit;
		chassis_motor2.target_current*=K_limit;
		chassis_motor3.target_current*=K_limit;
		chassis_motor4.target_current*=K_limit;
	
//ʹ�õĵ���ֵ��Ϊʵ�ʵ���ֵ�����ڵ���POWER_K1��POWER_K2�����¼���Power_in_actual
//	Power_out_1_actual=abs_f(((chassis_motor1.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*(K_M*chassis_motor1.actual_current*20.0f/16384.0f));
//	Power_out_2_actual=abs_f(((chassis_motor2.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*(K_M*chassis_motor2.actual_current*20.0f/16384.0f));
//	Power_out_3_actual=abs_f(((chassis_motor3.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*(K_M*chassis_motor3.actual_current*20.0f/16384.0f));
//	Power_out_4_actual=abs_f(((chassis_motor4.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*(K_M*chassis_motor4.actual_current*20.0f/16384.0f));
//	Power_out_actual=Power_out_1_actual+Power_out_2_actual+Power_out_3_actual+Power_out_4_actual;
//	Power_in_1_actual=Power_out_1_actual+POWER_K1*(K_M*chassis_motor1.actual_current*20.0f/16384.0f)*(K_M*chassis_motor1.actual_current*20.0f/16384.0f)+POWER_K2*((chassis_motor1.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor1.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f);
//	Power_in_2_actual=Power_out_2_actual+POWER_K1*(K_M*chassis_motor2.actual_current*20.0f/16384.0f)*(K_M*chassis_motor2.actual_current*20.0f/16384.0f)+POWER_K2*((chassis_motor2.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor2.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f);
//	Power_in_3_actual=Power_out_3_actual+POWER_K1*(K_M*chassis_motor3.actual_current*20.0f/16384.0f)*(K_M*chassis_motor3.actual_current*20.0f/16384.0f)+POWER_K2*((chassis_motor3.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor3.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f);
//	Power_in_4_actual=Power_out_4_actual+POWER_K1*(K_M*chassis_motor4.actual_current*20.0f/16384.0f)*(K_M*chassis_motor4.actual_current*20.0f/16384.0f)+POWER_K2*((chassis_motor4.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f)*((chassis_motor4.actual_speed/reduction_ratio)*2.0f*3.1415927f/60.0f);
//	Power_in_actual=Power_in_1_actual+Power_in_2_actual+Power_in_3_actual+Power_in_4_actual;
}
/**
  * @breif         ���̹��ʻ�����
  * @param[in]     target_power���趨��Ŀ��ֵ
	* @param[in]     target_power�����ص���ʵֵ  
	* @param[in]     last_power����һ�η��ص���ʵֵ
	* @param[out]    �ĸ�������������
  * @retval        none     
  */


static float chassis_buffer_loop(uint16_t buffer)
{
	float temp;
	b_pid.target_buffer=50;
	b_pid.actual_buffer=buffer;
	buffer_pid_realize(&b_pid);
	temp=1.07-((float)b_pid.PID_OUT/1000.0f);
	temp*=0.85;
//	if(temp>1.2f)  temp-=0.2f;

	return temp;
}

/**
  * @breif         ���̷��º�������ֹ����º󻺳���������
  * @param[in]     buffer�����̻�������
	* @param[out]    �ĸ�������������
  * @retval        none     
  */
static void chassis_fly(uint16_t buffer)
{
	if(buffer<20)
	{
		chassis_motor1.pid.speed_loop.vpid.PID_OUT*=0.5f;
		chassis_motor2.pid.speed_loop.vpid.PID_OUT*=0.5f;
		chassis_motor3.pid.speed_loop.vpid.PID_OUT*=0.5f;
		chassis_motor4.pid.speed_loop.vpid.PID_OUT*=0.5f;
	}
}

/**
  * @breif         �����ٶ��Ż������ٶȱ仯��ƽ��
  * @param[in]     chassis_control_order.vx_set��x�����ٶ�
	* @param[in]     chassis_control_order.vy_set��y�����ٶ�
	* @param[out]    real_chassis_speed.real_vx���Ż���x�����ٶ�
	* @param[out]    real_chassis_speed.real_vy���Ż���y�����ٶ�
  * @retval        none     
  */
static void speed_optimize(void)
{
	static float last_xspeed,last_yspeed;
	if((chassis_control_order.vx_set!=0)||(chassis_control_order.vy_set!=0))
	{
		if(abs(chassis_control_order.vx_set-last_xspeed)>max_d_speed_x)
		{
			if(chassis_control_order.vx_set>last_xspeed)
				real_chassis_speed.real_vx+=max_d_speed_x;
			else if(chassis_control_order.vx_set<-last_xspeed)
				real_chassis_speed.real_vx-=max_d_speed_x;
		}
		else real_chassis_speed.real_vx=chassis_control_order.vx_set;

		if(abs(chassis_control_order.vy_set-last_yspeed)>max_d_speed_y)
		{
			if(chassis_control_order.vy_set>last_yspeed)
				real_chassis_speed.real_vy+=max_d_speed_y;
			else if(chassis_control_order.vy_set<-last_yspeed)
				real_chassis_speed.real_vy-=max_d_speed_y;
		}
		else real_chassis_speed.real_vy=chassis_control_order.vy_set;
		
		last_xspeed=real_chassis_speed.real_vx;
		last_yspeed=real_chassis_speed.real_vy;
	}
	else
	{

		if(real_chassis_speed.real_vx>0)
		{
			real_chassis_speed.real_vx-=max_d_speed_x_stop;
			if(real_chassis_speed.real_vx<0)
				real_chassis_speed.real_vx=0;
		}
		else 
		{
			real_chassis_speed.real_vx+=max_d_speed_x_stop;
			if(real_chassis_speed.real_vx>0)
				real_chassis_speed.real_vx=0;
		}
		
		if(real_chassis_speed.real_vy>0)
		{
			real_chassis_speed.real_vy-=max_d_speed_y_stop;
			if(real_chassis_speed.real_vy<0)
				real_chassis_speed.real_vy=0;
		}
		else 
		{
			real_chassis_speed.real_vy+=max_d_speed_y_stop;
			if(real_chassis_speed.real_vy>0)
				real_chassis_speed.real_vy=0;
		}
	}
		
	
//	chassis_control_order.vx_set=real_chassis_speed.real_vx;
//	chassis_control_order.vy_set=real_chassis_speed.real_vy;
}

/**
  * @breif         �˶�ѧ�ֽ⣬���������ĵ��ٶ�ת��Ϊ�ĸ����ӵ��ٶ�
  * @param[in]     speed_x��x�����ٶ�
	* @param[in]     speed_y��y�����ٶ�
	* @param[in]     speed_r����ת�ٶ�
	* @param[out]    �ĸ������Ŀ���ٶ�
  * @retval        none     
  */
static void chassis_speed_control(float speed_x, float speed_y, float speed_r)
{
	int max;
		//�ٶȻ��㣬�˶�ѧ�ֽ�
	BaseVel_To_WheelVel(speed_x, speed_y, speed_r);
	
	max=find_max();
	if(max>MAX_MOTOR_SPEED)
	{
		chassis_motor1.target_speed=(int)(chassis_motor1.target_speed*MAX_MOTOR_SPEED*1.0/max);
		chassis_motor2.target_speed=(int)(chassis_motor2.target_speed*MAX_MOTOR_SPEED*1.0/max);
		chassis_motor3.target_speed=(int)(chassis_motor3.target_speed*MAX_MOTOR_SPEED*1.0/max);
		chassis_motor4.target_speed=(int)(chassis_motor4.target_speed*MAX_MOTOR_SPEED*1.0/max);
	}
	set_chassis_speed(chassis_motor1.target_speed, chassis_motor2.target_speed, chassis_motor3.target_speed, chassis_motor4.target_speed);
}	

/**
  * @breif         ����ģʽ��ͨ���ǶȻ���Ŀ��Ƕ�ת��ΪĿ���ٶ�
  * @param[in]     none
	* @param[out]    ������ת�ٶ�
  * @retval        none     
  */
static float chassis_follow(void)
{
	//��̨ǹ�ڶ�Ӧ�ĽǶ�ֵ
	chassis_center.pid.position_loop.apid.target_angle=GIMBAL_HEAD_ANGLE;
	chassis_center.pid.position_loop.apid.actual_angle=chassis_center.actual_angle;
	follow_pid_realize();
//	if(fabs(chassis_center.pid.position_loop.apid.target_angle-chassis_center.pid.position_loop.apid.actual_angle)>0.5f)
				return (float)chassis_center.pid.position_loop.apid.PID_OUT;
//		return 0;6
}
STEPSTAR step_flag;
float K_VX,K_VY,B_VX,B_VY; //�ֱ����K��B һ�κ���
int step_times_x=0,step_times_y=0; //ʱ��
float TIME_LIMIT=270; //б�µ�ʱ�� 
int STEP_VALUE=50; //��ֵ����step_value����б��
//б�º���״̬�ж�
void step_flag_judge(float VX_,float VY_,float LAST_VX_,float LAST_VY_)
{
	if(step_flag==NO_STEP)
	{
		if(abs(VX_-LAST_VX_)>STEP_VALUE&&abs(VX_)>100) step_flag=X_STEP;
	    else if(abs(VY_-LAST_VY_)>STEP_VALUE&&abs(VY_)>90) step_flag=Y_STEP;
		return;
	}
	if(step_flag==X_STEP)
	{
		if(step_times_x>TIME_LIMIT)
	    {
		    step_times_x=0;
		    step_flag=NO_STEP;
			return;
	    }
		 if(abs(VX_)<=1.0f) 
	     {
		    step_flag=NO_STEP;
		    step_times_x=0;
			return;
	     }
		if(abs(VY_-LAST_VY_)>STEP_VALUE&&abs(VY_)>90) 
		{
			step_flag=XY_STEP;
		}
		return;
	}
	
	if(step_flag==Y_STEP)
	{
		if(step_times_y>TIME_LIMIT)
	    {
		    step_times_y=0;
		    step_flag=NO_STEP;
			return;
	    }
		 if(abs(VY_)<=1.0f) 
	     {
		    step_flag=NO_STEP;
		    step_times_y=0;
			return;
	     }
		if(abs(VX_-LAST_VX_)>STEP_VALUE&&abs(VX_)>90) 
		{
			step_flag=XY_STEP;
		}
		return;
	}
	
	if(step_flag==XY_STEP)
	{
		if(step_times_y>TIME_LIMIT &&step_times_x>TIME_LIMIT)
	    {
		    step_times_y=0;
			step_times_x=0;
		    step_flag=NO_STEP;
			return;
	    }
		if(step_times_x>TIME_LIMIT)
	    {
		    step_times_x=0;
		    step_flag=Y_STEP;
			return;
	    }
		if(step_times_y>TIME_LIMIT)
	    {
		    step_times_y=0;
		    step_flag=X_STEP;
			return;
	    }
		 if(abs(VY_)<=1.0f&&abs(VX_)<=1.0f) 
	     {
		    step_flag=NO_STEP;
		    step_times_y=0;
			 step_times_x=0;
			 return;
	     }
		 if(abs(VY_)<=1.0f) 
	     {
		    step_flag=X_STEP;
		    step_times_y=0;
	     }
		 if(abs(VX_)<=1.0f) 
	     {
		    step_flag=Y_STEP;
		    step_times_x=0;
	     }
		return;
	}
	
	
}

void step_star(float *VX_,float *VY_,float LAST_VX_,float LAST_VY_)
{
	step_flag_judge(*VX_,*VY_,LAST_VX_,LAST_VY_);

	if(step_flag==NO_STEP)  return;

	
	if(step_flag==X_STEP)
	{
		step_times_x++;
		if(step_times_x<=1)
		{
			K_VX=(*VX_-LAST_VX_)/TIME_LIMIT;
			B_VX=LAST_VX_;
		}
		
		*VX_=(float)(K_VX*(float)step_times_x)+B_VX;
		
	}
	if(step_flag==Y_STEP)
	{
		step_times_y++;
		if(step_times_y<=Y_STEP)
		{
			K_VY=(*VY_-LAST_VY_)/TIME_LIMIT;
			B_VY=LAST_VY_;
		}
		
		*VY_=K_VY*(float)step_times_y+B_VY;
	}
	if(step_flag==XY_STEP)
	{
		step_times_y++;
		if(step_times_y<=Y_STEP)
		{
			K_VY=(*VY_-LAST_VY_)/TIME_LIMIT;
			B_VY=LAST_VY_;
		}
		step_times_x++;
		if(step_times_x<=1)
		{
			K_VX=(*VX_-LAST_VX_)/TIME_LIMIT;
			B_VX=LAST_VX_;
		}
		
		*VX_=(float)(K_VX*(float)step_times_x)+B_VX;
		
		*VY_=K_VY*(float)step_times_y+B_VY;
	}
}

float vx,vy,wz;
float yu;
float wz_speed=-1.5;
/**
  * @breif         ѡ������˶�ģʽ
  * @param[in]     none
	* @param[out]    ��������������ٶ�
  * @retval        none     
  */
static void chassis_move_mode(void)
{
	
	vx=(float)chassis_control_order.vx_set;
	vy=(float)chassis_control_order.vy_set;
	wz=(float)chassis_control_order.wz_set;
	
	avx=vx;
	avy=vy;
	step_star(&avx,&avy,last_vx,last_vy);
	if(step_flag!=NO_STEP)
	{
		vx=avx;
	  vy=avy;
	}
	
//	CHASSIS_vPID_max=8000;
//	chassis_control_order.chassis_mode=CHASSIS_NORMAL;
	switch(chassis_control_order.chassis_mode)
	{
		case CHASSIS_NORMAL:
		break;
		case CHASSIS_NO_FORCE://����
		{   
			wz=0;
			vx=0;
			vy=0;
		}
		break;
		case CHASSIS_FOLLOW://�涯
		{			
			chassis_spin(&vx,&vy);
			wz=-1.0f*chassis_follow();
		}
		break;
		case CHASSIS_SPIN://С����
		{
			chassis_spin(&vx,&vy);
			if(turn_flag==0)
				wz=wz_speed;
			else
				wz=-2.0f;
		}
		break;
		case SENTRY_CONTROL:
		{
			chassis_spin(&vx,&vy);		
		}
		break;
		default:break;

	}
	last_vx=(float)chassis_control_order.vx_set;
	last_vy=(float)chassis_control_order.vy_set;
	chassis_speed_control(vx,vy,wz);
}

/**
  * @breif         �����ĸ�����ĵ���
  * @param[in]     none
	* @param[out]    �ĸ�����ĵ���ֵ
  * @retval        none     
  */
static void can_send_chassis_current(void)
{
	static uint8_t cdata[8];

	cdata[0]=(chassis_motor1.target_current)>>8;
	cdata[1]=(chassis_motor1.target_current)&0xFF;
	cdata[2]=(chassis_motor2.target_current)>>8;
	cdata[3]=(chassis_motor2.target_current)&0xFF;
	cdata[4]=(chassis_motor3.target_current)>>8;
	cdata[5]=(chassis_motor3.target_current)&0xFF;
	cdata[6]=(chassis_motor4.target_current)>>8;
	cdata[7]=(chassis_motor4.target_current)&0xFF;
	
	Can_Tx_Message(&hcan1,cdata);
}

/**
  * @breif         ���Ƶ��תһ��Ȧ��,����Ŀ�ĵغ��л�������ģʽ
  * @param[in]     none
	* @param[out]    �������
  * @retval        none     
  */
int8_t xrun_flag;
int8_t yrun_flag;

void chassis_control(void)
{
//x������5��Ϊ����
	if(xrun_flag==1)
	{
		if(chassis_motor1.total_angle<chassis_motor1.start_angle_x+8192*xrun_cnt) 					
		{
			chassis_control_order.vx_set=100;
			chassis_control_order.vy_set=0;
		}
	}
	
	else if(yrun_flag==1)
	{
		if(chassis_motor2.total_angle<chassis_motor2.start_angle_y+8192*yrun_cnt)
		{
			chassis_control_order.vx_set=0;
			chassis_control_order.vy_set=100;
		}
	}
		

}









