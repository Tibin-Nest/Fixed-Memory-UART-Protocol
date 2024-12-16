/*
 * T_UART_Protocol.c
 *
 *  Created on: Nov 28, 2024
 *      Author: tibin.mathew
 */


#include "T_UART_Protocol.h"


#define RX_WRITE_OFFSET				(rxBuffer.writeIndex * DATA_WIDTH)
#define RX_READ_OFFSET				(rxBuffer.readIndex * DATA_WIDTH)
#define TX_WRITE_OFFSET				(txBuffer.writeIndex * DATA_WIDTH)
#define TX_READ_OFFSET				(txBuffer.readIndex * DATA_WIDTH)
#define RX_BUFFER					(rxBuffer.dataBuffer)
#define TX_BUFFER					(txBuffer.dataBuffer)

#define CRC_POLYNOMIAL				(0x70)
#define CRC_INIT					(0x00)

#define CRC_START_VALUE				(CRC_INIT ^ START_BYTE)

#if(GET_ACK == 1)
	bool sendACK = true;
#endif

typedef struct RING_BUFFER
{
	uint8_t dataBuffer[BUFFER_SIZE * DATA_WIDTH];
	uint8_t writeIndex;
	uint8_t readIndex;
	uint8_t byteCount;
}RING_BUFFER;

static void USARTx_IRQHandler_Func();
static void TIMx_IRQHandler_Func();

static void setupTimeoutTimer();
static void stopTimeoutTimer();
static void startTimeoutTimer(uint32_t idleTimeout_ms);


uint32_t txFrameCount = 0;
uint32_t rxFrameCount = 0;

RING_BUFFER rxBuffer = {"", 0, 0, 0};
RING_BUFFER txBuffer = {"", 0, 0, 0};

bool hasDataToSend = false;
bool hasDataToRead = false;

bool gotStartByte = false;
bool sentStartByte = false;

bool gotCRC = false;

uint8_t crcRx = CRC_INIT;
uint8_t crcTx = CRC_INIT;

uint32_t timeoutDuration = 1000;

TIM_TypeDef* timeoutTimer;

STATUS ProtocolSetup(uint32_t timeoutDuration_ms)
{
	UART_init();
	setupTimeoutTimer();
	stopTimeoutTimer();
	timeoutDuration = timeoutDuration_ms;
	crcRx = CRC_START_VALUE;
	crcTx = CRC_START_VALUE;
	UART_start_Rx();
	return STATUS_OK;
}


/**
  * @brief 	Rx and Tx UART2 interrupt handler that checks frame protocol
  * @note 	Receive:
  * 		Only starts storing if start byte has been sent. Next DATA_WIDTH bytes are stored unchecked.
  * 		If the next byte is not end byte, the writeIndex is not incremented and the written frame
  * 		can be overwritten.
  * 		Transmit:
  * 		Sends start byte if not sent, then sends DATA_WIDTH bytes from buffer, then sends an end byte.
  * @param 	None
  * @retval None
  */

// Change this to the USART module that is being used
void USART2_IRQHandler()
{
	USARTx_IRQHandler_Func();
}

static void USARTx_IRQHandler_Func()
{
	// Receive
	if(USARTx->SR & USART_SR_RXNE)
	{
		uint8_t ch = USARTx->DR;
		if(ch == START_BYTE)
		{
			gotStartByte = gotStartByte;
		}

		// Wait for start byte
		if(!gotStartByte)
		{
			if(ch != START_BYTE)
			{
				// Do something if needed
				return;
			}

			// Reset CRC
			crcRx = CRC_START_VALUE;

			gotStartByte = true;
			startTimeoutTimer(timeoutDuration);
		}
		// Read bytes till count is DATA_WIDTH
		else if(rxBuffer.byteCount < DATA_WIDTH)
		{
			RX_BUFFER[RX_WRITE_OFFSET + rxBuffer.byteCount++] = ch;

			// Update CRC
			crcRx ^= ch;
			for(uint8_t bit = 0; bit < 8; bit++)
			{
				if(crcRx & 0b10000000)
				{
					crcRx = (crcRx << 1) ^ CRC_POLYNOMIAL;
				}
				else
				{
					crcRx = crcRx << 1;
				}

			}
		}
		// Last byte
		else
		{
			stopTimeoutTimer();

			rxBuffer.byteCount = 0;
			gotStartByte = false;

			// If not CRC
			if(ch != crcRx)
			{
				// Do something if needed

				#if(GET_ACK == 1)			// Set flag to send NACK
					sendACK = false;
				#endif
			}

			// If it is CRC
			else
			{
				// Do something if needed

				// Increment the writeIndex cyclicly
				rxBuffer.writeIndex++;
				if(rxBuffer.writeIndex == BUFFER_SIZE)
				{
					rxBuffer.writeIndex = 0;
				}

				hasDataToRead = true;

				#if(GET_ACK == 1)			// Set flag to send ACK
					sendACK = true;
				#endif
			}
		}
	}


	// Transmit
	else if(USARTx->SR & USART_SR_TXE)
	{
		// If no data to send, stop Tx
		if(!hasDataToSend)
		{
			UART_stop_Tx();
			return;
		}

		if(!sentStartByte)
		{
			// Reset CRC
			crcTx = CRC_START_VALUE;

			// Send Start Byte
			sentStartByte = true;
			UART_send(START_BYTE);
		}
		// Send DATA_WIDTH bytes from buffer
		else if(txBuffer.byteCount < DATA_WIDTH)
		{
			uint8_t ch = TX_BUFFER[TX_READ_OFFSET + txBuffer.byteCount++];
			UART_send(ch);

			// Update CRC
			crcTx ^= ch;
			for(uint8_t bit = 0; bit < 8; bit++)
			{
				if(crcTx & 0b10000000)
				{
					crcTx = (crcTx << 1) ^ CRC_POLYNOMIAL;
				}
				else
				{
					crcTx = crcTx << 1;
				}
			}
		}
		// Send CRC
		else
		{
			txBuffer.byteCount = 0;
			sentStartByte = false;

			txBuffer.readIndex++;
			if(txBuffer.readIndex == BUFFER_SIZE)
			{
				txBuffer.readIndex = 0;
			}
			if(txBuffer.readIndex == txBuffer.writeIndex)
			{
				hasDataToSend = false;
			}
			UART_send(crcTx);

		}
	}
}

/**
  * @brief 	Write the frame data to the transmit buffer
  * @note 	Prevents direct access to the transmit buffer.
  * 		Writes data in frames of DATA_WIDTH
  * 		Automatically increments writeIndex
  * @param 	frame	-	frame data to send
  * @retval STATUS_NO_DATA 	-	if no data in frame
  * 		STATUS_OK		-	if data is stored in outFrame
  */
STATUS writeFrameData(uint8_t* frame)
{
	if(frame == NULL)
	{
		return STATUS_NO_DATA;
	}

	// Prevent overwriting if buffer full
	if(hasDataToSend && txBuffer.readIndex == txBuffer.writeIndex)
	{
		return STATUS_BUFFER_FULL;
	}

	// Add data to buffer
	for(int i = 0; i < DATA_WIDTH; i++)
	{
		TX_BUFFER[TX_WRITE_OFFSET + i] = frame[i];
	}

	// Increment writeIndex cyclically
	txBuffer.writeIndex++;
	if(txBuffer.writeIndex == BUFFER_SIZE)
	{
		txBuffer.writeIndex = 0;
	}

	// Set the flag to read frame
	hasDataToSend = true;

	// Start transmission in UART;
	UART_start_Tx();

	return STATUS_OK;
}

/**
  * @brief 	Get the frame data from the receive buffer
  * @note 	Prevents direct access to the receive buffer.
  * 		Gives data in frames of DATA_WIDTH
  * 		Automatically increments readIndex
  * @param 	outFrame	-	buffer to return the frame (only data)
  * @retval STATUS_NO_DATA 	-	if all data is read
  * 		STATUS_OK		-	if data is stored in outFrame
  */
STATUS getFrameData(uint8_t* outFrame)
{
	if(!hasDataToRead)
	{
		return STATUS_NO_DATA;
	}

	// Get data from the buffer to pointer
	for(int i = 0; i < DATA_WIDTH; i++)
	{
		outFrame[i] = RX_BUFFER[RX_READ_OFFSET + i];
	}

	// Increment readIndex cyclically
	rxBuffer.readIndex++;
	if(rxBuffer.readIndex == BUFFER_SIZE)
	{
		rxBuffer.readIndex = 0;
	}

	// Set flag if no more frames to read
	if(rxBuffer.readIndex == rxBuffer.writeIndex)
	{
		hasDataToRead = false;
	}

	return STATUS_OK;
}

#if TIMEOUT_TIMER == 2
void TIM2_IRQHandler()
{
	TIMx_IRQHandler_Func();
}
#endif

#if TIMEOUT_TIMER == 3
void TIM3_IRQHandler()
{
	TIMx_IRQHandler_Func();
}
#endif

#if TIMEOUT_TIMER == 4
void TIM4_IRQHandler()
{
	TIMx_IRQHandler_Func();
}
#endif

static void TIMx_IRQHandler_Func()
{
	if(timeoutTimer->SR & TIM_SR_UIF)
	{
		// Reset the buffers
		gotStartByte = false;
		rxBuffer.byteCount = 0;
		timeoutTimer->SR &= ~TIM_SR_UIF;
		stopTimeoutTimer();
	}
}

static void setupTimeoutTimer()
{
	switch(TIMEOUT_TIMER)
	{
	case 2:
		timeoutTimer = TIM2;
		break;
	case 3:
		timeoutTimer = TIM3;
		break;
	case 4:
		timeoutTimer = TIM4;
		break;
	}

	RCC->APB1ENR |= (1 << (TIMEOUT_TIMER - 2));

//	timeoutTimer->CR1 |=  TIM_CR1_CEN;
//	timeoutTimer->DIER |= TIM_DIER_UIE;
	timeoutTimer->PSC = (TIMER_CLK / 1000) - 1;
}

static void startTimeoutTimer(uint32_t idleTimeout_ms)
{
	timeoutTimer->ARR = idleTimeout_ms;
	timeoutTimer->DIER  = 0;
	timeoutTimer->DIER |= TIM_DIER_UIE;       	// Enable hardware interrupt and update

	timeoutTimer->CNT = 0;

	timeoutTimer->CR1 |= TIM_CR1_CEN;			// Start timer

	NVIC_DisableIRQ(TIM2_IRQn + TIMEOUT_TIMER - 2);

	timeoutTimer->EGR |= TIM_EGR_UG;
	timeoutTimer->SR &= ~TIM_SR_UIF;

	NVIC_SetPriority(TIM2_IRQn + TIMEOUT_TIMER - 2, 2);
//	timeoutTimer->SR = 0;
	NVIC_EnableIRQ(TIM2_IRQn + TIMEOUT_TIMER - 2);

}

static void stopTimeoutTimer()
{
	timeoutTimer->CR1 &= ~TIM_CR1_CEN;
	timeoutTimer->DIER = 0;
	NVIC_DisableIRQ(TIM2_IRQn + TIMEOUT_TIMER - 2);

}

//static void resetTimeoutTimer()
//{
//	timeoutTimer->CNT = 0;
//}
