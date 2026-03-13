#include "scene_button_cluster.h"

#include "base_components/button.h"
#include "base_components/led.h"
#include "device_config/components.h"
#include "hal/zigbee.h"
#include "zigbee/consts.h"
#include "zigbee/cluster_common.h"

#include <stdint.h>
#include "hal/printf_selector.h"


static void handle_on_universal(void*, btn_event_t);
static void emulate_somrig_actions(zigbee_scene_button_cluster* self);

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

    zigbee_scene_button_cluster_init(self);
    self->button = button;
    self->led = led;
    self->scene_button_index = self - scene_button_clusters;

    button->on_event = handle_on_universal;
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
}

static void handle_on_universal(void* context, btn_event_t evt) {
    zigbee_scene_button_cluster* self = (zigbee_scene_button_cluster*)context;
    if (self->settings.somrig_actions_enabled) {
        emulate_somrig_actions(self);
    }
}

static void emulate_somrig_actions(zigbee_scene_button_cluster* self) {
    button_t* btn = self->button;

    uint8_t zb_evt = 0xff;
    uint8_t led_should_blink = (self->led != NULL) && self->settings.led_enabled;
    uint8_t long_in_current_state = btn->long_either;
    uint8_t currently_down = btn->pressed;
    uint8_t previous_down_was_long = btn->long_pressed;

    if (currently_down && !long_in_current_state) {
        self->somrig_press_cnt ^= 1;
    }
    uint8_t press_count = 2 - (self->somrig_press_cnt & 1);

    if (currently_down && !long_in_current_state && press_count == 1) {
        if (led_should_blink) {
            led_blink_overwrite(self->led, 500, 300, 1);
        }
        zb_evt = ZIGBEE_SOMRIG_COMMAND_INITIAL_PRESS;
    }
    if (currently_down && long_in_current_state && press_count == 1) {
        if (led_should_blink) {
            led_blink_overwrite(self->led, 600, 600, 2);
        }
        zb_evt = ZIGBEE_SOMRIG_COMMAND_LONG_PRESS;
    }
    if (!currently_down && long_in_current_state && press_count == 1 && !previous_down_was_long) {
        if (led_should_blink && self->led->blink_times_left == 0) {
            led_blink_overwrite(self->led, 250, 300, 1);
        }
        zb_evt = ZIGBEE_SOMRIG_COMMAND_SHORT_RELEASE;
    }
    if (!currently_down && !long_in_current_state && press_count == 1 && previous_down_was_long) {
        zb_evt = ZIGBEE_SOMRIG_COMMAND_LONG_RELEASE;
    }
    if (!currently_down && !long_in_current_state && press_count == 2 && !previous_down_was_long) {
        if (led_should_blink) {
            led_blink_overwrite(self->led, 300, 300, 3);
        }
        zb_evt = ZIGBEE_SOMRIG_COMMAND_DOUBLE_PRESS;
    }

    if (zb_evt != 0xff) {
        send_event(self, zb_evt, btn->multi_press_cnt);
        self->somrig_last_event = zb_evt;
    }

    if (!currently_down && long_in_current_state) {
        self->somrig_press_cnt = 0;
    }
}

void zigbee_scene_button_cluster_init(zigbee_scene_button_cluster* self) {
    self->button = (button_t*)NULL;
    self->endpoint = 0;
    self->led = (led_t*)NULL;
    self->scene_button_index = 0;
    self->somrig_press_cnt = 0;
    self->somrig_last_event = 0;
    zigbee_scene_button_cluster_settings_init(&self->settings);
    
}

void zigbee_scene_button_cluster_settings_init(zigbee_scene_button_cluster_settings* self) {
    self->debounce_delay = 50;
    self->hold_delay = 600;
    self->led_enabled = 1;
    self->somrig_actions_enabled = 1;
    self->multi_press_delay = 600;
}
