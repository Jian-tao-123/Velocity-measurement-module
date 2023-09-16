#include "my_init.h"

void mode_init(void)
{
	chassis_center.pid.loop_flag=POSITION_LOOP;
	chassis_motor1.pid.loop_flag=SPEED_LOOP;
	chassis_motor2.pid.loop_flag=SPEED_LOOP;
	chassis_motor3.pid.loop_flag=SPEED_LOOP;
	chassis_motor4.pid.loop_flag=SPEED_LOOP;
	
}
void all_init(void)
{
	motor_pid_init(&(chassis_center.pid));
	motor_pid_init(&(chassis_motor1.pid));
	motor_pid_init(&(chassis_motor2.pid));
	motor_pid_init(&(chassis_motor3.pid));
	motor_pid_init(&(chassis_motor4.pid));
	pid_init();
	POWER_PID_Init(&p_pid);
	BUFFER_PID_Init(&b_pid);
	referee_usart_fifo_init();
	remote_control_init();
	bsp_can_init();
	HAL_TIM_Base_Start_IT(&htim3);
	init_referee_struct_data();
}

