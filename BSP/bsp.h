/**
  ******************************************************************************
  * @文	件 ： bsp.h
  * @作	者 ： INNFOS Software Team
  * @版	本 ： V1.0.0
  * @日	期 ： 2019.6.14
  * @摘	要 ： 底层驱动初始化
  ******************************************************************************/
#ifndef __BSP_H
#define __BSP_H

/* Includes ----------------------------------------------------------------------*/
#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "bsp_can.h"

/* Port defines-------------------------------------------------------------------*/
#define LED1 PDout(2)	
#define LED2 PDout(3)	 

#define LED1_ON()		GPIO_ResetBits(GPIOD, GPIO_Pin_2)
#define LED1_OFF()		GPIO_SetBits(GPIOD, GPIO_Pin_2)
#define LED1_TOGGLE()	(GPIOD->ODR ^= GPIO_Pin_2)

#define LED2_ON()		GPIO_ResetBits(GPIOD, GPIO_Pin_3)
#define LED2_OFF()		GPIO_SetBits(GPIOD, GPIO_Pin_3)
#define LED2_TOGGLE()	(GPIOD->ODR ^= GPIO_Pin_3)



void BSP_Init(void);

#endif
