#ifndef _COMPONENTS_H_
#define _COMPONENTS_H_

#include "base_components/button.h"
#include "base_components/led.h"
#include "base_components/relay.h"
#include "base_components/network_indicator.h"
#include "hal/zigbee.h"
#include "zigbee/cover_cluster.h"
#include "zigbee/cover_switch_cluster.h"
#include "zigbee/relay_cluster.h"
#include "zigbee/switch_cluster.h"
#include "zigbee/basic_cluster.h"
#include "zigbee/group_cluster.h"
#include "zigbee/scene_button_cluster.h"


#define COMPONENTS_MAX_LED_COUNT 5
#define COMPONENTS_MAX_BUTTON_COUNT 11
#define COMPONENTS_MAX_RELAY_COUNT 10

#define COMPONENTS_MAX_ZIGBEE_ENDPOINT_COUNT 10
#define COMPONENTS_MAX_ZIGBEE_CLUSTER_COUNT 32

#define COMPONENTS_MAX_SWITCH_CLUSTER_COUNT 4
#define COMPONENTS_MAX_RELAY_CLUSTER_COUNT 4
#define COMPONENTS_MAX_COVER_CLUSTER_COUNT 3
#define COMPONENTS_MAX_COVER_SWITCH_CLUSTER_COUNT 3
#define COMPONENTS_MAX_SCENE_BUTTON_CLUSTER_COUNT 7

extern network_indicator_t network_indicator;

extern uint8_t allow_simultaneous_latching_pulses;


extern led_t   leds[COMPONENTS_MAX_LED_COUNT];
extern uint8_t leds_cnt;

extern button_t buttons[COMPONENTS_MAX_BUTTON_COUNT];
extern uint8_t  buttons_cnt;

extern relay_t relays[COMPONENTS_MAX_RELAY_COUNT]; // 4 relay endpoints + 3 cover endpoints
extern uint8_t relays_cnt;

extern zigbee_basic_cluster basic_cluster;

extern zigbee_group_cluster group_cluster;

extern zigbee_switch_cluster switch_clusters[COMPONENTS_MAX_SWITCH_CLUSTER_COUNT];
extern uint8_t switch_clusters_cnt;

extern zigbee_relay_cluster relay_clusters[COMPONENTS_MAX_RELAY_CLUSTER_COUNT];
extern uint8_t relay_clusters_cnt;

extern zigbee_cover_switch_cluster cover_switch_clusters[COMPONENTS_MAX_COVER_SWITCH_CLUSTER_COUNT];
extern uint8_t cover_switch_clusters_cnt;

extern zigbee_cover_cluster cover_clusters[COMPONENTS_MAX_COVER_CLUSTER_COUNT];
extern uint8_t cover_clusters_cnt;

extern hal_zigbee_cluster  clusters[COMPONENTS_MAX_ZIGBEE_CLUSTER_COUNT];
extern int clusters_cnt;
extern hal_zigbee_endpoint endpoints[COMPONENTS_MAX_ZIGBEE_ENDPOINT_COUNT];
extern int endpoints_cnt;
extern int endpoints_alloc_last_again;


extern zigbee_scene_button_cluster scene_button_clusters[COMPONENTS_MAX_SCENE_BUTTON_CLUSTER_COUNT];
extern uint8_t scene_button_clusters_cnt;


#endif