#include "scene_button_cluster.h"

#include "base_components/button.h"
#include "base_components/led.h"
#include "device_config/components.h"
#include "hal/zigbee.h"
#include "zigbee/consts.h"
#include "zigbee/cluster_common.h"

#include <stdint.h>
#include "hal/printf_selector.h"


static void handle_on_press(void*);
static void handle_on_long_press(void*);
static void handle_on_multi_press(void*, uint8_t);
static void handle_on_release(void*);

static zigbee_scene_button_cluster* cluster_by_endpoint[32] = { 0 };

static const uint8_t  multistate_out_of_service = 0;
static const uint8_t  multistate_flags          = 0;
static const uint16_t multistate_num_of_states  = 3;



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

void scene_button_cluster_add_to_endpoint(zigbee_scene_button_cluster* cluster, hal_zigbee_endpoint *endpoint) {
    cluster->endpoint = endpoint->endpoint;
    cluster_by_endpoint[cluster->endpoint] = cluster;

    // Output
    endpoint->clusters[endpoint->cluster_count].cluster_id      = ZCL_CLUSTER_IKEA_TRADFRI_SOMRIG_BUTTON;
    endpoint->clusters[endpoint->cluster_count].attribute_count = 0;
    endpoint->clusters[endpoint->cluster_count].attributes      = (hal_zigbee_attribute*)NULL;
    endpoint->clusters[endpoint->cluster_count].is_server       = 0;
    endpoint->cluster_count++;
}

static void send_event(zigbee_scene_button_cluster* self, uint8_t action_index, uint8_t action_param) {
    uint8_t payload = action_param;
    hal_zigbee_cmd cmd = {
        .endpoint            = self->endpoint,
        .profile_id          = ZCL_HA_PROFILE,
        .cluster_id          = ZCL_CLUSTER_IKEA_TRADFRI_SOMRIG_BUTTON,
        .command_id          = action_index,
        .cluster_specific    = 1,
        .direction           = HAL_ZIGBEE_DIR_CLIENT_TO_SERVER,
        .disable_default_rsp = 1,
        .manufacturer_code   = 0,
        .payload             = &payload,
        .payload_len         = 1,
    };
    hal_zigbee_send_cmd_to_bindings(&cmd);
    printf(
        "Sent zigbee command endpoint=%d cluster=%d command=%d payload=%d\r\n",
        (int)cmd.endpoint, 
        (int)cmd.cluster_id,
        (int)cmd.command_id,
        (int)payload
    );

    // self->multistate_state = button_index * 4 + action_index;
    // hal_zigbee_notify_attribute_changed(
    //     self->endpoint,
    //     ZCL_CLUSTER_MULTISTATE_INPUT_BASIC,
    //     ZCL_ATTR_MULTISTATE_INPUT_PRESENT_VALUE
    // );
}

static void handle_on_press(void* context) {
    zigbee_scene_button_cluster* self = (zigbee_scene_button_cluster*)context;
    led_blink_overwrite(self->led, 500, 300, 1);
    send_event(self, 1, 1);
}

static void handle_on_release(void* context) {
    zigbee_scene_button_cluster* self = (zigbee_scene_button_cluster*)context;
    if (self->led->blink_times_left == 0) {
        led_blink_overwrite(self->led, 250, 300, 1);
    }
    send_event(self, 2, 1);
}

static void handle_on_multi_press(void* context, uint8_t press_count) {
    zigbee_scene_button_cluster* self = (zigbee_scene_button_cluster*)context;
    led_blink_overwrite(self->led, 300, 300, 3);
    send_event(self, 3, press_count);
}

static void handle_on_long_press(void* context) {
    zigbee_scene_button_cluster* self = (zigbee_scene_button_cluster*)context;
    led_blink_overwrite(self->led, 600, 600, 2);
    send_event(self, 4, 1);
}
