#ifndef _SCENE_BUTTON_CLUSTER_H_
#define _SCENE_BUTTON_CLUSTER_H_

#include "base_components/button.h"
#include "base_components/led.h"
#include "hal/zigbee.h"
#include <stdint.h>


typedef struct {
    uint8_t     led_enabled;
    uint8_t     somrig_actions_enabled;
    uint16_t    debounce_delay;
    uint16_t    hold_delay;
    uint16_t    multi_press_delay;
} zigbee_scene_button_cluster_settings;

typedef struct {
    uint8_t     endpoint;
    uint8_t     scene_button_index;
    uint8_t     somrig_pending;
    uint8_t     somrig_long;
    button_t *  button;
    led_t *     led;

    zigbee_scene_button_cluster_settings settings;
} zigbee_scene_button_cluster;

typedef enum {
    ZIGBEE_SOMRIG_COMMAND_INITIAL_PRESS = 0x01,
    ZIGBEE_SOMRIG_COMMAND_LONG_PRESS    = 0x02,
    ZIGBEE_SOMRIG_COMMAND_SHORT_RELEASE = 0x03,
    ZIGBEE_SOMRIG_COMMAND_LONG_RELEASE  = 0x04,
    ZIGBEE_SOMRIG_COMMAND_DOUBLE_PRESS  = 0x06,
} zigbee_ikea_somrig_command_t;

typedef enum {
    ZIGBEE_SCENE_BUTTON_COMMAND_EDGE_PRESSED = 0x01,
    ZIGBEE_SCENE_BUTTON_COMMAND_EDGE_RELEASED = 0x02,
    ZIGBEE_SCENE_BUTTON_COMMAND_HOLD_PRESSED = 0x03,
    ZIGBEE_SCENE_BUTTON_COMMAND_HOLD_RELEASED = 0x04,
} zigbee_scene_button_cluster_command_t;


zigbee_scene_button_cluster* scene_button_cluster_new(button_t* button, led_t * led);
void scene_button_cluster_add_to_endpoint(zigbee_scene_button_cluster* self, hal_zigbee_endpoint *endpoint);

void zigbee_scene_button_cluster_init(zigbee_scene_button_cluster* self);
void zigbee_scene_button_cluster_settings_init(zigbee_scene_button_cluster_settings* self);


#endif
