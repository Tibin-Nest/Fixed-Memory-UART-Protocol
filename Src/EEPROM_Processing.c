/*
 * EEPROM_Processing.c
 *
 *  Created on: Nov 29, 2024
 *      Author: tibin.mathew
 */

#include "EEPROM_Processing.h"


uint8_t frameData[DATA_WIDTH];

STATUS stringToInt(uint8_t startIndex, uint8_t* output, uint8_t* outIndex);
void testRead(uint8_t* buff, uint8_t Size);
void testWrite(uint8_t* buff, uint8_t Size);

void processCommand()
{
	if(getFrameData(frameData) == STATUS_NO_DATA)
	{
		return;
	}

	if(frameData[0] == 'r')
	{
		uint8_t i = 2;			// first two bytes are 'r' and ' '

		/**********Read data from EEPROM********/
		// Get start page
		uint8_t pageNo;
		if(stringToInt(i, &pageNo, &i) != STATUS_OK)
		{
			// Do something
			return;
		}

		// Get byteOffset
		uint8_t byteOffset;
		if(stringToInt(i, &byteOffset, &i) != STATUS_OK)
		{
			// Do something
			return;
		}

		// Get size of data in bytes
		uint8_t dataSize;
		if(stringToInt(i, &dataSize, &i) != STATUS_OK)
		{
			// Do something
			return;
		}

		uint8_t EEPROMData[DATA_WIDTH];

		while(dataSize)
		{
			memset(EEPROMData, '\0', DATA_WIDTH);

			if(dataSize > DATA_WIDTH)
			{
//				EEPROM_Read(DEVICE_ADDR, pageNo, byteOffset, 2, EEPROMData, DATA_WIDTH);
				testRead(EEPROMData, DATA_WIDTH);

				if(byteOffset + DATA_WIDTH > PAGE_SIZE)
				{
					pageNo++;
					byteOffset = (byteOffset + DATA_WIDTH) - PAGE_SIZE;
				}

				dataSize -= DATA_WIDTH;
			}
			else
			{
//				EEPROM_Read(DEVICE_ADDR, pageNo, byteOffset, 2, EEPROMData, dataSize);
				testRead(EEPROMData, dataSize);

				dataSize = 0;
			}

			while(writeFrameData(EEPROMData) == STATUS_BUFFER_FULL);
		}
//		startUARTTx();

	}
	else if(frameData[0] == 'w')
	{
		uint8_t i = 2;			// first two bytes are 'w' and ' '

		/**********Write data to EEPROM********/
		// Get start page
		uint8_t pageNo;
		if(stringToInt(i, &pageNo, &i) != STATUS_OK)
		{
			// Do something
			return;
		}

		// Get byteOffset
		uint8_t byteOffset;
		if(stringToInt(i, &byteOffset, &i) != STATUS_OK)
		{
			// Do something
			return;
		}

		// Get size of data in bytes
		uint8_t dataSize;
		if(stringToInt(i, &dataSize, &i) != STATUS_OK)
		{
			// Do something
			return;
		}

		uint8_t sendSize = 0;
		while(dataSize)
		{
			if(i + dataSize > DATA_WIDTH)
			{
				sendSize = DATA_WIDTH - i;

				// Send all the bytes to send in the current frame
//				EEPROM_Write(DEVICE_ADDR, pageNo, byteOffset, 2, frameData + i, sendSize);
				testWrite(frameData + i, sendSize);

				getFrameData(frameData);
				dataSize -= sendSize;
				i = 0;

				// Change the pageNo and byteOffset
				if(byteOffset + sendSize > PAGE_SIZE)
				{
					pageNo++;
					byteOffset = (byteOffset + sendSize) - PAGE_SIZE;
				}
			}
			else
			{
//				EEPROM_Write(DEVICE_ADDR, pageNo, byteOffset, 2, frameData + i, dataSize);
				testWrite(frameData + i, dataSize);
				dataSize = 0;
			}
		}
//		startUARTTx();
	}
}


STATUS stringToInt(uint8_t startIndex, uint8_t* output, uint8_t* outIndex)
{
	uint16_t val = 0;

	// Read till space or escape sequences
	while(frameData[startIndex] != ' ' && frameData[startIndex] != '\r' && frameData[startIndex] != '\n')
	{
		if(frameData[startIndex] > '9' || frameData[startIndex] < '0')
		{
			return STATUS_INVALID_FORMAT;
		}
		val *= 10;
		val += frameData[startIndex++] - '0';

		// If end of frame is reached, go to next frame
		if(startIndex == DATA_WIDTH)
		{
			getFrameData(frameData);
			startIndex = 0;
		}
	}

	*output = val;
	*outIndex = startIndex + 1;

	if(startIndex + 1 >= DATA_WIDTH)
	{
		getFrameData(frameData);
		*outIndex = 0;
	}

	return STATUS_OK;
}

void testWrite(uint8_t* buff, uint8_t Size)
{
	for(int i = 0; i < 100 * Size; i++);
}

void testRead(uint8_t* buff, uint8_t Size)
{
	for(int i = 0; i < Size; i++)
	{
		buff[i] = 'a' + (i % 26);
	}
}
