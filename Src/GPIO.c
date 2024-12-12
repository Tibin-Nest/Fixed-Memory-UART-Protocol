#include "GPIO.h"



void GPIOInit(GPIO_TypeDef* GPIOx, uint8_t pinNumber, MODER mode)
{
	if(GPIOx == GPIOA)
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	}
	else if(GPIOx == GPIOB)
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	}
	else if(GPIOx == GPIOC)
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	}
	else if(GPIOx == GPIOD)
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	}
	else if(GPIOx == GPIOE)
	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	}

	GPIOx->MODER &= ~(0b11 << (pinNumber * 2));
	GPIOx->MODER |= (mode << (pinNumber * 2));
}

void setAlternateFunction(GPIO_TypeDef* GPIOx, uint8_t pinNumber, uint8_t alternateFunction)
{
	alternateFunction &= 0xF;
	uint8_t afrIndex = 0;
	if(pinNumber > 7)
	{
		afrIndex = 1;
		pinNumber -= 8;
	}

	GPIOx->AFR[afrIndex] &= ~(0b1111 << (pinNumber * 4));
	GPIOx->AFR[afrIndex] |= (alternateFunction << (pinNumber * 4));
}

void writeToPin(GPIO_TypeDef* GPIOx, uint8_t pinNumber, STATE state)
{
	GPIOx->ODR &= ~(1 << pinNumber);
	GPIOx->ODR |= (state << pinNumber);
}
