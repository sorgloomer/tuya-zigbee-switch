#include "identify_cluster.h"
#include "zigbee/consts.h"
#include "../functions/lightshow.h"

#define ATTRID_IDENTIFY_IDENTIFY_TIME               0x0000
#define COMMAND_IDENTIFY_IDENTIFY                   0x00
#define COMMAND_IDENTIFY_IDENTIFY_QUERY             0x01
#define COMMAND_IDENTIFY_TRIGGER_EFFECT             0x40
#define COMMAND_IDENTIFY_IDENTIFY_QUERY_RESPONSE    0x00
#define EFFECT_ID_BLINK                             0x00
#define EFFECT_ID_BREATHE                           0x01
#define EFFECT_ID_OKAY                              0x02
#define EFFECT_ID_CHANNEL_CHANGE                    0x0b
#define EFFECT_ID_FINISH_EFFECT                     0xfe
#define EFFECT_ID_STOP_EFFECT                       0xff
#define EFFECT_VARIANT_DEFAULT                      0x00

zigbee_identify_cluster global_zigbee_identify_cluster;

static void identify_cluster_command_callback(uint8_t endpoint,
                                              uint16_t cluster_id,
                                              uint8_t command_id,
                                              void *cmd_payload);

void identify_cluster_add_to_endpoint(hal_zigbee_endpoint* endpoint) {
    global_zigbee_identify_cluster.endpoint = endpoint->endpoint;
    hal_zigbee_cluster *hal_cluster = &endpoint->clusters[endpoint->cluster_count++];

    hal_cluster->cluster_id      = ZCL_CLUSTER_IDENTIFY;
    hal_cluster->attribute_count = 0;
    hal_cluster->attributes      = (hal_zigbee_attribute*)NULL;
    hal_cluster->is_server       = 1;
    hal_cluster->cmd_callback    = identify_cluster_command_callback;
}

void zigbee_identify_cluster_init(zigbee_identify_cluster* cluster, uint8_t endpoint) {
    cluster->endpoint = endpoint;
}

static void run_identify()  {
    lightshow_start(10000);
}

static void identify_cluster_command_callback(
    uint8_t endpoint,
    uint16_t cluster_id,
    uint8_t command_id,
    void *cmd_payload
) {
    if (
        endpoint == global_zigbee_identify_cluster.endpoint 
        && cluster_id == ZCL_CLUSTER_IDENTIFY
        && command_id == COMMAND_IDENTIFY_IDENTIFY
    ) {
        run_identify();
    }
}

