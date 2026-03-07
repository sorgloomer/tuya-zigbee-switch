#ifndef _SCENE_BUTTON_CLUSTER_H_
#define _SCENE_BUTTON_CLUSTER_H_

#include "base_components/button.h"
#include "base_components/led.h"
#include "hal/zigbee.h"
#include <stdint.h>

typedef struct {
    uint8_t              endpoint;
    uint16_t             multistate_state;
    hal_zigbee_attribute multistate_attr_infos[4];

    uint8_t              scene_button_index;
    button_t *           button;
    led_t *              led;
} zigbee_scene_button_cluster;


zigbee_scene_button_cluster* scene_button_cluster_new(button_t* button, led_t * led);
void scene_button_cluster_add_to_endpoint(zigbee_scene_button_cluster* cluster, hal_zigbee_endpoint *endpoint);


#endif
