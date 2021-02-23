/*
 * ms5803.h
 *
 *  Created on: Feb 22, 2021
 *      Author: Ravi Kirschner
 */

#ifndef SRC_MS5803_H_
#define SRC_MS5803_H_

#include <stdint.h>
#include <math.h>
#include "stm32f7xx.h"

typedef enum
{
	PRESSURE = 0x00,
	TEMPERATURE = 0x10
} measurement; //whether to measure pressure or temperature

typedef enum
{
	ADC_256  = 0x00,
	ADC_512  = 0x02,
	ADC_1024 = 0x04,
	ADC_2048 = 0x06,
	ADC_4096 = 0x08
}  precision; //what level of precision do we want

#define MS5803_RESET 0x1E // reset command
#define MS5803_ADC_READ 0x00 // ADC read command
#define MS5803_ADC_CONV 0x40 // ADC conversion command
#define MS5803_PROM 0xA0 // Coefficient location
#define MS5803_ADDR 0x77 << 1 //MS5803 Address

uint16_t MS5803_coefficient[6]; //coefficients
HAL_StatusTypeDef MS5803_reset(void *handle); //reset function
HAL_StatusTypeDef MS5803_coeff(void *handle, uint16_t* coeff, uint8_t value); //coefficient function
uint32_t MS5803_ADC(void *handle, measurement type, precision prec); //ADC function
void MS5803_get_values(void *handle, precision prec, float *temperature, float *pressure); //get temperature and pressure.
HAL_StatusTypeDef MS5803_read(void *handle, uint8_t *bufp, uint16_t len); //read from ms5803
HAL_StatusTypeDef MS5803_write(void *handle, uint8_t *bufp, uint16_t len); //write command to ms5803


#endif /* SRC_MS5803_H_ */
