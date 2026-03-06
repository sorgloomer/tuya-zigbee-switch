#include <stdint.h>
#include <stdbool.h>

#include "./lightshow.h"

#include "hal/timer.h"
#include "hal/tasks.h"
#include "hal/gpio.h"
#include "device_config/config_parser.h"
#include "device_config/components.h"

#define BLINK_DELAY_MS 70

static uint32_t timestamp_section0_end;
static uint32_t timestamp_end;
static uint32_t timestamp_led;
static int index_section;
static int index_led;

static hal_task_t lightshow_task;

static void lightshow_handler(void*);
static void task_schedule_at(hal_task_t* task, uint32_t timestamp);
static int compare_times(uint32_t arg_l, uint32_t arg_r);
static void set_all_leds(uint8_t value);

void lightshow_start() {
    if (leds_cnt <= 0) return;
    index_section = 0;
    index_led = 0;

    uint32_t started = hal_millis();
    timestamp_led = started;
    timestamp_section0_end = started + 500;
    timestamp_end = started + 1000;


    hal_tasks_init(&lightshow_task);
    lightshow_task.handler = lightshow_handler;
    lightshow_handler(NULL);
}

static void schedule_next() {
    timestamp_led += BLINK_DELAY_MS;
    if (compare_times(timestamp_led, timestamp_section0_end) < 0) {
        index_section = 1;
        task_schedule_at(&lightshow_task, timestamp_led);
    } else {
        index_section = 2;
        task_schedule_at(&lightshow_task, timestamp_section0_end);
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
            schedule_next();
            break;
        case 1:
            led_set(&leds[index_led], 0);
            index_led = (index_led + 1) % leds_cnt;
            led_set(&leds[index_led], 1);
            schedule_next();
            break;
        case 2:
            set_all_leds(1);
            index_section = 3;
            task_schedule_at(&lightshow_task, timestamp_end);
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