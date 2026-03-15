#include <stdint.h>
#include <stdbool.h>

#include "./lightshow.h"

#include "hal/timer.h"
#include "hal/tasks.h"
#include "hal/gpio.h"
#include "device_config/config_parser.h"
#include "device_config/components.h"

#define DELAY_MS_ROUNDROBIN 70
#define DELAY_MS_BLINK 500

static uint32_t timestamp_section0_end;
static uint32_t timestamp_end;
static uint32_t timestamp_led;
static int index_section;
static int index_led;
static uint8_t blink_state;

static hal_task_t lightshow_task;
static uint8_t lightshow_task_initialized = 0;

static void lightshow_handler(void*);
static void task_schedule_at(hal_task_t* task, uint32_t timestamp);
static int compare_times(uint32_t arg_l, uint32_t arg_r);
static void set_all_leds(uint8_t value);

void lightshow_init() {

}
void lightshow_start(uint32_t duration_ms) {
    if (lightshow_task_initialized) {
        hal_tasks_unschedule(&lightshow_task);
    } else {
        hal_tasks_init(&lightshow_task);
        lightshow_task.handler = lightshow_handler;
        lightshow_task_initialized = 1;
    }

    if (leds_cnt <= 0) return;

    index_section = 0;
    index_led = 0;

    uint32_t started = hal_millis();
    timestamp_led = started;
    timestamp_section0_end = started + duration_ms / 2;
    timestamp_end = started + duration_ms;

    lightshow_handler(NULL);
}

static void schedule_roundrobin_next() {
    timestamp_led += DELAY_MS_ROUNDROBIN;
    if (compare_times(timestamp_led, timestamp_section0_end) < 0) {
        index_section = 1;
        task_schedule_at(&lightshow_task, timestamp_led);
    } else {
        index_section = 2;
        blink_state = 0;
        task_schedule_at(&lightshow_task, timestamp_section0_end);
    }
}

static void schedule_blink_next() {
    timestamp_led += DELAY_MS_BLINK;
    if (compare_times(timestamp_led, timestamp_end) < 0) {
        index_section = 2;
        task_schedule_at(&lightshow_task, timestamp_led);
    } else {
        index_section = 3;
        task_schedule_at(&lightshow_task, timestamp_end);
    }
}

static void task_schedule_at(hal_task_t* task, uint32_t timestamp) {
    int32_t delay = timestamp - hal_millis();
    hal_tasks_schedule(task, (delay < 1) ? 1 : (uint32_t)delay);
}

static void lightshow_handler(void* _) {
    switch (index_section) {
        case 0:
            set_all_leds(0);
            led_set(&leds[index_led], 1);
            schedule_roundrobin_next();
            break;
        case 1:
            led_set(&leds[index_led], 0);
            index_led = (index_led + 1) % leds_cnt;
            led_set(&leds[index_led], 1);
            schedule_roundrobin_next();
            break;
        case 2:
            blink_state = !blink_state;
            set_all_leds(blink_state);
            schedule_blink_next();
            break;
        case 3:
            set_all_leds(0);
            index_section = 4;
            break;
    }
}

static void set_all_leds(uint8_t value) {
    for (int idx = 0; idx < leds_cnt; idx++) {
        led_set(&leds[idx], value);
    }
}

static int compare_times(uint32_t arg_l, uint32_t arg_r) {
    int32_t diff = arg_l - arg_r;
    return (diff > 0) - (diff < 0);
}