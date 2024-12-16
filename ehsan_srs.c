/*
 * T_UART_Protocol.c
 *
 *  Created on: Nov 28, 2024
 *      Author: tibin.mathew
 */


#include "T_UART_Protocol.h"

#define RXNE						(0b1 << 5)
#define TXE							(0b1 << 7)
#define IDLE						(0b1 << 4)
#define ORE							(0b1 << 3)

#define CEN    	(1<<0)
#define UIE     (1<<0)


#define IDLE_COUNT_THRESHOLD		(5)

#define crc_polynomial		0x70

#if(GET_ACK == 1)
	bool sendACK = true;
#endif
	int crc_rx=0;
	int crc_tx=0;
	// int crc_flag=0;

typedef struct RING_BUFFER
{
	uint8_t dataBuffer[BUFFER_SIZE * DATA_WIDTH];
	uint8_t writeIndex;
	uint8_t readIndex;
	uint8_t byteCount;
}RING_BUFFER;

void setupTimeoutTimer(uint8_t idleTimeoutSec);
void stopTimeoutTimer();
void startTimeoutTimer();

bool hasDataToSend = false;
bool hasDataToRead = false;

RING_BUFFER rxBuffer = {"", 0, 0, 0};
RING_BUFFER txBuffer = {"", 0, 0, 0};

bool gotStartByte = false;
bool sentStartByte = false;

uint8_t idleCount = 0;

TIM_TypeDef* timeoutTimer;


STATUS ProtocolSetup(uint8_t timeoutDurationSec)
{
	uart_init();
	startUARTRx();
	setupTimeoutTimer(timeoutDurationSec);
	return STATUS_OK;
}

void USART2_IRQHandler()
{
//	if(USART2->SR & ORE)
//	{
//		return;
//	}


	if(USART2->SR & RXNE)
	{

		uint8_t ch = USART2->DR;
		if(!gotStartByte)
		{
			if(ch != START_BYTE)
			{
				// Do something if needed
				return;
			}

			startTimeoutTimer();
			gotStartByte = true;
		}
		else
		{
			// Read bytes till count is 15
			if(rxBuffer.byteCount < DATA_WIDTH)
			{
				RX_BUFFER[RX_WRITE_OFFSET + rxBuffer.byteCount++] = ch;
				crc_rx^=ch;
				for(uint8_t bit=0;bit<8;bit++)
				  {
				    if(crc_rx & 0x80)
				     {
				       crc_rx = (crc_rx<<1) ^crc_polynomial;

				      }
				     else
				        crc_rx=crc_rx<<1;

		            }
             }

			else if (rxBuffer.byteCount == DATA_WIDTH)
			 {
			   if(crc_rx!=ch)
				{
				  stopTimeoutTimer();
				  rxBuffer.byteCount = 0;
				  gotStartByte = false;
				}
			}

			// If count over, check if byte is Endbyte

			else
			{
				stopTimeoutTimer();
				rxBuffer.byteCount = 0;
				gotStartByte = false;
				 if(ch != END_BYTE)
				 {
				 	// Do something if needed

				 	#if(GET_ACK == 1)		// Set flag to send NACK
				 		sendACK = false;
				 	#endif
				 }
				 else
				 {
				 	// Do something if needed


					// Increment the writeIndex cyclicly
				 	rxBuffer.writeIndex++;
				  	if(rxBuffer.writeIndex == BUFFER_SIZE)
					{
						rxBuffer.writeIndex = 0;
						crc_rx=0;
					}

					hasDataToRead = true;

					#if(GET_ACK == 1)			// Set flag to send ACK
						sendACK = true;
					#endif
				}
			}
		}
	}



	else if(USART2->SR & TXE)
	{

		if(!hasDataToSend)
		{
			stopUARTTx();
			return;
		}

		if(!sentStartByte)
		{
			// Send Start Byte
			sentStartByte = true;
			uart_send(START_BYTE);
		}

		else
		{
			if(txBuffer.byteCount < DATA_WIDTH)
			{
				uart_send(TX_BUFFER[TX_READ_OFFSET + txBuffer.byteCount++]);
				crc_tx^=TX_BUFFER[TX_READ_OFFSET + (txBuffer.byteCount)-1];
				for(uint8_t bit=0;bit<8;bit++)
				  {
				    if(crc_tx & 0x80)
				      {
				        crc_tx = (crc_tx<<1) ^crc_polynomial;
				       }
				      else
				        crc_tx=crc_tx<<1;

				   }

			}
			else if(txBuffer.byteCount == DATA_WIDTH)
			{
				uart_send(crc_tx);
				txBuffer.byteCount++;
			}

			else
			{
				txBuffer.byteCount = 0;
				sentStartByte = false;
				txBuffer.readIndex++;
				if(txBuffer.readIndex == BUFFER_SIZE)
				{
					txBuffer.readIndex = 0;
					crc_tx=0;
				}
				if(txBuffer.readIndex == txBuffer.writeIndex)
				{
					hasDataToSend = false;
				}

				uart_send(END_BYTE);
			}
		}
	}
}

void writeFrameData(uint8_t* frame)
{
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

	hasDataToSend = true;
}

bool hasDataToRead_func()
{
	return hasDataToRead;
}

void getFrameData(uint8_t* outFrame)
{
	if(!hasDataToRead)
	{
		return;
	}
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
	if(rxBuffer.readIndex == rxBuffer.writeIndex)
	{
		hasDataToRead = false;
	}
}

#if TIMEOUT_TIMER == 2
void TIM2_IRQHandler()
{
	if(timeoutTimer->SR & (1 << 0))
	{
		// Reset the buffers
		gotStartByte = false;
		rxBuffer.byteCount = 0;
		stopTimeoutTimer();
		timeoutTimer->SR &= ~(1 << 0);
	}
}
#endif

#if TIMEOUT_TIMER == 3
void TIM3_IRQHandler()
{
	// Reset the buffers
	gotStartByte = false;
	rxBuffer.byteCount = 0;
	stopTimeoutTimer();
}
#endif

#if TIMEOUT_TIMER == 4
void TIM4_IRQHandler()
{
	// Reset the buffers
	gotStartByte = false;
	rxBuffer.byteCount = 0;
	stopTimeoutTimer();
}
#endif

void setupTimeoutTimer(uint8_t idleTimeoutSec)
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

	timeoutTimer->CR1 |= (1<<0) | (1<<2);
	timeoutTimer->DIER |= (1<<0);
	timeoutTimer->PSC = 7999;
	timeoutTimer->ARR = idleTimeoutSec * 1000;
}

void startTimeoutTimer()
{
	timeoutTimer->DIER |= UIE;       // Enable hardware interrupt and update

	timeoutTimer->CNT = 0;

	timeoutTimer->CR1 |= CEN;			    // Start timer
	timeoutTimer->SR &= ~(1 << 0);

//	switch(TIMEOUT_TIMER)
//	{
//		case 2:
//			NVIC_SetPriority(TIM2_IRQn, 2);
//			NVIC_EnableIRQ(TIM2_IRQn);
//			break;
//		case 3:
//			NVIC_SetPriority(TIM3_IRQn, 2);
//			NVIC_EnableIRQ(TIM3_IRQn);
//			break;
//		case 4:
//			NVIC_SetPriority(TIM4_IRQn, 2);
//			NVIC_EnableIRQ(TIM4_IRQn);
//			break;
//	}

	NVIC_SetPriority(TIM2_IRQn + TIMEOUT_TIMER - 2, 2);
	NVIC_EnableIRQ(TIM2_IRQn + TIMEOUT_TIMER - 2);

}

void stopTimeoutTimer()
{
	timeoutTimer->CR1 &= ~CEN;
	timeoutTimer->DIER = 0;
	NVIC_DisableIRQ(TIM2_IRQn + TIMEOUT_TIMER - 2);

}

void resetTimeoutTimer()
{
	timeoutTimer->CNT = 0;
}
