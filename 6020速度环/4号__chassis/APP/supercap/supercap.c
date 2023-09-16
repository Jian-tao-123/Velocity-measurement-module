#include "supercap.h"
#include "referee.h"
uint16_t supercap_volt=0;  //超级电容电压
/**
  * @breif         超级电容，根据不同的功率上限给电容上单片机发送数据
  * @param[in]     none 
	* @param[out]    代表不同功率的字符
  * @retval        none     
  */
void supercap(void)
{
	static uint8_t send_data;
	uint16_t power_limit;
	get_chassis_power_limit(&power_limit);
	
	switch(power_limit)
	{
		case 40:
		{
			send_data='I';
			break;
		}
		case 45:
		{
			send_data='J';
			break;
		}
		case 50:
		{
			send_data='K';
			break;
		}
		case 55:
		{
			send_data='L';
			break;
		}
		case 60:
		{
			send_data='M';
			break;
		}
		case 80:
		{
			send_data='Q';
			break;
		}
		case 100:
		{
			send_data='U';
			break;
		}			
	}
	HAL_UART_Transmit(&huart1, &send_data, 1, 20);
}
