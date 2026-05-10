#include "led.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);
static enum led_state _led_state = LED_STATE_IDLE;

static void _led_main() {
    if (!gpio_is_ready_dt(&led)) return;
    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE)) return;

    while (1) {
        switch (_led_state) {
            case LED_STATE_PAIRING: {
                gpio_pin_toggle_dt(&led);
                k_sleep(K_MSEC(500));
                continue;
            }
            default: {
                gpio_pin_set_dt(&led, 0);
                k_sleep(K_FOREVER);
                continue;
            }
        }
    }
}

static K_THREAD_DEFINE(
    _led_thread,
    512, // Stack size,
    _led_main,
    NULL, // Userdata 1
    NULL, // Userdata 2
    NULL, // Userdata 3
    15, // Priority
    0, // Options
    0 // Delay
);

void led_set_state(enum led_state state) {
    if (_led_state == state) return;
    _led_state = state;
    k_wakeup(_led_thread);
}