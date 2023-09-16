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
  float i_max;		//�����޷�
  float out_max;	//����޷�
  
  float ref;      // Ŀ��ֵ
  float fdb;      // ʵʱֵ
	float Dbuf[3];  //΢���� 0���� 1��һ�� 2���ϴ�
  float err[3];   // err[0]��ʾ����ƫ��,err[1]��ʾ�ϴ�ƫ��

  float p_out;		//kp��Ӧ�����(kp*����kp�ɱ���)
  float i_out;		//ki��Ӧ�����(��ki*���Ļ���)
  float d_out;		//kd��Ӧ�����(kd*���ı仯ֵ����kd�ɱ���)
  float output;
}pid_struct_t;

#define CHASSIS_TASK()    HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
void pid_yinit (pid_struct_t *pid,float kp,float ki,float kd,float i_max,float out_max);
extern pid_struct_t yaw_motor_pid;//pitch���PID�ṹ��

#endif 


