#include "identify_cluster.h"

#include "functions/lightshow.h"
#include "hal/printf_selector.h"
#include "zigbee/consts.h"

zigbee_identify_cluster global_zigbee_identify_cluster;

static hal_zigbee_cmd_result_t identify_cluster_command_trampoline(
    uint8_t endpoint,
    uint16_t cluster_id,
    uint8_t command_id,
    void *cmd_payload
);

void identify_cluster_add_to_endpoint(hal_zigbee_endpoint* endpoint) {
    zigbee_identify_cluster* my_cluster = &global_zigbee_identify_cluster;
    my_cluster->endpoint = endpoint->endpoint;

    hal_zigbee_cluster* hal_cluster = &endpoint->clusters[endpoint->cluster_count++];
    hal_cluster->attribute_count = 0;
    hal_cluster->attributes      = (hal_zigbee_attribute*)NULL;
    hal_cluster->cluster_id      = ZCL_CLUSTER_IDENTIFY;
    hal_cluster->cmd_callback    = identify_cluster_command_trampoline;
    hal_cluster->is_server       = 1;
}

void zigbee_identify_cluster_init(zigbee_identify_cluster* cluster, uint8_t endpoint) {
    cluster->endpoint = endpoint;
}

static void run_identify(uint32_t time_ms)  {
    lightshow_start(time_ms);
}

static hal_zigbee_cmd_result_t identify_cluster_command_trampoline(
    uint8_t endpoint,
    uint16_t cluster_id,
    uint8_t command_id,
    void *cmd_payload
) {
    printf("Identify Cluster Command %d %d %d", (int)endpoint, (int)cluster_id, (int)command_id);
    if (
        endpoint == global_zigbee_identify_cluster.endpoint 
        && cluster_id == ZCL_CLUSTER_IDENTIFY
        && command_id == ZCL_CMD_IDENTIFY_IDENTIFY
    ) {
        uint16_t time_s = cmd_payload ? *(uint16_t*)cmd_payload : 10;
        run_identify(time_s * 1000);
        return HAL_ZIGBEE_CMD_PROCESSED;
    }
    return HAL_ZIGBEE_CMD_SKIPPED;
}

