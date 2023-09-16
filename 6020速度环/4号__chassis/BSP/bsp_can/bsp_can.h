#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "can.h"
#include "chassis_move.h"
#include "referee.h"

#define SPIN_TO_FOLLOW        1
#define MODE_NO_SWITCH     0


#define chassis_data_analyze() \
do{ \
		chassis_control_order.vx_set=gdata[0]; \
    chassis_control_order.vy_set=gdata[1]; \
		chassis_control_order.wz_set=gdata[2];	\
    chassis_center.target_current=gdata[3];\
	}while(0)         

	
#define CAN_3508Motor1_ID    0x201
#define CAN_3508Motor2_ID    0x202
#define CAN_3508Motor3_ID    0x203
#define CAN_3508Motor4_ID    0x204
#define CAN_GIMBAL_Y_ID      0x209
#define GIMBAL_CONTROL_ID_1  0x007
#define GIMBAL_CONTROL_ID_2  0x006
#define GIMBAL_CONTROL_ID_3  0x008
#define GIMBAL_CONTROL_ID_4  0x00A
#define GIMBAL_CONTROL_ID_5  0x00B
	
#define LOOP_BACK_ID         0x003
#define CHASSIS_TO_GIMBAL_ID_2         0x009
uint8_t bsp_can_init(void);
uint8_t Can_Tx_Message(CAN_HandleTypeDef *hcan,uint8_t *mdata);
void send_gimbal_data(void);	
void send_gimbal_data_2(void);
void send_gimbal_data_3(void);
void send_gimbal_data_4(void);
void send_gimbal_data_5(void);

uint8_t canTX_gimbal_y(int16_t yaw);
extern int8_t turn_flag; 
#endif

