#include "scene_button_cluster.h"

#include "base_components/button.h"
#include "base_components/led.h"
#include "device_config/components.h"
#include "hal/zigbee.h"
#include <stdint.h>

static void handle_on_press(void*);
static void handle_on_long_press(void*);
static void handle_on_multi_press(void*, uint8_t);
static void handle_on_release(void*);

zigbee_scene_button_cluster* scene_button_cluster_new(button_t* button, led_t * led) {
    if (button == NULL) return NULL;
    if (scene_button_clusters_cnt >= COMPONENTS_MAX_SCENE_BUTTON_CLUSTER_COUNT) {
        return NULL;
    }
    button->long_press_duration_ms  = 500;
    button->multi_press_duration_ms = 500;

    zigbee_scene_button_cluster* self = &scene_button_clusters[scene_button_clusters_cnt++];

    self->button = button;
    self->led = led;
    self->scene_button_index = self - scene_button_clusters;

    button->on_press = (ev_button_callback_t)handle_on_press;
    button->on_long_press = (ev_button_callback_t)handle_on_long_press;
    button->on_multi_press = (ev_button_multi_press_callback_t)handle_on_multi_press;
    button->on_release = (ev_button_callback_t)handle_on_release;
    button->callback_param = self;
    return self;
}

static void handle_on_press(void* context) {
    zigbee_scene_button_cluster* self = (zigbee_scene_button_cluster*)context;
    led_blink_overwrite(self->led, 500, 300, 1);
}

static void handle_on_release(void* context) {
    zigbee_scene_button_cluster* self = (zigbee_scene_button_cluster*)context;
    if (self->led->blink_times_left == 0) {
        led_blink_overwrite(self->led, 250, 300, 1);
    }
}

static void handle_on_multi_press(void* context, uint8_t press_count) {
    zigbee_scene_button_cluster* self = (zigbee_scene_button_cluster*)context;
    led_blink_overwrite(self->led, 300, 300, 3);
}

static void handle_on_long_press(void* context) {
    zigbee_scene_button_cluster* self = (zigbee_scene_button_cluster*)context;
    led_blink_overwrite(self->led, 600, 600, 2);
}
