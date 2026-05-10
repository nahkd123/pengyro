#include "pengyro.h"
#include "led.h"

#include <math.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>

LOG_MODULE_REGISTER(pengyro);

#define PENGYRO_BT_UUID_SERVICE BT_UUID_128_ENCODE(0x217a5545, 0x9217, 0x403b, 0xa2af, 0xaf04cf7fad88)
#define PENGYRO_BT_UUID_CHAR_CONFIG BT_UUID_128_ENCODE(0xee8cf7e0, 0xa370, 0x4fb2, 0x9d16, 0xd1e31ac66051)
#define PENGYRO_BT_UUID_CHAR_CONSTS BT_UUID_128_ENCODE(0xf1563870, 0xf4e5, 0x41ba, 0x8165, 0x7954f2513905)
#define PENGYRO_BT_UUID_CHAR_DATA BT_UUID_128_ENCODE(0xd312a6d2, 0x2375, 0x48fb, 0xa333, 0xc413144bc6c8)
#define PENGYRO_BT_UUID_CHAR_ROTATION BT_UUID_128_ENCODE(0x270b1d88, 0xac32, 0x4658, 0x99d3, 0xbabd43a2db93)

static const struct bt_uuid_128 _pengyro_bt_uuid_service = BT_UUID_INIT_128(PENGYRO_BT_UUID_SERVICE);
static const struct bt_uuid_128 _pengyro_bt_uuid_char_config = BT_UUID_INIT_128(PENGYRO_BT_UUID_CHAR_CONFIG);
static const struct bt_uuid_128 _pengyro_bt_uuid_char_consts = BT_UUID_INIT_128(PENGYRO_BT_UUID_CHAR_CONSTS);
static const struct bt_uuid_128 _pengyro_bt_uuid_char_data = BT_UUID_INIT_128(PENGYRO_BT_UUID_CHAR_DATA);
static const struct bt_uuid_128 _pengyro_bt_uuid_char_rotation = BT_UUID_INIT_128(PENGYRO_BT_UUID_CHAR_ROTATION);

static bool _pengyro_notify_data = false;
static bool _pengyro_notify_rotation = false;
static struct pengyro_consts _pengyro_consts;
static struct pengyro_config _pengyro_config = { .cmd = PENGYRO_CMD_IDLE };

static void _pengyro_bt_on_data_ccc_config_changed(const struct bt_gatt_attr* attr, uint16_t value) {
    _pengyro_notify_data = value == BT_GATT_CCC_NOTIFY;
    LOG_INF("Data CCC configuration changed to %d", _pengyro_notify_data);
}

static void _pengyro_bt_on_rotation_ccc_config_changed(const struct bt_gatt_attr* attr, uint16_t value) {
    _pengyro_notify_rotation = value == BT_GATT_CCC_NOTIFY;
    LOG_INF("Rotation CCC configuration changed to %d", _pengyro_notify_rotation);
}

static ssize_t _pengyro_bt_on_config_read(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf, uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &_pengyro_config, sizeof(_pengyro_config));
}

static ssize_t _pengyro_bt_on_config_write(struct bt_conn* conn, const struct bt_gatt_attr* attr, const void* buf, uint16_t len, uint16_t offset, uint8_t flags) {
    int err;

    if (len < sizeof(uint16_t) || len > sizeof(struct pengyro_config)) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    if (offset != 0) return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);

    uint16_t cmd = *((uint16_t*)buf);
    if (cmd != _pengyro_config.cmd && _pengyro_config.cmd != PENGYRO_CMD_IDLE) return BT_GATT_ERR(BT_ATT_ERR_WRITE_REQ_REJECTED);
    if (cmd >= PENGYRO_CMD_MAX) return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    _pengyro_config.cmd = cmd;

    #ifdef CONFIG_LOG
    if (cmd != PENGYRO_CMD_IDLE) LOG_INF("Executing command: 0x%02x", cmd);
    #endif

    if (len == sizeof(struct pengyro_config)) {
        struct pengyro_config* config = (struct pengyro_config*)buf;
        bool changed = config->data_rate != _pengyro_config.data_rate;
        changed |= config->acc_range != _pengyro_config.acc_range;
        changed |= config->gyr_range != _pengyro_config.gyr_range;

        if (changed) {
            if ((err = pengyro_on_configure(config))) {
                LOG_ERR("Error while configuring sensor (%d)", err);
                return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
            }

            _pengyro_config.data_rate = config->data_rate;
            _pengyro_config.acc_range = config->acc_range;
            _pengyro_config.gyr_range = config->gyr_range;

            LOG_INF("Reconfigured device");
        }
    }

    return len;
}

static ssize_t _pengyro_bt_on_consts_read(struct bt_conn* conn, const struct bt_gatt_attr* attr, void* buf, uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &_pengyro_consts, sizeof(_pengyro_consts));
}

static BT_GATT_SERVICE_DEFINE(_pengyro_bt_service,
    BT_GATT_PRIMARY_SERVICE(&_pengyro_bt_uuid_service),
    BT_GATT_CHARACTERISTIC(&_pengyro_bt_uuid_char_config.uuid, BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, _pengyro_bt_on_config_read, _pengyro_bt_on_config_write, NULL),
    BT_GATT_CHARACTERISTIC(&_pengyro_bt_uuid_char_consts.uuid, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, _pengyro_bt_on_consts_read, NULL, NULL),
    BT_GATT_CHARACTERISTIC(&_pengyro_bt_uuid_char_data.uuid, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
    BT_GATT_CCC(_pengyro_bt_on_data_ccc_config_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(&_pengyro_bt_uuid_char_rotation.uuid, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
    BT_GATT_CCC(_pengyro_bt_on_rotation_ccc_config_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL,
        PENGYRO_BT_UUID_SERVICE
    )
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, (sizeof(CONFIG_BT_DEVICE_NAME) - 1))
};

static const struct bt_le_adv_param* adv_param = BT_LE_ADV_PARAM(
    (BT_LE_ADV_OPT_CONN | BT_LE_ADV_OPT_USE_IDENTITY),
    800, // Min interval 500ms
    801, // Max interval 500.625ms
    NULL // Undirected advertising
);

static void _pengyro_bt_on_advertise_work(struct k_work* work) {
    int err;
    
    if ((err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd)))) {
        LOG_ERR("Error while advertising (%d)", err);
        return;
    }

    LOG_INF("Now advertising as %s", CONFIG_BT_DEVICE_NAME);
    led_set_state(LED_STATE_PAIRING);
}

static K_WORK_DEFINE(_pengyro_advertise_work, _pengyro_bt_on_advertise_work);

static void _pengyro_bt_advertise() {
    k_work_submit(&_pengyro_advertise_work);
}

static void _pengyro_bt_on_connected(struct bt_conn* conn, uint8_t err) {
    led_set_state(LED_STATE_IDLE);
}

static void _pengyro_bt_on_conn_recycled() {
    LOG_INF("A new connection slot is available, starting advertising");
    _pengyro_bt_advertise();
}

BT_CONN_CB_DEFINE(_pengyro_bt_callbacks) = {
    .connected = _pengyro_bt_on_connected,
    .recycled = _pengyro_bt_on_conn_recycled
};

// Main thread
int pengyro_main() {
    int err;
    struct pengyro_data data;
    double rotation = 0;

    pengyro_on_init( &_pengyro_consts, &_pengyro_config);
    LOG_INF("Loaded default values");

    if ((err = bt_enable(NULL))) {
        LOG_ERR("Error while enabling Bluetooth (%d). This is a fatal error.", err);
        return 1;
    } else {
        LOG_INF("Enabled Bluetooth!\n");
    }

    const struct bt_gatt_attr* data_attr = bt_gatt_find_by_uuid(
        _pengyro_bt_service.attrs,
        _pengyro_bt_service.attr_count,
        &_pengyro_bt_uuid_char_data.uuid
    );

    if (data_attr == NULL) {
        LOG_ERR("Data attribute is not defined. This is a fatal error.");
        return 2;
    }

    const struct bt_gatt_attr* rotation_attr = bt_gatt_find_by_uuid(
        _pengyro_bt_service.attrs,
        _pengyro_bt_service.attr_count,
        &_pengyro_bt_uuid_char_rotation.uuid
    );

    if (rotation_attr == NULL) {
        LOG_ERR("Rotation attribute is not defined. This is a fatal error.");
        return 2;
    }

    _pengyro_bt_advertise();

    while (1) {
        bool last_polling = false;

        k_sleep(K_MSEC(500));
        LOG_INF("Initializing sensor...");

        if ((err = pengyro_on_setup())) {
            LOG_ERR("Error while setting up sensor (%d), restarting", err);
            continue;
        }

        if ((err = pengyro_on_configure(&_pengyro_config))) {
            LOG_ERR("Error while configuring sensor (%d), restarting", err);
            continue;
        }

        LOG_INF("Sensor is initialized!");

        while (1) {
            bool polling = _pengyro_notify_data || _pengyro_notify_rotation;

            // Commands
            if (_pengyro_config.cmd == PENGYRO_CMD_CALIBRATE) {
                if (!last_polling && !polling) {
                    LOG_INF("Resuming sensor for calibration...");

                    if ((err = pengyro_on_poll_start())) {
                        LOG_ERR("Sensor resuming failed (%d), restarting", err);
                        break;
                    }
                }

                if ((err = pengyro_on_calibrate())) {
                    LOG_ERR("Sensor calibration failed (%d), restarting", err);
                    break;
                }

                _pengyro_config.cmd = PENGYRO_CMD_IDLE;

                if (!last_polling && !polling) {
                    LOG_INF("Suspending sensor after calibration...");

                    if ((err = pengyro_on_poll_stop())) {
                        LOG_ERR("Sensor suspending failed (%d)", err);
                    }
                }
            }

            // Polling data
            if (last_polling ^ polling) {
                LOG_INF("Polling state changed");
                last_polling = polling;

                if (polling) {
                    if ((err = pengyro_on_poll_start())) {
                        LOG_ERR("Sensor resuming failed (%d), restarting", err);
                        break;
                    }
                } else {
                    if ((err = pengyro_on_poll_stop())) {
                        LOG_ERR("Sensor suspending failed (%d)\n", err);
                    }
                }
            }

            if (polling) {
                if ((err = pengyro_on_poll(&data))) {
                    LOG_ERR("Sensor poll failed (%d), restarting", err);
                    break;
                }
            } else {
                // TODO: Wake the thread up from sleep
                k_sleep(K_MSEC(100));
                continue;
            }

            // Processing & notifying
            if (_pengyro_notify_data) {
                if ((err = bt_gatt_notify(NULL, data_attr, &data, sizeof(struct pengyro_data)))) {
                    LOG_WRN("Bluetooth GATT notify failed (%d)", err);
                }
            }

            if (_pengyro_notify_rotation) {
                double gyr_y = (double)data.gyr[1] * _pengyro_config.gyr_range / 32767;
                double rotated = gyr_y * data.delta * _pengyro_consts.time_scale / 1e6;
                rotation = fmod(rotation + rotated, 360.0);
                if (rotation < 0) rotation += 360.0;

                if ((err = bt_gatt_notify(NULL, rotation_attr, &rotation, sizeof(rotation)))) {
                    LOG_WRN("Bluetooth GATT notify failed (%d)", err);
                }
            }
        }
    }

    return 0;
}
