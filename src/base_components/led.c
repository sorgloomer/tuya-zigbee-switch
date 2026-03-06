#include "led.h"

#include "hal/gpio.h"
#include "hal/tasks.h"
#include "hal/timer.h"

#include <stdbool.h>
#include <stdio.h>

static void _led_set_but_do_not_reset_blink(led_t *led, uint8_t value);
static void _led_blink_overwrite(led_t *led, uint16_t on_time_ms, uint16_t off_time_ms,
               uint16_t times);

void led_init(led_t *led) {
    led_off(led);
}

void led_set(led_t *led, uint8_t value) {
    _led_set_but_do_not_reset_blink(led, value);
    led->blink_times_left = 0;
}

void led_on(led_t *led) {
    led_set(led, 1);
}

void led_off(led_t *led) {
    led_set(led, 0);
}

static void _led_set_but_do_not_reset_blink(led_t *led, uint8_t value) {
    value = !!value;
    uint8_t on_high = !!led->on_high;
    hal_gpio_write(led->pin, on_high == value);
    led->on = value;
}

static void led_blink_handler(void *arg) {
    led_t *led = (led_t *)arg;

    if (led->blink_times_left == 0)
        return;

    if (led->on) {
        _led_set_but_do_not_reset_blink(led, 0);
        if (led->blink_times_left != LED_BLINK_FOREVER) {
            led->blink_times_left--;
        }
        hal_tasks_schedule(&led->blink_task, led->blink_time_off);
    } else {
        _led_set_but_do_not_reset_blink(led, 1);
        hal_tasks_schedule(&led->blink_task, led->blink_time_on);
    }
}

void led_blink(led_t *led, uint16_t on_time_ms, uint16_t off_time_ms,
               uint16_t times) {
    if (led->blink_times_left != 0) {
        led->blink_times_left = times;
        return;
    }

    _led_blink_overwrite(led, on_time_ms, off_time_ms, times);
}

void led_blink_overwrite(led_t *led, uint16_t on_time_ms, uint16_t off_time_ms,
               uint16_t times) {
    if (led->blink_times_left != 0) {
        hal_tasks_unschedule(&led->blink_task);
    }
    _led_blink_overwrite(led, on_time_ms, off_time_ms, times);
}

static void _led_blink_overwrite(led_t *led, uint16_t on_time_ms, uint16_t off_time_ms,
               uint16_t times) {
    _led_set_but_do_not_reset_blink(led, 1);
    led->blink_time_on      = on_time_ms;
    led->blink_time_off     = off_time_ms;
    led->blink_times_left   = times;
    led->blink_task.handler = led_blink_handler;
    led->blink_task.arg     = led;
    hal_tasks_init(&led->blink_task);
    hal_tasks_schedule(&led->blink_task, on_time_ms);
}
