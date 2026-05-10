#include "bmi160.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bmi160);

#define BMI160_REG_CHIPID 0x00
#define BMI160_REG_DATA 0x04
#define BMI160_REG_STATUS 0x1B
#define BMI160_REG_ACC_CONF 0x40
#define BMI160_REG_ACC_RANGE 0x41
#define BMI160_REG_GYR_CONF 0x42
#define BMI160_REG_GYR_RANGE 0x43
#define BMI160_REG_FOC_CONF 0x69
#define BMI160_REG_OFFSET 0x71
#define BMI160_REG_CMD 0x7E

#define BMI160_STATUS_FOC_READY 0b00001000
#define BMI160_STATUS_NVM_READY 0b00010000

#define BMI160_FOC_GYR_ENABLE 0b01000000

#define BMI160_ODR_25 0b0110
#define BMI160_ODR_50 0b0111
#define BMI160_ODR_100 0b1000
#define BMI160_ODR_200 0b1001
#define BMI160_ODR_400 0b1010
#define BMI160_ODR_800 0b1011
#define BMI160_ODR_1600 0b1100
#define BMI160_ACC_RANGE_2 0b0011
#define BMI160_ACC_RANGE_4 0b0101
#define BMI160_ACC_RANGE_8 0b1000
#define BMI160_GYR_RANGE_2000 0b000
#define BMI160_GYR_RANGE_1000 0b001
#define BMI160_GYR_RANGE_500 0b010
#define BMI160_GYR_RANGE_250 0b011
#define BMI160_GYR_RANGE_125 0b100
#define BMI160_GYR_CONF_BWP_NORMAL 0b0100000

#define BMI160_CMD_FOC 0x03
#define BMI160_CMD_PMU_ACCELEROMETER(x) (0b00010000 | (x))
#define BMI160_CMD_PMU_GYROSCOPE(x) (0b00010100 | (x))
#define BMI160_CMD_SOFTRESET 0xB6

#define BMI160_PMU_SUSPEND 0b00
#define BMI160_PMU_NORMAL 0b01

#define BMI160_CHIP_ID 0xD1
#define BMI160_TIMESCALE_US 39

int bmi160_reset(const struct i2c_dt_spec* sensor) {
    int err;
    uint8_t chip_id;

    if (!i2c_is_ready_dt(sensor)) return -EBUSY;

    if ((err = i2c_reg_write_byte_dt(sensor, BMI160_REG_CMD, BMI160_CMD_SOFTRESET))) return err;
	k_sleep(K_MSEC(1));

	if ((err = i2c_reg_read_byte_dt(sensor, BMI160_REG_CHIPID, &chip_id))) return err;
	if (chip_id != 0xD1) return -EBADMSG;
	k_sleep(K_MSEC(1));

    return 0;
}

int bmi160_config(const struct i2c_dt_spec* sensor, uint16_t odr, uint16_t acc_range, uint16_t gyr_range) {
    int err;
    uint8_t odr_data, acc_range_data = 0, gyr_range_data = 0;

    switch (odr) {
        case 25: odr_data = BMI160_ODR_25; break;
        case 50: odr_data = BMI160_ODR_50; break;
        case 100: odr_data = BMI160_ODR_100; break;
        case 200: odr_data = BMI160_ODR_200; break;
        case 400: odr_data = BMI160_ODR_400; break;
        case 800: odr_data = BMI160_ODR_800; break;
        case 1600: odr_data = BMI160_ODR_1600; break;
        default:
            LOG_ERR("Invalid ODR: %dHz", odr);
            return -EINVAL;
    }

    switch (acc_range) {
        case 2: acc_range_data = BMI160_ACC_RANGE_2; break;
        case 4: acc_range_data = BMI160_ACC_RANGE_4; break;
        case 8: acc_range_data = BMI160_ACC_RANGE_8; break;
        default:
            LOG_ERR("Invalid accelerometer range: %dG", acc_range);
            return -EINVAL;
    }

    switch (gyr_range) {
        case 125: gyr_range_data = BMI160_GYR_RANGE_125; break;
        case 250: gyr_range_data = BMI160_GYR_RANGE_250; break;
        case 500: gyr_range_data = BMI160_GYR_RANGE_500; break;
        case 1000: gyr_range_data = BMI160_GYR_RANGE_1000; break;
        case 2000: gyr_range_data = BMI160_GYR_RANGE_2000; break;
        default:
            LOG_ERR("Invalid gyroscope range: %ddeg/s", gyr_range);
            return -EINVAL;
    }

    LOG_INF("Configuring sensor with data rate %dHz", odr);

    if (acc_range != 0) {
        LOG_INF("Configuring accelerometer with range +-%dg", acc_range);
        
        if ((err = i2c_reg_write_byte_dt(sensor, BMI160_REG_ACC_CONF, odr_data))) return err;
	    k_sleep(K_MSEC(1));

        if ((err = i2c_reg_write_byte_dt(sensor, BMI160_REG_ACC_RANGE, acc_range_data))) return err;
	    k_sleep(K_MSEC(1));
    }

    if (gyr_range != 0) {
        LOG_INF("Configuring gyroscope with range +-%ddeg/s", gyr_range);

        if ((err = i2c_reg_write_byte_dt(sensor, BMI160_REG_GYR_CONF, odr_data))) return err;
	    k_sleep(K_MSEC(1));

        if ((err = i2c_reg_write_byte_dt(sensor, BMI160_REG_GYR_RANGE, gyr_range_data))) return err;
	    k_sleep(K_MSEC(1));
    }

    return 0;
}

int bmi160_pmu(const struct i2c_dt_spec* sensor, bool acc, bool gyr) {
    int err;

    if ((err = i2c_reg_write_byte_dt(sensor, BMI160_REG_CMD, BMI160_CMD_PMU_ACCELEROMETER(acc ? BMI160_PMU_NORMAL : BMI160_PMU_SUSPEND)))) return err;
    k_sleep(K_MSEC(10));

    if ((err = i2c_reg_write_byte_dt(sensor, BMI160_REG_CMD, BMI160_CMD_PMU_GYROSCOPE(gyr ? BMI160_PMU_NORMAL : BMI160_PMU_SUSPEND)))) return err;
    k_sleep(K_MSEC(100));

    LOG_INF("Reconfigured power mode");
    return 0;
}

int bmi160_poll(const struct i2c_dt_spec* sensor, struct bmi160_data* data) {
    int err;
	uint8_t reg_data = BMI160_REG_DATA;
    if ((err = i2c_write_read_dt(sensor, &reg_data, 1, data, sizeof(struct bmi160_data) - 1))) return err;
    return 0;
}

int bmi160_offset_get(const struct i2c_dt_spec* sensor, struct bmi160_offset* offset) {
    int err;
    uint8_t reg_data = BMI160_REG_OFFSET;
	if ((err = i2c_write_read_dt(sensor, &reg_data, 1, offset, sizeof(struct bmi160_offset)))) return err;
	k_sleep(K_MSEC(1));
    return 0;
}

int bmi160_calibrate(const struct i2c_dt_spec* sensor, bool gyr, int8_t acc_x, int8_t acc_y, int8_t acc_z) {
    int err;
	uint8_t status;
    uint8_t data = (gyr ? BMI160_FOC_GYR_ENABLE : 0) | (acc_x << 4) | (acc_y << 2) | acc_z;

    if ((err = i2c_reg_write_byte_dt(sensor, BMI160_REG_FOC_CONF, data))) return err;
	k_sleep(K_MSEC(1));

	if ((err = i2c_reg_write_byte_dt(sensor, BMI160_REG_CMD, BMI160_CMD_FOC))) return err;
	k_sleep(K_MSEC(1));

    while (1) {
		if ((err = i2c_reg_read_byte_dt(sensor, BMI160_REG_STATUS, &status))) return err;
		k_sleep(K_MSEC(20));
		if (status & BMI160_STATUS_FOC_READY) break;
	}

    return 0;
}
