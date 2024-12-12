/*
 * uart.c
 *
 *  Created on: Dec 9, 2024
 *      Author: tibin.mathew
 */

// Tx : PA2
// Rx : PA3

#include "uart.h"
#include "stm32f4xx.h"
#include "GPIO.h"

uint16_t calculateBaudRate();

void enableRxInterrupt();
void disableRxInterrupt();
void enableTxInterrupt();
void disableTxInterrupt();

void UART_init()
{
	// need to change here according to USARTx
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	GPIOInit(UART_GPIO, TX_PIN_NUMBER, ALTERNATE_FUNCTION);
	GPIOInit(UART_GPIO, RX_PIN_NUMBER, ALTERNATE_FUNCTION);

	setAlternateFunction(UART_GPIO, TX_PIN_NUMBER, 7);
	setAlternateFunction(UART_GPIO, RX_PIN_NUMBER, 7);

	USARTx->CR1 |= USART_CR1_TE;
	USARTx->CR1 |= USART_CR1_RE;


	// Baud rate of 115200
	USARTx->BRR = calculateBaudRate();

	USARTx->CR1 |= USART_CR1_UE;

	// need to change here according to USARTx
	NVIC_EnableIRQ(USART2_IRQn);
}

void UART_send(uint8_t data)
{
	while(!(USARTx->SR & USART_SR_TXE));

	USARTx->DR = data;
}

uint16_t calculateBaudRate()
{
	uint16_t USART_DIV_M = (uint16_t)(PCLK / (16 * BAUD_RATE));
	uint8_t USART_DIV_F = (uint8_t) ((PCLK % (16 * BAUD_RATE)) / BAUD_RATE);

	return ((USART_DIV_M << 4) | (USART_DIV_F & 0xF));
}



void UART_start_Rx()
{
	USARTx->CR1 |= USART_CR1_RE;
	enableRxInterrupt();
}

void UART_start_Tx()
{
	USARTx->CR1 |= USART_CR1_TE;
	enableTxInterrupt();
}

void UART_stop_Tx()
{
	while(!(USARTx->SR & USART_SR_TC));
	USARTx->CR1 &= ~USART_CR1_TE;
	disableTxInterrupt();
}

void enableRxInterrupt()
{
	USARTx->CR1  |= USART_CR1_RXNEIE; // Enable RXNEIE (Receive Not Empty) interrupt for USARTx.
}

void disableRxInterrupt()
{
	USARTx->CR1  &= ~USART_CR1_RXNEIE; // Enable RXNEIE (Receive Not Empty) interrupt for USARTx.
}

void enableTxInterrupt()
{
	USARTx->CR1  |= USART_CR1_TXEIE; // Enable TXEIE (Transmit Empty) interrupt for USARTx.
}

void disableTxInterrupt()
{
	USARTx->CR1  &= ~USART_CR1_TXEIE; // Enable TXEIE (Transmit Empty) interrupt for USARTx.
}
