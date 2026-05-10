#ifndef _SENSOR_H
#define _SENSOR_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

struct bmi160_data {
	int16_t mag[3];
	int16_t rhall;
	int16_t gyr[3];
	int16_t acc[3];
	uint32_t time;
};

struct bmi160_offset {
	int16_t acc[3];
	int16_t gyr[3];
};

/**
 * @brief Reset the BMI160 sensor
 *
 * This function reset the BMI160 sensor and reconfigure it with given parameters.
 *
 * @param sensor The sensor devicetree specification
 * @param odr The output data rate for both accelerometer and gyroscope
 * @param acc_range The range of the accelerometer (G), or 0 to disable
 * @param gyr_range The range of the gyroscope (deg/s), or 0 to disable
 * @returns 0 if success, negative value on error
 */
int bmi160_reset(const struct i2c_dt_spec* sensor);

/**
 * @brief Configure the sensor
 *
 * This function configure the sensor with given parameters.
 *
 * @param sensor The sensor devicetree specification
 * @param odr The output data rate for both accelerometer and gyroscope
 * @param acc_range The range of the accelerometer (G), or 0 to disable
 * @param gyr_range The range of the gyroscope (deg/s), or 0 to disable
 * @returns 0 if success, negative value on error
 */
int bmi160_config(const struct i2c_dt_spec* sensor, uint16_t odr, uint16_t acc_range, uint16_t gyr_range);

/**
 * @brief Configure power mode
 *
 * This function configure power mode of accelerometer and gyroscope.
 *
 * @param sensor The sensor devicetree specification
 * @param acc true to put accelerometer to normal mode, false to suspend
 * @param gyr true to put gyroscope to normal mode, false to suspend
 */
int bmi160_pmu(const struct i2c_dt_spec* sensor, bool acc, bool gyr);

/**
 * @brief Poll the sensor
 *
 * This function read the sensor data.
 *
 * @param sensor The sensor devicetree specification
 * @param data The pointer to struct to obtain data
 * @returns 0 if success, negative value on error
 */
int bmi160_poll(const struct i2c_dt_spec* sensor, struct bmi160_data* data);

/**
 * @brief Read the offsets
 *
 * This function read the calibration data stored on sensor.
 *
 * @param sensor The sensor devicetree specification
 * @param data The pointer to struct to obtain offsets
 * @returns 0 if success, negative value on error
 */
int bmi160_offset_get(const struct i2c_dt_spec* sensor, struct bmi160_offset* offset);

/**
 * @brief Calibrate the sensor
 *
 * This function trigger fast offset calibration on the sensor. The calibration data is then stored in offset register.
 * All the possible accelerometer target values are:
 *
 * - `0` (disable axis)
 * - `1` (+1g)
 * - `2` (-1g)
 * - `3` (0g)
 *
 * @param sensor The sensor devicetree specification
 * @param gyr Whether to include gyroscope in calibration
 * @param acc_x Accelerometer X axis target value
 * @param acc_y Accelerometer Y axis target value
 * @param acc_z Accelerometer Z axis target value
 * @returns 0 if success, negative value on error
 */
int bmi160_calibrate(const struct i2c_dt_spec* sensor, bool gyr, int8_t acc_x, int8_t acc_y, int8_t acc_z);

#endif //_SENSOR_H