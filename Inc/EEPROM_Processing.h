/*
 * EEPROM_Processing.h
 *
 *  Created on: Nov 29, 2024
 *      Author: tibin.mathew
 */

/*
 * This file is an interface between UART data and EEPROM.
 * Checks for proper format of the command and then reads or
 * writes based on the command
 * The commands are:
 * 		r <page number> <byte offset> <number of bytes to read>\r\n
 * 		w <page number> <byte offset> <number of bytes to write> <bytes to write>\r\n
 */

#ifndef EEPROM_PROCESSING_H_
#define EEPROM_PROCESSING_H_

#include "T_UART_Protocol.h"
#include "stdio.h"
//#include "I2C_EEPROM.h"

#define DEVICE_ADDR (0b10100000)

void processCommand();


#endif /* EEPROM_PROCESSING_H_ */
