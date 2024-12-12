/*
 * GPIO.h
 *
 *  Created on: Dec 10, 2024
 *      Author: tibin.mathew
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>
#include "stm32f4xx.h"

typedef enum MODER
{
	INPUT,
	GENRAL_OUTPUT,
	ALTERNATE_FUNCTION,
	ANALOG_MODE
}MODER;

typedef enum STATE
{
	LOW,
	HIGH
}STATE;

void GPIOInit(GPIO_TypeDef* GPIOx, uint8_t pinNumber, MODER mode);
void setAlternateFunction(GPIO_TypeDef* GPIOx, uint8_t pinNumber, uint8_t alternateFunction);
void writeToPin(GPIO_TypeDef* GPIOx, uint8_t pinNumber, STATE state);

#endif /* GPIO_H_ */
