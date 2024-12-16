/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "stm32f4xx.h"
//#include "uart.h"
#include "GPIO.h"
#include "T_UART_Protocol.h"
#include "EEPROM_Processing.h"


int main(void)
{
	#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
		SCB->CPACR |= ((3UL << 20U)|(3UL << 22U));  /* set CP10 and CP11 Full Access */
	#endif

	ProtocolSetup(2000);


    /* Loop forever */
	while(1)
	{
		processCommand();
	}
}
