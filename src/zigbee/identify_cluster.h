#ifndef _IDENTIFY_CLUSTER_H_
#define _IDENTIFY_CLUSTER_H_

#include <stdint.h>
#include "hal/zigbee.h"


typedef struct {
    uint8_t     endpoint;
} zigbee_identify_cluster;

extern zigbee_identify_cluster global_zigbee_identify_cluster;

void identify_cluster_add_to_endpoint(hal_zigbee_endpoint* endpoint);

#endif