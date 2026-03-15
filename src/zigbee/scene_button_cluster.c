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

static inline void _send_event(
    zigbee_scene_button_cluster* self,
    uint16_t cluster_id,
    uint8_t command_id,
    uint8_t* payload,
    uint8_t payload_size
) {
    hal_zigbee_cmd cmd = {
        .endpoint            = self->endpoint,
        .profile_id          = ZCL_HA_PROFILE,
        .cluster_id          = cluster_id,
        .command_id          = command_id,
        .cluster_specific    = 1,
        .direction           = HAL_ZIGBEE_DIR_CLIENT_TO_SERVER,
        .disable_default_rsp = 1,
        .manufacturer_code   = 0,
        .payload             = payload,
        .payload_len         = payload_size,
    };
    hal_zigbee_send_cmd_to_bindings(&cmd);
}

static void send_event_somrig(
    zigbee_scene_button_cluster* self,
    uint8_t command_id
) {
    uint8_t press_count = self->button->multi_press_cnt;
    uint8_t payload[1] = { press_count };
    _send_event(self, ZCL_CLUSTER_IKEA_TRADFRI_SOMRIG_BUTTON, command_id, payload, 1);
    printf(
        "Sent zigbee somrig command endpoint=%d command=%d payload=%02x\r\n",
        (int)self->endpoint,
        (int)command_id,
        (int)press_count
    );
}

static void send_event_universal(
    zigbee_scene_button_cluster* self,
    uint8_t command_id,
    uint8_t param_flags,
    uint8_t param_press_count
) {
    uint8_t payload[2] = { param_flags, param_press_count };
    _send_event(self, ZCL_CLUSTER_IKEA_TRADFRI_SOMRIG_BUTTON, command_id, payload, 2);
    printf(
        "Sent zigbee somrig command endpoint=%d command=%d payload=%02x%02x\r\n",
        (int)self->endpoint,
        (int)command_id,
        (int)param_flags,
        (int)param_press_count
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

    uint8_t led_should_blink = (self->led != NULL) && self->settings.led_enabled;
    uint8_t pressed = btn->pressed;
    uint8_t timeout = btn->event_level_held;
    uint8_t previous_down_was_long = btn->long_pressed;

    if (timeout == 0 && pressed != 0) {
        if (led_should_blink) {
            led_blink_overwrite(self->led, 300, 300, 1);
        }
        if (self->somrig_pending == 0) {
            send_event_somrig(self, ZIGBEE_SOMRIG_COMMAND_INITIAL_PRESS);
        }
        self->somrig_long = 0;
        self->somrig_pending++;
    }

    if (timeout == 0 && pressed == 0) {
        if (self->somrig_long == 1) {
            send_event_somrig(self, ZIGBEE_SOMRIG_COMMAND_LONG_RELEASE);
            self->somrig_pending = 0;
        }
        if (self->somrig_pending == 2) {
            send_event_somrig(self, ZIGBEE_SOMRIG_COMMAND_DOUBLE_PRESS);
            self->somrig_pending = 0;
        }
    }

    if (timeout != 0 && pressed != 0) {
        if (self->somrig_pending == 2) {
            send_event_somrig(self, ZIGBEE_SOMRIG_COMMAND_SHORT_RELEASE);
            send_event_somrig(self, ZIGBEE_SOMRIG_COMMAND_INITIAL_PRESS);
            self->somrig_pending = 0;
        }
        send_event_somrig(self, ZIGBEE_SOMRIG_COMMAND_LONG_PRESS);
        self->somrig_long = 1;
    }

    if (timeout != 0 && pressed == 0) {
        if (self->somrig_pending == 1 && self->somrig_long == 0) {
            send_event_somrig(self, ZIGBEE_SOMRIG_COMMAND_SHORT_RELEASE);
        }
        self->somrig_pending = 0;
        self->somrig_long = 0;
    }
}

void zigbee_scene_button_cluster_init(zigbee_scene_button_cluster* self) {
    self->button = (button_t*)NULL;
    self->endpoint = 0;
    self->led = (led_t*)NULL;
    self->scene_button_index = 0;
    self->somrig_long = 0;
    self->somrig_pending = 0;
    zigbee_scene_button_cluster_settings_init(&self->settings);
    
}

void zigbee_scene_button_cluster_settings_init(zigbee_scene_button_cluster_settings* self) {
    self->debounce_delay = 50;
    self->hold_delay = 600;
    self->led_enabled = 1;
    self->somrig_actions_enabled = 1;
    self->multi_press_delay = 600;
}
