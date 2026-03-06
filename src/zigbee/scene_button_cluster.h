#ifndef _SCENE_BUTTON_CLUSTER_H_
#define _SCENE_BUTTON_CLUSTER_H_

#include "base_components/button.h"
#include "base_components/led.h"
#include "hal/zigbee.h"
#include <stdint.h>

typedef struct {
    uint8_t     scene_button_index;
    button_t *  button;
    led_t *     led;
} zigbee_scene_button_cluster;


zigbee_scene_button_cluster* scene_button_cluster_new(button_t* button, led_t * led);


#endif
