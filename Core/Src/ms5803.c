/*
 * ms5803.c
 *
 *  Created on: Feb 22, 2021
 *      Author: Ravi Kirschner
 */

#include "ms5803.h"

/**
 * @brief Reads from MS5803
 * @param handle The I2C handle being used
 * @param bufp The buffer to be read into
 * @param len The length of the buffer in 8-bit increments
 * @retval HAL Status
 */
HAL_StatusTypeDef MS5803_read(void *handle, uint8_t *bufp, uint16_t len) {
	return HAL_I2C_Master_Receive(handle, MS5803_ADDR, bufp, len, 1000);
}

/**
 * @brief Writes to MS5803
 * @param handle The I2C handle being used
 * @param bufp The buffer to read from
 * @param len The length of the buffer in 8-bit increments
 * @retval HAL Status
 */
HAL_StatusTypeDef MS5803_write(void *handle, uint8_t *bufp, uint16_t len) {
	return HAL_I2C_Master_Transmit(handle, MS5803_ADDR, bufp, len, 1000);
}

/**
 * @brief Resets the MS5803
 * @param handle The I2C Handle being used
 * @retval HAL Status
 */
HAL_StatusTypeDef MS5803_reset(void *handle) {
	uint8_t buf[12];
	buf[0] = MS5803_RESET;
	HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(handle, MS5803_ADDR, buf, 1, 1000);
	HAL_Delay(3);
	return ret;
}

/**
 * @brief Gets the 6 Coefficients from the MS5803 and reads them into the MS5803_coefficient array.
 * @param handle The I2C Handle being used
 * @param coeff The pointer to the coefficient being read in to
 * @param value The coefficient number
 * @return HAL Status
 */
HAL_StatusTypeDef MS5803_coeff(void *handle, uint16_t *coeff, uint8_t value) {
	uint8_t buf[12];
	buf[0] = MS5803_PROM + (value << 1); //coefficient to read
	HAL_StatusTypeDef x = MS5803_write(handle, buf, 1); //tell MS5803 that we want it
	HAL_Delay(2); //delay until it is ready
	uint8_t c[2];
	x = MS5803_read(handle, c, 2); //read the coefficient
	*coeff = (c[0] << 8) + c[1]; //turn the two 8-bit values into one coherent value.
	return x;
}

/**
 * @brief Reads the MS5803 ADC
 * @param handle The I2C Handle being used
 * @param type The measurement type, chosen from measurement enum
 * @param prec The precision to use, chosen from precision enum
 * @retval Raw 24-bit data from the ADC
 */
uint32_t MS5803_ADC(void *handle, measurement type, precision prec) {
	uint32_t result;
	uint8_t buf[12];
	buf[0] = MS5803_ADC_CONV + type + prec; //tell the ADC to convert along with the precision and type
	MS5803_write(handle, buf, 1);
	HAL_Delay(1);
	switch(prec) {
		case ADC_256: HAL_Delay(1);
		case ADC_512: HAL_Delay(3);
		case ADC_1024: HAL_Delay(4);
		case ADC_2048: HAL_Delay(6);
		case ADC_4096: HAL_Delay(10); //Delay longer if higher precision, as conversion takes longer.
	}
	buf[0] = MS5803_ADC_READ; //Tell the MS5803 that we want to read the ADC
	MS5803_write(handle, buf, 1);
	HAL_Delay(2);
	uint8_t c[3];
	MS5803_read(handle, c, 3); //Read out the ADC
	result = (c[0] << 16) + (c[1] << 8) + c[2]; //Convert the three 8-bit values into one value.
	return result;
}

/**
 * @brief Gets temperature and pressure values from the MS5803
 * @param handle The I2C Handle being used
 * @param prec The precision to be used
 * @param temperature The pointer to the temperature variable being read in to.
 * @param pressure The pointer to the pressure variable being read in to.
 */
void MS5803_get_values(void *handle, precision prec, float *temperature, float *pressure) {
	uint32_t temperature_raw = MS5803_ADC(handle, TEMPERATURE, prec);
	uint32_t pressure_raw = MS5803_ADC(handle, PRESSURE, prec); //get temperature and pressure raw values

	int32_t sub = MS5803_coefficient[4] * 256;
	int32_t dT = temperature_raw - sub;
	int32_t temp = 2000 + (dT*MS5803_coefficient[5])/(8388608);
	*temperature = temp/100.f; //determine temperature according to datasheet

	int64_t add = ((int64_t)MS5803_coefficient[3])*((int64_t)(temperature_raw - sub))/128;
	int64_t OFF = ((int64_t)MS5803_coefficient[1])*65536+add;
	int64_t SENS = MS5803_coefficient[0] * (32768) + (MS5803_coefficient[2]*dT)/(256);
	int64_t mult = pressure_raw*SENS/2097152;
	int32_t pres = (mult-OFF)/32768;
	*pressure = pres/10.0f; //determine pressure according to datasheet
}
