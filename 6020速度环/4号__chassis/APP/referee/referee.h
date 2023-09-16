#ifndef REFEREE_H
#define REFEREE_H

#include "main.h"

#include "protocol.h"

typedef enum
{
    RED_HERO        = 1,
    RED_ENGINEER    = 2,
    RED_STANDARD_1  = 3,
    RED_STANDARD_2  = 4,
    RED_STANDARD_3  = 5,
    RED_AERIAL      = 6,
    RED_SENTRY      = 7,
    BLUE_HERO       = 11,
    BLUE_ENGINEER   = 12,
    BLUE_STANDARD_1 = 13,
    BLUE_STANDARD_2 = 14,
    BLUE_STANDARD_3 = 15,
    BLUE_AERIAL     = 16,
    BLUE_SENTRY     = 17,
} robot_id_t;

typedef enum
{
    PROGRESS_UNSTART        = 0,
    PROGRESS_PREPARE        = 1,
    PROGRESS_SELFCHECK      = 2,
    PROGRESS_5sCOUNTDOWN    = 3,
    PROGRESS_BATTLE         = 4,
    PROGRESS_CALCULATING    = 5,
} game_progress_t;

typedef __packed struct //0x0001
{
 uint8_t game_type : 4;
 uint8_t game_progress : 4;
 uint16_t stage_remain_time;
uint64_t SyncTimeStamp;
}ext_game_status_t;

typedef __packed struct //0x0002
{
 uint8_t winner;
}ext_game_result_t;

typedef __packed struct //0x0003
{
	uint16_t red_1_robot_HP;
	uint16_t red_2_robot_HP;
	uint16_t red_3_robot_HP;
	uint16_t red_4_robot_HP;
	uint16_t red_5_robot_HP;
	uint16_t red_7_robot_HP;
	uint16_t red_outpost_HP;
	uint16_t red_base_HP;
	uint16_t blue_1_robot_HP;
	uint16_t blue_2_robot_HP;
	uint16_t blue_3_robot_HP;
	uint16_t blue_4_robot_HP;
	uint16_t blue_5_robot_HP;
	uint16_t blue_7_robot_HP;
	uint16_t blue_outpost_HP;
	uint16_t blue_base_HP;
}ext_game_robot_HP_t;

typedef __packed struct//0x0005人工智能挑战赛加成、惩罚区分布与潜伏模式状态
{
 uint8_t F1_zone_status:1;
 uint8_t F1_zone_buff_debuff_status:3; 
 uint8_t F2_zone_status:1;
 uint8_t F2_zone_buff_debuff_status:3; 
 uint8_t F3_zone_status:1;
 uint8_t F3_zone_buff_debuff_status:3; 
 uint8_t F4_zone_status:1;
 uint8_t F4_zone_buff_debuff_status:3; 
 uint8_t F5_zone_status:1;
 uint8_t F5_zone_buff_debuff_status:3; 
 uint8_t F6_zone_status:1;
 uint8_t F6_zone_buff_debuff_status:3;
 uint16_t red1_bullet_left;
 uint16_t red2_bullet_left;
uint16_t blue1_bullet_left;
uint16_t blue2_bullet_left;
uint8_t lurk_mode;
uint8_t res;
} ext_ICRA_buff_debuff_zone_and_lurk_status_t;

typedef __packed struct//0x0101场地事件数据
{
 uint32_t event_type;
} ext_event_data_t;

typedef __packed struct //0x0102补给站动作标识
{
    uint8_t supply_projectile_id;
    uint8_t supply_robot_id;
    uint8_t supply_projectile_step;
    uint8_t supply_projectile_num;
}ext_supply_projectile_action_t;




typedef __packed struct //0x0104裁判警告信息
{
    uint8_t level;
    uint8_t foul_robot_id;
}ext_referee_warning_t;

typedef __packed struct//0x0105飞镖发射口倒计时
{
 uint8_t dart_remaining_time;
} ext_dart_remaining_time_t;

typedef __packed struct //0x0201比赛机器人状态
{
	uint8_t robot_id;
	uint8_t robot_level;
	uint16_t remain_HP;
	uint16_t max_HP;
	uint16_t shooter_id1_17mm_cooling_rate;
	uint16_t shooter_id1_17mm_cooling_limit;
	uint16_t shooter_id1_17mm_speed_limit;
	uint16_t shooter_id2_17mm_cooling_rate;
	uint16_t shooter_id2_17mm_cooling_limit;
	uint16_t shooter_id2_17mm_speed_limit;
	uint16_t shooter_id1_42mm_cooling_rate;
	uint16_t shooter_id1_42mm_cooling_limit;
	uint16_t shooter_id1_42mm_speed_limit;
	uint16_t chassis_power_limit;
	uint8_t mains_power_gimbal_output : 1;
	uint8_t mains_power_chassis_output : 1;
	uint8_t mains_power_shooter_output : 1;
}ext_game_robot_status_t;

typedef __packed struct //0x0202实时功率热量数据
{
	uint16_t chassis_volt;
	uint16_t chassis_current;
	float chassis_power;
	uint16_t chassis_power_buffer;
	uint16_t shooter_id1_17mm_cooling_heat;
	uint16_t shooter_id2_17mm_cooling_heat;
	uint16_t shooter_id1_42mm_cooling_heat;
}ext_power_heat_data_t;


typedef __packed struct //0x0203机器人位置
{
    float x;
    float y;
    float z;
    float yaw;
}ext_game_robot_pos_t;

typedef __packed struct //0x0204机器人增益
{
    uint8_t power_rune_buff;
}ext_buff_musk_t;

typedef __packed struct//0x0205空中机器人能量状态
{
 uint8_t attack_time;
} aerial_robot_energy_t;

typedef __packed struct //0x0206伤害状态
{
    uint8_t armor_id : 4;
    uint8_t hurt_type : 4;
} ext_robot_hurt_t;

typedef __packed struct //0x0207实时射击信息
{
	uint8_t bullet_type;
	uint8_t shooter_id;
	uint8_t bullet_freq;
	float bullet_speed;
} ext_shoot_data_t;

typedef __packed struct //0x0208子弹剩余发射数
{
	uint16_t bullet_remaining_num_17mm;
	uint16_t bullet_remaining_num_42mm;
	uint16_t coin_remaining_num;
} ext_bullet_remaining_t;

typedef __packed struct  //0x0209机器人RFD状态
{
 uint32_t rfid_status;
}ext_rfid_status_t;

typedef __packed struct //0x020A飞镖机器人客户端指令数据
{
 uint8_t dart_launch_opening_status;
 uint8_t dart_attack_target;
 uint16_t target_change_time;
 uint16_t operate_launch_cmd_time;
} ext_dart_client_cmd_t;

typedef __packed struct//0x0301交互数据接收信息
{
 uint16_t data_cmd_id;
 uint16_t sender_ID;
 uint16_t receiver_ID;
}ext_student_interactive_header_data_t;
typedef __packed struct//0x0301交互数据
{
uint8_t data[118];
} robot_interactive_data_t;
typedef __packed struct //0x0301UI图像删除
{
	uint8_t operate_tpye;
	uint8_t layer;
} ext_client_custom_graphic_delete_t;


typedef __packed struct //UI图像数据
{
	uint8_t graphic_name[3];
	uint32_t operate_tpye:3;
	uint32_t graphic_tpye:3;
	uint32_t layer:4;
	uint32_t color:4;
	uint32_t start_angle:9;
	uint32_t end_angle:9;
	uint32_t width:10;
	uint32_t start_x:11;
	uint32_t start_y:11;
	uint32_t radius:10;
	uint32_t end_x:11;
	uint32_t end_y:11;
} graphic_data_struct_t;

typedef __packed struct  //绘制一个图像
{
	graphic_data_struct_t grapic_data_struct;
} ext_client_custom_graphic_single_t;

typedef __packed struct  //绘制两个图像
{
	graphic_data_struct_t grapic_data_struct[2];
} ext_client_custom_graphic_double_t;

typedef __packed struct	 //绘制五个图像
{
	graphic_data_struct_t grapic_data_struct[5];
} ext_client_custom_graphic_five_t;

typedef __packed struct	 //绘制七个图像
{
	graphic_data_struct_t grapic_data_struct[7];
} ext_client_custom_graphic_seven_t;

typedef __packed struct  //绘制字符
{
	graphic_data_struct_t grapic_data_struct;
	uint8_t data[30];
} ext_client_custom_character_t;

typedef __packed struct //0x0301  用于绘制图像
{
	uint16_t data_cmd_id;
	uint16_t sender_ID;
  uint16_t receiver_ID;
	ext_client_custom_graphic_single_t ext_client_custom_graphic_single;
}ext_student_interactive_header_data_graphic_t;

typedef __packed struct //0x0301  用于绘制字符
{
	uint16_t data_cmd_id;
	uint16_t sender_ID;
  uint16_t receiver_ID;
	ext_client_custom_character_t ext_client_custom_character;
}ext_student_interactive_header_data_character_t;
//typedef __packed struct

//{
//graphic_data_struct_t grapic_data_struct;
//uint8_t data[30];
//} ext_client_custom_character_t;
typedef __packed struct//0x0302 
{
uint8_t data[39];
} ext_robot_interactive_data_t;
typedef __packed struct//0x0303 小地图下发信息标识
{
float target_position_x;
float target_position_y;
float target_position_z;
uint8_t commd_keyboard;
uint16_t target_robot_ID;
} ext_robot_command_t;

typedef __packed struct 
{
    float data1;
    float data2;
    float data3;
    uint8_t data4;
} custom_data_t;

typedef __packed struct//0x0305客户端接收信息
{
uint16_t target_robot_ID;
float target_position_x;
float target_position_y;
} ext_client_map_command_;
//typedef __packed struct//0x0304客户端下发信息
//{
//int16_t mouse_x;
//int16_t mouse_y;
//int16_t mouse_z;
//int8_t left_button_down;
//int8_t right_button_down;
//uint16_t keyboard_value;
//uint16_t reserved;
//} ext_robot_command_t;

extern void init_referee_struct_data(void);
extern void referee_data_solve(uint8_t *frame);
extern void referee_unpack_fifo_data(void);
extern void get_chassis_power_and_buffer_and_max(fp32 *power, uint16_t *buffer,uint16_t *max_power);
extern void get_chassis_power_limit(uint16_t *power_limit);
extern uint8_t get_robot_id(void);

extern void get_shoot_heat0_limit_and_heat0(uint16_t *heat0_limit, uint16_t *heat0);
extern void get_shoot_heat1_limit_and_heat1(uint16_t *heat1_limit, uint16_t *heat1);
extern void get_shoot_data(uint8_t *bullet_freq,	float *bullet_speed,uint8_t*bullet_speedlimit);

extern void get_robot_hurt( uint8_t* shot_flag);
//-----------------------3.15--------------------//
void get_self_own_side_blood(uint16_t*outpost_blood,uint16_t*self_blood,uint16_t*enemy_outpost_blood);
extern void get_game_process_get_time(uint8_t *game_process,	uint16_t*time);
extern void get_game_state(uint8_t* game_process);
//-----------------------5.26--------------------//

void get_robot_position(float *x,float *y,float *z,float *yaw);
void get_referee_warning(uint8_t *level,uint8_t *foul_robot_id);
void get_robot_target_position(float *target_position_x,float *target_position_y,uint8_t *commd_keyboard);
extern ext_game_status_t game_state;
extern ext_game_robot_HP_t game_robot_HP_t;

#endif
