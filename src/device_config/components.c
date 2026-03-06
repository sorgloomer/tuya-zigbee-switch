#include "components.h"

network_indicator_t network_indicator = {
    .leds                        = { NULL, NULL, NULL, NULL },
    .has_dedicated_led           = 0,
    .manual_state_when_connected = 1,
};

led_t   leds[COMPONENTS_MAX_LED_COUNT];
uint8_t leds_cnt = 0;

button_t buttons[COMPONENTS_MAX_BUTTON_COUNT];
uint8_t  buttons_cnt = 0;

relay_t relays[COMPONENTS_MAX_RELAY_COUNT]; // 4 relay endpoints + 3 cover endpoints
uint8_t relays_cnt = 0;

zigbee_basic_cluster basic_cluster = {
    .deviceEnable = 1,
};

zigbee_group_cluster group_cluster = {};

zigbee_switch_cluster switch_clusters[COMPONENTS_MAX_SWITCH_CLUSTER_COUNT];
uint8_t switch_clusters_cnt = 0;

zigbee_relay_cluster relay_clusters[COMPONENTS_MAX_RELAY_CLUSTER_COUNT];
uint8_t relay_clusters_cnt = 0;

zigbee_cover_switch_cluster cover_switch_clusters[COMPONENTS_MAX_COVER_SWITCH_CLUSTER_COUNT];
uint8_t cover_switch_clusters_cnt = 0;

zigbee_cover_cluster cover_clusters[COMPONENTS_MAX_COVER_CLUSTER_COUNT];
uint8_t cover_clusters_cnt = 0;

zigbee_scene_button_cluster scene_button_clusters[COMPONENTS_MAX_SCENE_BUTTON_CLUSTER_COUNT];
uint8_t scene_button_clusters_cnt = 0;

hal_zigbee_cluster  clusters[COMPONENTS_MAX_ZIGBEE_CLUSTER_COUNT];
hal_zigbee_endpoint endpoints[COMPONENTS_MAX_ZIGBEE_ENDPOINT_COUNT];

uint8_t allow_simultaneous_latching_pulses = 0;
