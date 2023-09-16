/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t I2C_recvBuf[10] = {0};
uint8_t* I2C_sendBuf = NULL;
//static uint8_t I2C_recvBuf2[10]={0};
uint8_t rx_flag=0;
uint8_t position_x[3]={0,45,90};
uint8_t position_y[4]={0,12,24,36};

float average_speed;
float range_speed;
Bullets bullet[12];

uint8_t round_flag=0;
uint8_t shoot_flag=0;
uint8_t shoot_turns=0;
uint8_t last_shoot_turns=0;
uint8_t fly_bullets=0;  
uint32_t fly_time=0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim2);
  HAL_I2C_EnableListen_IT(&hi2c2); 
	OLED_Init();
	OLED_ColorTurn(0);//0正常显示，1 反色显示
  OLED_DisplayTurn(1);//0正常显示 1 屏幕翻转显示
  uint8_t* floatToBytes(float floatValue);
  float bytesToFloat(uint8_t *byteArray);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	HAL_I2C_EnableListen_IT(&hi2c2);
 //HAL_Delay(10);
	//HAL_Delay(500);
  //HAL_Delay(10);
  OLED_showspeed(range_speed,average_speed,bullet);
  I2C_sendBuf = floatToBytes(average_speed);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void shoot_data_clear(uint8_t shoot_turns,Bullets* bullets)  //??????
{
  if((shoot_turns==1)&&(round_flag))
  {
    round_flag=0;
    for(int i=1;i<bulletMax;i++)
    {
      bullets[i].shoot_speed=0;
      bullets[i].shoot_error_flag=0;
      bullets[i].start_time=0;
      bullets[i].end_time=0;
    }
  }
}
 
float cal_range_speed(uint8_t shoot_turns,Bullets* bullets)  //??????
{
  /*??????*/
  float range_speed=0;
  float max_speed=bullets[0].shoot_speed;
  float min_speed=bullets[0].shoot_speed;
  for(int i=0;i<shoot_turns;i++)
  {
    if(!bullets[i].shoot_error_flag)  //?????????
    {
      if(max_speed<bullets[i].shoot_speed) max_speed=bullets[i].shoot_speed;
      if(min_speed>bullets[i].shoot_speed) min_speed=bullets[i].shoot_speed;            
    }
  }
  range_speed=max_speed-min_speed;
  return range_speed;
}

float cal_average_speed(uint8_t shoot_turns,Bullets* bullets)  //????????
{
  /*??????????*/
  float average_speed=0;
  float speed_sum=0;
  uint8_t shoot_error_turns=0;
  for(int i=0;i<shoot_turns;i++)
  {
    if(!bullets[i].shoot_error_flag) //?????????
      speed_sum+=bullets[i].shoot_speed;
    else
      shoot_error_turns++;
  }
  if(shoot_turns>shoot_error_turns) //????????
    average_speed=(float)speed_sum/(shoot_turns-shoot_error_turns);
  else
    average_speed=0;
  return average_speed;
}

//???????
void cal_shoot_speed(uint8_t shoot_turns,Bullets* bullets)
{
  for(int i=0;i<shoot_turns;i++)
  {
    float fly_time=bullets[i].end_time-bullets[i].start_time;
    if(bullets[i].end_time!=0)  //?????з?????
    {
      if((fly_time>=deadTime*1000)||(fly_time==0))   //0.2s
        bullets[i].shoot_error_flag=1;  //?????????
      else
        bullets[i].shoot_speed=(float)length/fly_time*100.0f;   // m/s    
    }
  }
}

uint32_t get_time(void)
{
  return fly_time;
}


/*将float类型的数据拆成四个u8*/
uint8_t* floatToBytes(float floatValue) {
    
    static uint8_t byteArray[4];
    // 将float类型的数据转换为32位整数
    uint32_t intData = *((uint32_t*)&floatValue);

    // 将32位整数拆分成4个字节
    byteArray[0] = (uint8_t)(intData & 0xFF);
    byteArray[1] = (uint8_t)((intData >> 8) & 0xFF);
    byteArray[2] = (uint8_t)((intData >> 16) & 0xFF);
    byteArray[3] = (uint8_t)((intData >> 24) & 0xFF);

    return byteArray;
}

/*四个字节合并成一个float数据*/
float bytesToFloat(uint8_t *byteArray) {
    uint32_t intData = (uint32_t)byteArray[0] |
                      ((uint32_t)byteArray[1] << 8) |
                      ((uint32_t)byteArray[2] << 16) |
                      ((uint32_t)byteArray[3] << 24);

    return *((float*)&intData);
}

/**
  * @brief  ???ж???????
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin==GPIO_PIN_4) //???
  {
		if(!shoot_flag)
		{
			if(shoot_turns>=bulletMax)  
			{
				shoot_turns=0;
				//round_flag=1; //一轮结束
			}
			bullet[shoot_turns].start_time=get_time();
			shoot_flag=1;
		}
  }
  else if(GPIO_Pin==GPIO_PIN_5) //???
  {
		if(shoot_flag)
	  {
			bullet[shoot_turns].end_time=get_time();
			shoot_turns++;
			shoot_flag=0;			
		}
  }
}

/**
  * @brief  ??????ж???????
  * @param  None
  * @retval None
  */  

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==&htim1) //?????1 0.01ms
  {
    fly_time++;
    if(fly_time>=1000000000) fly_time=0;
  }  
	if(htim==&htim2) //?????2  1ms
	{
		//shoot_data_clear(shoot_turns,bullet); //????????
    cal_shoot_speed(shoot_turns,bullet); //???????
    average_speed=cal_average_speed(shoot_turns,bullet); //??????????
    range_speed=cal_range_speed(shoot_turns,bullet); //??????
		if(shoot_turns>=bulletMax)  
		{
			//shoot_turns=0;
			round_flag=1; //一轮结束
		}
	}
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
		rx_flag++;

  if(TransferDirection == I2C_DIRECTION_TRANSMIT)
  {
		HAL_I2C_Slave_Seq_Receive_IT(&hi2c2, I2C_recvBuf, sizeof(I2C_recvBuf), I2C_LAST_FRAME);    
  }
  else if(TransferDirection == I2C_DIRECTION_RECEIVE)
  {
		HAL_I2C_Slave_Seq_Transmit_IT(&hi2c2, I2C_sendBuf, sizeof(I2C_sendBuf), I2C_LAST_FRAME);    
  }
}
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{

}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{

		HAL_I2C_DeInit(hi2c);
		MX_I2C2_Init();
	
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
