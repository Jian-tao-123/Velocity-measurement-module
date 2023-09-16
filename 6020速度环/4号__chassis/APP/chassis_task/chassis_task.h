#ifndef __CHASSIS_TASK_H
#define __CHASSIS_TASK_H

#include "chassis_move.h"
#include "tim.h"
#include "remote_control.h"
#include "referee.h"
#include "bsp_can.h"
#include "referee_UI.h"
typedef struct _pid_struct_t
{
  float kp;
  float ki;
  float kd;
  float i_max;		//积分限幅
  float out_max;	//输出限幅
  
  float ref;      // 目标值
  float fdb;      // 实时值
	float Dbuf[3];  //微分项 0最新 1上一次 2上上次
  float err[3];   // err[0]表示最新偏差,err[1]表示上次偏差

  float p_out;		//kp对应的输出(kp*误差，与kp成比例)
  float i_out;		//ki对应的输出(对ki*误差的积分)
  float d_out;		//kd对应的输出(kd*误差的变化值，与kd成比例)
  float output;
}pid_struct_t;

#define CHASSIS_TASK()    HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
void pid_yinit (pid_struct_t *pid,float kp,float ki,float kd,float i_max,float out_max);
extern pid_struct_t yaw_motor_pid;//pitch电机PID结构体

#endif 


