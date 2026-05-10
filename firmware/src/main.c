#include "bmi160.h"
#include "pengyro.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

// Connect guide for I2C0
// SCL: GPIO 0.11
// SDA: GPIO 1.00

static const struct i2c_dt_spec sensor = I2C_DT_SPEC_GET(DT_NODELABEL(bmi160));
static uint32_t last_sensortime;

int main(void) {
	LOG_INF("Hello there!");
	return pengyro_main();
}

void pengyro_on_init(struct pengyro_consts* consts, struct pengyro_config* config) {
	consts->time_max = 0xFFFFFF;
	consts->time_scale = 39;
	config->data_rate = 1600;
	config->acc_range = 8;
	config->gyr_range = 2000;
}

int pengyro_on_setup() {
	int err;
	if ((err = bmi160_reset(&sensor))) return err;
	return 0;
}

int pengyro_on_configure(struct pengyro_config* config) {
	int err;
	if ((err = bmi160_config(&sensor, config->data_rate, config->acc_range, config->gyr_range))) return err;
	return 0;
}

int pengyro_on_calibrate() {
	int err;
	struct bmi160_offset offset;

	if ((err = bmi160_calibrate(&sensor, true, 0, 0, 0))) return err;
	k_sleep(K_MSEC(1));

	#ifdef CONFIG_LOG
	if ((err = bmi160_offset_get(&sensor, &offset))) {
		LOG_ERR("Error while trying to log offsets");
	} else {
		LOG_INF("Calibration finished!");
		LOG_INF("  Accelerometer: %d / %d / %d", offset.acc[0], offset.acc[1], offset.acc[2]);
		LOG_INF("  Gyroscope:     %d / %d / %d", offset.gyr[0], offset.gyr[1], offset.gyr[2]);
	}
	#endif

	k_sleep(K_MSEC(1));
	return 0;
}

int pengyro_on_poll_start() {
	int err;
	struct bmi160_data bmi160;
	if ((err = bmi160_pmu(&sensor, true, true))) return err;
	if ((err = bmi160_poll(&sensor, &bmi160))) return err;
	last_sensortime = bmi160.time;
	return 0;
}

int pengyro_on_poll(struct pengyro_data *data) {
	int err;
	struct bmi160_data bmi160;
	if ((err = bmi160_poll(&sensor, &bmi160))) return err;

	data->time = bmi160.time;
	data->delta = bmi160.time > last_sensortime ? bmi160.time - last_sensortime : 0xFFFFFF - last_sensortime + bmi160.time + 1;
	
	for (size_t axis = 0; axis < 3; axis++) {
		data->acc[axis] = bmi160.acc[axis];
		data->gyr[axis] = bmi160.gyr[axis];
	}

	last_sensortime = bmi160.time;
	return 0;
}

int pengyro_on_poll_stop() {
	int err;
	if ((err = bmi160_pmu(&sensor, false, false))) return err;
	return 0;
}
