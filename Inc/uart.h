/*
 * uart.h
 *
 *  Created on: Dec 9, 2024
 *      Author: tibin.mathew
 */

#ifndef UART_H_
#define UART_H_

#include "stdint.h"

#define TX_PIN_NUMBER		(2)
#define RX_PIN_NUMBER		(3)
#define UART_GPIO			GPIOA
#define USARTx				USART2

#define PCLK				(16000000U)
#define BAUD_RATE			(115200U)

void UART_init();
void UART_send(uint8_t data);

void UART_start_Rx();
void UART_start_Tx();
void UART_stop_Tx();


#endif /* UART_H_ */
