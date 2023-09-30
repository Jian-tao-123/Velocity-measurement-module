## 简介

> 通过软件spi和七针OLED通信，预留从机IIC接口和主机通信（IIC从文件）

## IIC

> IIC(MASTER AND SLAVE)

## 硬件

- 1.54 寸 OLED(七针)
- stm32f103c8t6(两个 IIC 通信)
  ~~采用硬件 IIC~~

## 接线方式

- OLED

```C
#define OLED_SCL_Clr() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET)//SCL
#define OLED_SCL_Set() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET)

#define OLED_SDA_Clr() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET)//SDA
#define OLED_SDA_Set() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET)

#define OLED_RES_Clr() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET)//RES
#define OLED_RES_Set() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_SET)

#define OLED_DC_Clr()  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET)//DC
#define OLED_DC_Set()  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET)

#define OLED_CS_Clr()  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET)//CS
#define OLED_CS_Set()  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET)

```

- IIC
  - 主机
  ```C
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
  ```
  - 从机
  ```C
    PB10     ------> I2C2_SCL
    PB11     ------> I2C2_SDA
  ```
## CubeMx配置
![Alt text](image.png)
