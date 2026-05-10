#ifndef _LED_H
#define _LED_H

enum led_state {
    LED_STATE_IDLE = 0,
    LED_STATE_PAIRING = 1,
    LED_STATE_MAX
};

/**
 * @brief Set LED state
 *
 * Set the new LED indicator state. This also reset the internal states.
 *
 * @param state The state to set
 */
void led_set_state(enum led_state state);

#endif