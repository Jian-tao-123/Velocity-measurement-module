#include "chassis_task.h"
#include "supercap.h"
#include "kinematics.h"
#include "referee_UI.h"

#define LimitMax(input, max)   \
    {                          \
        if (input > max)       \
        {                      \
            input = max;       \
        }                      \
        else if (input < -max) \
        {                      \
            input = -max;      \
        }                      \
    }


float pid_calc(pid_struct_t *pid, float ref, float fdb);

void pid_yinit (pid_struct_t *pid,float kp,float ki,float kd,float i_max,float out_max)
{
  pid->kp      = kp;
  pid->ki      = ki;
  pid->kd      = kd;
  pid->i_max   = i_max;
  pid->out_max = out_max;
}

pid_struct_t yaw_motor_pid;//pitch电机PID结构体



/**
  * @breif         底盘所有任务函数，实质上是定时器中断，1ms进入一次
  * @param[in]     none
	* @param[out]    none
  * @retval        none     
  */
float yaw_temp_voltage = 0;

void CHASSIS_TASK()
{
	static int time_count=1;
	time_count++;

	if(time_count%5==0)
	{

		yaw_temp_voltage = pid_calc(&yaw_motor_pid, 100,chassis_center.actual_speed);  //带入pid计算
		canTX_gimbal_y(yaw_temp_voltage);
	}
	if(time_count>=1000)			//清除计数标志    1s
	{time_count=1;
	}

}

float pid_calc(pid_struct_t *pid, float ref, float fdb)
{
  pid->ref = ref;
  pid->fdb = fdb;
	pid->err[2] = pid->err[1];
  pid->err[1] = pid->err[0];
  pid->err[0] = pid->ref - pid->fdb;
  
  pid->p_out  = pid->kp * pid->err[0];
  pid->i_out += pid->ki * pid->err[0];
	pid->Dbuf[2] = pid->Dbuf[1];
	pid->Dbuf[1] = pid->Dbuf[0];
	pid->Dbuf[0] = (pid->err[0] - pid->err[1]);
  pid->d_out  = pid->kd * pid->d_out;
	
	/*积分限幅*/
//  LIMIT_MIN_MAX(pid->i_out, -pid->i_max, pid->i_max);
  LimitMax(pid->i_out, pid->i_max);
  pid->output = pid->p_out + pid->i_out + pid->d_out;
	
	/*输出限幅*/
//  LIMIT_MIN_MAX(pid->output, -pid->out_max, pid->out_max);
	LimitMax(pid->output, pid->out_max);
  return pid->output;
}

