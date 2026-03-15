#include "hal/gpio.h"
#include "hal/printf_selector.h"
#include "hal/zigbee.h"
#include "zigbee/basic_cluster.h"
#include "zigbee/consts.h"
#include "zigbee/group_cluster.h"
#include "zigbee/identify_cluster.h"
#include "zigbee/relay_cluster.h"
#include "zigbee/switch_cluster.h"
#include "zigbee/cover_switch_cluster.h"
#include "zigbee/cover_cluster.h"
#include "zigbee/scene_button_cluster.h"

#include <stdint.h>
#include <string.h>

#include "base_components/led.h"
#include "base_components/network_indicator.h"
#include "config_nv.h"
#include "device_config/reset.h"
#include "device_config/components.h"
#include "hal/system.h"
#include "hal/zigbee.h"
#include "hal/zigbee_ota.h"

#define MULTI_PRESS_CNT_TO_RESET    10

#define SETTING_B_BUTTON 'B'
#define SETTING_C_COVER 'C'
#define SETTING_I_INDICATOR 'I'
#define SETTING_L_LED 'L'
#define SETTING_M_SWITCH_MOMENTARY 'M'
#define SETTING_R_RELAY 'R'
#define SETTING_S_SWITCH 'S'
#define SETTING_T_SCENE_BUTTON 'T'
#define SETTING_X_SWITCH_COVER 'X'
#define SETTING_IMAGE 'i'

// Forward declarations
void periferals_init(void);

// extern ota_preamble_t baseEndpoint_otaInfo;

uint32_t parse_int(const char *s);
static zigbee_scene_button_cluster* scene_button_parse_new(char* entry);
static led_t* led_parse_new(char* entry);
static button_t* button_parse_new(char* entry);

static hal_zigbee_endpoint* endpoint_new_or_reuse();
static void endpoint_new_finalize(hal_zigbee_endpoint* endpoint);
static void endpoint_new_finalize_and_reuse(hal_zigbee_endpoint* endpoint);

char *seek_until(char *cursor, char needle);
char *extract_next_entry(char **cursor);

void on_reset_clicked(void *_) {
    hal_factory_reset();
}

void on_multi_press_reset(void *_, uint8_t press_count) {
    if (press_count >= MULTI_PRESS_CNT_TO_RESET) {
        hal_factory_reset();
    }
}

void parse_config() {
    device_config_read_from_nv();
    char *cursor = (char *)device_config_str.data;

    const char *zb_manufacturer = extract_next_entry(&cursor);

    basic_cluster.manuName[0] = strlen(zb_manufacturer);
    if (basic_cluster.manuName[0] > 31) {
        printf("Manufacturer too big\r\n");
        reset_all();
    }
    memcpy(basic_cluster.manuName + 1, zb_manufacturer,
           basic_cluster.manuName[0]);

    const char *zb_model = extract_next_entry(&cursor);
    basic_cluster.modelId[0] = strlen(zb_model);
    if (basic_cluster.modelId[0] > 31) {
        printf("Model too big\r\n");
        reset_all();
    }
    memcpy(basic_cluster.modelId + 1, zb_model, basic_cluster.modelId[0]);

    bool  has_dedicated_status_led = false;
    char *entry;
    for (entry = extract_next_entry(&cursor); *entry != '\0';
         entry = extract_next_entry(&cursor)) {
        if (entry[0] == SETTING_S_SWITCH && entry[1] == 'L' && entry[2] == 'P') {
            // Simultaneous Latching Pulses == SLP
            allow_simultaneous_latching_pulses = 1;
        } else if (entry[0] == SETTING_B_BUTTON) {
            hal_gpio_pin_t  pin  = hal_gpio_parse_pin(entry + 1);
            hal_gpio_pull_t pull = hal_gpio_parse_pull(entry + 3);
            hal_gpio_init(pin, 1, pull);

            buttons[buttons_cnt].pin = pin;
            buttons[buttons_cnt].long_press_duration_ms  = 2000;
            buttons[buttons_cnt].multi_press_duration_ms = 800;
            buttons[buttons_cnt].on_long_press           = on_reset_clicked;
            buttons_cnt++;
        } else if (entry[0] == SETTING_L_LED) {
            led_t* led = led_parse_new(entry + 1);

            network_indicator.leds[0]           = led;
            network_indicator.leds[1]           = NULL;
            network_indicator.has_dedicated_led = true;
            
            has_dedicated_status_led = true;
        } else if (entry[0] == SETTING_I_INDICATOR) {
            hal_gpio_pin_t pin = hal_gpio_parse_pin(entry + 1);
            hal_gpio_init(pin, 0, HAL_GPIO_PULL_NONE);
            leds[leds_cnt].pin     = pin;
            leds[leds_cnt].on_high = entry[3] != 'i';
            led_init(&leds[leds_cnt]);

            for (int index = 0; index < 4; index++) {
                if (relay_clusters[index].indicator_led == NULL) {
                    relay_clusters[index].indicator_led = &leds[leds_cnt];
                    break;
                }
            }

            if (!has_dedicated_status_led) {
                for (int index = 0; index < 4; index++) {
                    if (network_indicator.leds[index] == NULL) {
                        network_indicator.leds[index] = &leds[leds_cnt];
                        break;
                    }
                }
            }
            leds_cnt++;
        } else if (entry[0] == SETTING_S_SWITCH) {
            hal_gpio_pin_t  pin  = hal_gpio_parse_pin(entry + 1);
            hal_gpio_pull_t pull = hal_gpio_parse_pull(entry + 3);
            hal_gpio_init(pin, 1, pull);

            buttons[buttons_cnt].pin = pin;
            buttons[buttons_cnt].long_press_duration_ms  = 800;
            buttons[buttons_cnt].multi_press_duration_ms = 800;
            buttons[buttons_cnt].on_multi_press          = on_multi_press_reset;

            switch_clusters[switch_clusters_cnt].switch_idx = switch_clusters_cnt;
            switch_clusters[switch_clusters_cnt].mode       =
                ZCL_ONOFF_CONFIGURATION_SWITCH_TYPE_TOGGLE;
            switch_clusters[switch_clusters_cnt].action =
                ZCL_ONOFF_CONFIGURATION_SWITCH_ACTION_TOGGLE_SIMPLE;
            switch_clusters[switch_clusters_cnt].relay_mode =
                ZCL_ONOFF_CONFIGURATION_RELAY_MODE_SHORT;
            switch_clusters[switch_clusters_cnt].binded_mode =
                ZCL_ONOFF_CONFIGURATION_BINDED_MODE_SHORT;
            switch_clusters[switch_clusters_cnt].relay_index =
                switch_clusters_cnt + 1;
            switch_clusters[switch_clusters_cnt].button          = &buttons[buttons_cnt];
            switch_clusters[switch_clusters_cnt].level_move_rate = 50;
            buttons_cnt++;
            switch_clusters_cnt++;
        } else if (entry[0] == SETTING_T_SCENE_BUTTON) {
            scene_button_parse_new(entry);
        } else if (entry[0] == SETTING_R_RELAY) {
            hal_gpio_pin_t pin = hal_gpio_parse_pin(entry + 1);
            hal_gpio_init(pin, 0, HAL_GPIO_PULL_NONE);

            relays[relays_cnt].pin     = pin;
            relays[relays_cnt].on_high = 1;

            if (entry[3] != '\0') {
                pin = hal_gpio_parse_pin(entry + 3);
                hal_gpio_init(pin, 0, HAL_GPIO_PULL_NONE);
                relays[relays_cnt].off_pin     = pin;
                relays[relays_cnt].is_latching = 1;
            }

            relay_clusters[relay_clusters_cnt].relay_idx = relay_clusters_cnt;
            relay_clusters[relay_clusters_cnt].relay     = &relays[relays_cnt];

            relays_cnt++;
            relay_clusters_cnt++;
        } else if (entry[0] == SETTING_X_SWITCH_COVER) {
            hal_gpio_pin_t  open_pin  = hal_gpio_parse_pin(entry + 1);
            hal_gpio_pin_t  close_pin = hal_gpio_parse_pin(entry + 3);
            hal_gpio_pull_t pull      = hal_gpio_parse_pull(entry + 5);

            hal_gpio_init(open_pin, 1, pull);
            hal_gpio_init(close_pin, 1, pull);

            buttons[buttons_cnt].pin = open_pin;
            buttons[buttons_cnt].long_press_duration_ms  = 800;
            buttons[buttons_cnt].multi_press_duration_ms = 800;
            buttons[buttons_cnt].on_multi_press          = on_multi_press_reset;
            button_t *open_button = &buttons[buttons_cnt++];

            buttons[buttons_cnt].pin = close_pin;
            buttons[buttons_cnt].long_press_duration_ms  = 800;
            buttons[buttons_cnt].multi_press_duration_ms = 800;
            buttons[buttons_cnt].on_multi_press          = on_multi_press_reset;
            button_t *close_button = &buttons[buttons_cnt++];

            cover_switch_clusters[cover_switch_clusters_cnt].open_button      = open_button;
            cover_switch_clusters[cover_switch_clusters_cnt].close_button     = close_button;
            cover_switch_clusters[cover_switch_clusters_cnt].cover_switch_idx =
                cover_switch_clusters_cnt;
            cover_switch_clusters_cnt++;
        } else if (entry[0] == SETTING_C_COVER) {
            hal_gpio_pin_t open_pin  = hal_gpio_parse_pin(entry + 1);
            hal_gpio_pin_t close_pin = hal_gpio_parse_pin(entry + 3);

            hal_gpio_init(open_pin, 0, HAL_GPIO_PULL_NONE);
            hal_gpio_init(close_pin, 0, HAL_GPIO_PULL_NONE);

            relays[relays_cnt].pin         = open_pin;
            relays[relays_cnt].on_high     = 1;
            relays[relays_cnt].is_latching = 0;
            relay_t *open_relay = &relays[relays_cnt++];

            relays[relays_cnt].pin         = close_pin;
            relays[relays_cnt].on_high     = 1;
            relays[relays_cnt].is_latching = 0;
            relay_t *close_relay = &relays[relays_cnt++];

            cover_clusters[cover_clusters_cnt].open_relay  = open_relay;
            cover_clusters[cover_clusters_cnt].close_relay = close_relay;
            cover_clusters[cover_clusters_cnt].cover_idx   = cover_clusters_cnt;
            cover_clusters_cnt++;
        } else if (entry[0] == SETTING_IMAGE) {
            uint32_t image_type = parse_int(entry + 1);
            hal_zigbee_set_image_type(image_type);
        } else if (entry[0] == SETTING_M_SWITCH_MOMENTARY) {
            for (int index = 0; index < switch_clusters_cnt; index++) {
                switch_clusters[index].mode =
                    ZCL_ONOFF_CONFIGURATION_SWITCH_TYPE_MOMENTARY;
            }
        }
    }

    periferals_init();

    printf(
        "Initializing Zigbee with %d switches, %d relays, %d cover switches, %d covers, %d scene buttons\r\n",
        (int)switch_clusters_cnt,
        (int)relay_clusters_cnt,
        (int)cover_switch_clusters_cnt,
        (int)cover_clusters_cnt,
        (int)scene_button_clusters_cnt
    );


    {
        hal_zigbee_endpoint* endpoint = endpoint_new_or_reuse();
        basic_cluster_add_to_endpoint(&basic_cluster, endpoint);
        identify_cluster_add_to_endpoint(endpoint);
        hal_ota_cluster_setup(&endpoint->clusters[endpoints->cluster_count++]);
        endpoint_new_finalize_and_reuse(endpoint);
    }

    for (int index = 0; index < switch_clusters_cnt; index++) {
        hal_zigbee_endpoint* endpoint = endpoint_new_or_reuse();
        switch_cluster_add_to_endpoint(&switch_clusters[index], endpoint);
        endpoint_new_finalize(endpoint);
    }
    for (int index = 0; index < relay_clusters_cnt; index++) {
        hal_zigbee_endpoint* endpoint = endpoint_new_or_reuse();
        relay_cluster_add_to_endpoint(&relay_clusters[index], endpoint);
        // Group cluster is stateless, safe to add to multiple endpoints
        group_cluster_add_to_endpoint(&group_cluster, endpoint);
        endpoint_new_finalize(endpoint);
    }

    for (int index = 0; index < cover_switch_clusters_cnt; index++) {
        hal_zigbee_endpoint* endpoint = endpoint_new_or_reuse();
        cover_switch_cluster_add_to_endpoint(&cover_switch_clusters[index], endpoint);
        endpoint_new_finalize(endpoint);
    }

    for (int index = 0; index < cover_clusters_cnt; index++) {
        hal_zigbee_endpoint* endpoint = endpoint_new_or_reuse();
        cover_cluster_add_to_endpoint(&cover_clusters[index], endpoint);
        endpoint_new_finalize(endpoint);
    }

    for (int index = 0; index < scene_button_clusters_cnt; index++) {
        hal_zigbee_endpoint* endpoint = endpoint_new_or_reuse();
        scene_button_cluster_add_to_endpoint(&scene_button_clusters[index], endpoint);
        endpoint_new_finalize(endpoint);
    }

    hal_zigbee_init(endpoints, endpoints_cnt);
    while (cursor != (char *)device_config_str.data) {
        cursor--;
        if (*cursor == '\0') {
            *cursor = ';';
        }
    }

    printf("Config parsed successfully\r\n");
}

static hal_zigbee_endpoint* endpoint_new_or_reuse() {
    if (endpoints_alloc_last_again) {
        endpoints_cnt--;
        clusters_cnt -= endpoints[endpoints_cnt].cluster_count;
        endpoints_alloc_last_again = 0;
    }
    if (endpoints_cnt >= COMPONENTS_MAX_ZIGBEE_ENDPOINT_COUNT) {
        return (hal_zigbee_endpoint*)NULL;
    }

    int endpoint_index = endpoints_cnt + 1;
    hal_zigbee_endpoint* self = &endpoints[endpoints_cnt++];
    self->endpoint   = endpoint_index;
    self->clusters   = clusters + clusters_cnt;
    self->profile_id = ZCL_HA_PROFILE;
    self->device_id  = 0xffff;
    return self;
}

static void endpoint_new_finalize(hal_zigbee_endpoint* endpoint) {
    clusters_cnt += endpoint->cluster_count;
}

static void endpoint_new_finalize_and_reuse(hal_zigbee_endpoint* endpoint) {
    endpoint_new_finalize(endpoint);
    endpoints_alloc_last_again = 1;
}


/** parses 3 chars */
static button_t* button_parse_new(char* entry) {
    if (buttons_cnt >= COMPONENTS_MAX_BUTTON_COUNT) {
        return (button_t*)NULL;
    }
    hal_gpio_pin_t  btn_pin  = hal_gpio_parse_pin(entry + 0);
    hal_gpio_pull_t btn_pull = hal_gpio_parse_pull(entry + 2);

    button_t* self = &buttons[buttons_cnt++];
    btn_init_before(self);
    self->pin = btn_pin;
    hal_gpio_init(btn_pin, 1, btn_pull);
    return self;
}

/** parses 3 chars */
static led_t* led_parse_new(char* entry) {
    if (leds_cnt >= COMPONENTS_MAX_LED_COUNT) {
        return (led_t*)NULL;
    }
    hal_gpio_pin_t led_pin  = hal_gpio_parse_pin(entry);
    char led_mode  = entry[2];

    led_t* self = &leds[leds_cnt++];
    self->pin = led_pin;
    self->on_high = led_mode != 'i';
    hal_gpio_init(led_pin, 0, HAL_GPIO_PULL_NONE);
    led_init(self);

    return self;
}

static zigbee_scene_button_cluster* scene_button_parse_new(char* entry) {
    int len = strlen(entry);
    if (len < 4) {
        return NULL;
    }
    button_t* button = button_parse_new(entry + 1);
    if (button == NULL) {
        return (zigbee_scene_button_cluster*)NULL;
    }

    led_t* led = NULL;
    if (len >= 7 && entry[4] != '-') {
        led = led_parse_new(entry + 4);
    }

    return scene_button_cluster_new(button, led);
}

void network_indicator_on_network_status_change(
    hal_zigbee_network_status_t new_status) {
    printf("Network status changed to %d\r\n", new_status);
    if (new_status == HAL_ZIGBEE_NETWORK_JOINED) {
        network_indicator_connected(&network_indicator);
        update_relay_clusters();
    } else {
        network_indicator_not_connected(&network_indicator);
    }
}

void periferals_init() {
    for (int index = 0; index < buttons_cnt; index++) {
        btn_init_after(&buttons[index]);
    }
    for (int index = 0; index < leds_cnt; index++) {
        led_init(&leds[index]);
    }
    for (int index = 0; index < relays_cnt; index++) {
        relay_init(&relays[index]);
    }
    if (hal_zigbee_get_network_status() == HAL_ZIGBEE_NETWORK_JOINED) {
        network_indicator_connected(&network_indicator);
    } else {
        network_indicator_not_connected(&network_indicator);
    }
    hal_register_on_network_status_change_callback(
        network_indicator_on_network_status_change);
}

// Helper functions

char *seek_until(char *cursor, char needle) {
    while (*cursor != needle && *cursor != '\0') {
        cursor++;
    }
    return(cursor);
}

char *extract_next_entry(char **cursor) {
    char *end = seek_until(*cursor, ';');

    *end = '\0';
    char *res = *cursor;
    *cursor = end + 1;
    return(res);
}

uint32_t parse_int(const char *s) {
    if (!s)
        return 0;

    uint32_t n = 0;
    while (*s >= '0' && *s <= '9') {
        n = n * 10 + (uint32_t)(*s - '0');
        s++;
    }
    return n;
}
