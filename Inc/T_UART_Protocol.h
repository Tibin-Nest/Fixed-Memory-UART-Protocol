/*
 * T_UART_Protocol.h
 *
 *  Created on: Nov 28, 2024
 *      Author: tibin.mathew
 */

/*
 * This is a protocol that acts as an interface between UART and any application.
 *
 * The protocol is as follows:
 * 		1.All data should be in frames
 * 		2.Each frame should be fixed length. If data is less than DATA_WIDTH, 0's can be added
 * 		3.Has a Start Byte and an End Byte.
 *
 * Features of this implementation:
 * 		1.Uses fixed memory space as a ring buffer for Rx and Tx.
 * 		2.Has timeout to recover from incorrect frame.
 * 		3.Completely independent of the UART or application settings. (Currently using USART2)
 */

/*
 * Required functions in UART
 * 		1.void UART_init();
 *		2.void UART_send(uint8_t data);
 *		3.void UART_start_Rx();
 *		4.void UART_start_Tx();
 *		5.void UART_stop_Tx();
 */

#ifndef T_UART_PROTOCOL_H_
#define T_UART_PROTOCOL_H_

#include "uart.h"
#include "stm32f4xx.h"

#if !defined(USARTx)
	#define USARTx			USART2
#endif

#ifndef NULL
	#define NULL			0
#endif

//#define USART				2

#define TIMER_CLK			8000000				// Clock frequency to the timer
#define TIMEOUT_TIMER		2					// Which timer to use for timeout (TIM2, TIM3, TIM4)

#define FRAME_WIDTH			(16)				// Total frame size including start and end byte
#define DATA_WIDTH			(FRAME_WIDTH - 2)	// Considering no CRC byte
#define BUFFER_SIZE			(4)					// No. of frames that can be stored

//#define START_BYTE		(0b10101010)
//#define END_BYTE			(0b11001100)
#define START_BYTE			'?'
#define END_BYTE			'+'

#define GET_ACK				0					// If 1, ACK or NACK is sent for every frame

typedef enum STATUS
{
	STATUS_OK,
	STATUS_INVALID_FORMAT,
	STATUS_CORUPT_FRAME,
	STATUS_DATA_LOST,
	STATUS_NO_DATA,
	STATUS_BUFFER_FULL
}STATUS;

typedef enum bool
{
	false,
	true
} bool;

STATUS ProtocolSetup(uint32_t timeoutDurationSec);
STATUS getFrameData(uint8_t* outFrame);
STATUS writeFrameData(uint8_t* frame);

#endif /* T_UART_PROTOCOL_H_ */
