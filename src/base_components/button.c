#include "button.h"
#include "hal/printf_selector.h"
#include "hal/tasks.h"
#include "hal/timer.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static void _btn_gpio_callback(hal_gpio_pin_t pin, void *arg);
static void _btn_debounce_callback(void *arg);
static void _btn_hold_callback(void *arg);
static void _btn_fire_universal_event(button_t* self,
                                      btn_event_t event);

void btn_init_before(button_t *self) {
    memset(self, sizeof(button_t), 0);
    self->long_press_duration_ms  = 600;
    self->multi_press_duration_ms = 600;
}

void btn_init_after(button_t *button) {
    // During device startup, button may be already pressed, but this should not
    // be detected as user press. So, to avoid such situation, special init is
    // required.
    uint8_t initial_level = hal_gpio_read(button->pin);

    uint8_t initial_pressed = initial_level == button->pressed_when_high;
    button->pressed             = initial_pressed;
    button->long_pressed        = initial_pressed; // TODO: are we sure? shouldn't we store it in nv?
    button->long_released       = !initial_pressed; // TODO: are we sure? shouldn't we store it in nv?
    button->debounce_last_level = initial_level;

    hal_tasks_init(&button->timer_debounce);
    button->timer_debounce.handler = _btn_debounce_callback;
    button->timer_debounce.arg     = button;
    hal_tasks_init(&button->timer_hold);
    button->timer_hold.handler = _btn_hold_callback;
    button->timer_hold.arg     = button;

    hal_gpio_callback(button->pin, _btn_gpio_callback, button);
}

static void _btn_gpio_callback(hal_gpio_pin_t pin, void *arg) {
    button_t *button    = (button_t *)arg;
    uint8_t   new_level = hal_gpio_read(button->pin);

    if (new_level == button->debounce_last_level) {
        return;
    }

    hal_tasks_unschedule(&button->timer_debounce);
    button->debounce_last_level  = new_level;
    button->debounce_last_change = hal_millis();
    hal_tasks_schedule(&button->timer_debounce, DEBOUNCE_DELAY_MS);
    printf("Button value changed to %d\r\n", button->debounce_last_level);
}

static void _btn_debounce_callback(void *arg) {
    button_t *button = (button_t *)arg;

    uint8_t is_pressed = button->debounce_last_level == button->pressed_when_high;
    uint32_t changed_at = button->debounce_last_change;

    uint8_t previous_long_pressed = button->long_pressed;
    button->pressed               = is_pressed;
    button->event_level_held           = 0;
    if (is_pressed) {
        printf("Press detected\r\n");
        button->multi_press_cnt += 1;
        button->pressed_at_ms = changed_at;
        button->long_pressed  = false;
        if (button->on_press != NULL) {
            button->on_press(button->callback_param);
        }
        if (button->multi_press_cnt > 1) {
            printf("Multi press detected: %d\r\n", button->multi_press_cnt);
            if (button->on_multi_press != NULL) {
                button->on_multi_press(button->callback_param, button->multi_press_cnt);
            }
        }
    } else {
        printf("Release detected\r\n");
        button->released_at_ms = changed_at;
        button->long_released  = false;
        if (button->on_release != NULL) {
            button->on_release(button->callback_param);
        }
    }

    _btn_fire_universal_event(button, is_pressed ? BTN_EVENT_EDGE_PRESSED : BTN_EVENT_EDGE_RELEASED);

    uint32_t already_was_long_for = hal_millis() - changed_at;
    hal_tasks_unschedule(&button->timer_hold);
    hal_tasks_schedule(&button->timer_hold,
                       already_was_long_for < button->long_press_duration_ms
                         ? button->long_press_duration_ms - already_was_long_for
                         : 0);
}

static void _btn_hold_callback(void *arg) {
    button_t *button = (button_t *)arg;
    uint8_t is_pressed = button->debounce_last_level == button->pressed_when_high;

    button->event_level_held = 1;
    if (is_pressed) {
        printf("Long press detected\r\n");
        button->long_pressed = true;
        if (button->on_long_press != NULL) {
            button->on_long_press(button->callback_param);
        }
    } else {
        button->long_released = true;
    }
    _btn_fire_universal_event(button, is_pressed ? BTN_EVENT_HOLD_PRESSED : BTN_EVENT_HOLD_RELEASED);
    if (!is_pressed) {
        button->multi_press_cnt = 0;
    }
}

static void _btn_fire_universal_event(button_t* self, btn_event_t event) {
    if (self->on_event != NULL) {
        self->on_event(self->callback_param, event);
    }
}
