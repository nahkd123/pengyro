#ifndef _PENGYRO_H
#define _PENGYRO_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/**
 * @brief PenGyro configuration
 *
 * This struct define the confiurations for PenGyro module. The configuration is stored inside the memory and it will be
 * reverted back to defaults upon power reset. Setting invalid values will result in `BT_ATT_ERR_VALUE_NOT_ALLOWED`
 * error.
 *
 * The characteristic UUID is `ee8cf7e0-a370-4fb2-9d16-d1e31ac66051`
 */
struct pengyro_config {
    /**
     * @brief Currently executing command
     *
     * This shows the command that is currently being executed. The commands are defined in `PENGYRO_CMD_*` enum. While
     * the command is being executed, this field will retain the value, and it will automatically set back to
     * `PENGYRO_CMD_IDLE` once the command is finished.
     *
     * Except for `PENGYRO_CMD_IDLE`, while the command is being executed, this field should never be modified.
     */
    uint16_t cmd;
    uint16_t data_rate;
    uint16_t acc_range;
    uint16_t gyr_range;
};

enum : uint8_t {
    PENGYRO_CMD_IDLE = 0x00,
    PENGYRO_CMD_CALIBRATE = 0x01,
    PENGYRO_CMD_MAX
};

/**
 * @brief PenGyro data constants
 *
 * This struct define the constants that will be used in `pengyro_data`. The accelerometer range is measured in G, the
 * gyroscope range is measured in deg/s and the timescale is measured in microseconds (us). Note that the max time value
 * is inclusive (as in the `time` is always less than or equals to `max_time`).
 *
 * The characteristic UUID is `f1563870-f4e5-41ba-8165-7954f2513905`
 */
struct pengyro_consts {
    uint32_t time_max;
    uint16_t time_scale;
};

/**
 * @brief PenGyro data
 *
 * This struct define the data that will be reported to host.
 *
 * - `time` is the absolute time, which wraps around when reaching `time_max` defined in `pengyro_consts`
 * - `delta` is the delta time between last and current data
 * - `acc` is the accelerometer values in -32767 -> 32767 range
 * - `gyr` is the gyroscope values in -32767 -> 32767 range
 *
 * The time values are meant to be scaled by `time_scale` defined in `pengyro_consts` to obtain the time in microseconds
 * (us) and used in calculating the velocity, position and rotation.
 *
 * The characteristic UUID is `d312a6d2-2375-48fb-a333-c413144bc6c8`
 */
struct pengyro_data {
    uint32_t time;
    uint32_t delta;
    int16_t acc[3];
    int16_t gyr[3];
};

/**
 * @brief PenGyro rotation data
 *
 * This typdef define the rotation data that will be reported to host. The value is reported in degrees and wraps around
 * 360deg. In other words, the value is between 0deg (inclusively) and 360deg (exclusively).
 *
 * The characteristic UUID is `270b1d88-ac32-4658-99d3-babd43a2db93`
 */
typedef double pengyro_rotation;

/**
 * @brief Callback on initializing default values
 *
 * This function will be called by PenGyro to initialize default values. Since the values are not stored in NVRAM, this
 * function will always be called on start.
 */
void pengyro_on_init(struct pengyro_consts* consts, struct pengyro_config* config);

/**
 * @brief Callback on setting up
 *
 * This function will be called by PenGyro to initialize the sensor. If this function is failed, the system will attempt
 * to reinitialize everything.
 */
int pengyro_on_setup();

/**
 * @brief Callback on configure
 *
 * This function will be called by PenGyro to reconfigure the sensor. If this function is failed, the system will
 * attempt to reinitialize everything.
 */
int pengyro_on_configure(struct pengyro_config* config);

/**
 * @brief Callback on polling start
 *
 * This function will be called by PenGyro right before `pengyro_on_poll()` is called, or right after the calibration is
 * finished. Use this callback to resume the sensor from suspend mode.
 */
int pengyro_on_poll_start();

/**
 * @brief Callback on polling stop
 *
 * This function will be called by PenGyro when the sensor is no longer needed to be polled. Use this callback to put
 * the sensor to suspend mode.
 */
int pengyro_on_poll_stop();

/**
 * @brief Callback on polling
 *
 * This function will be called by PenGyro when a new data need to be read from sensor. If this function is failed, the
 * system will attempt to reinitialize everything. PenGyro only need data when there is at least 1 Bluetooth connection
 * start listening for data.
 *
 * @returns 0 if successful, non-zero if failed.
 */
int pengyro_on_poll(struct pengyro_data* data);

/**
 * @brief Callback on calibrate
 *
 * This function will be called by PenGyro when the sensor is needed to be calibrated. If this function is failed, the
 * system attempt to reinitialize everything.
 */
int pengyro_on_calibrate();

/**
 * @brief Main function for PenGyro
 *
 * This function is meant to be called in `main()` function of your Zephyr app.
 *
 * @param led The LED indicator device
 */
int pengyro_main();

#endif // _PENGYRO_H