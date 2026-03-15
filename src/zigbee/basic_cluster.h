#ifndef _BASIC_CLUSTER_H_
#define _BASIC_CLUSTER_H_

#include "hal/zigbee.h"

#include <stddef.h>

#define POWER_SOURCE_UNKNOWN               0x00
#define POWER_SOURCE_MAINS_1_PHASE         0x01
#define POWER_SOURCE_MAINS_3_PHASE         0x02
#define POWER_SOURCE_BATTERY               0x03
#define POWER_SOURCE_DC                    0x04
#define POWER_SOURCE_EMERG_MAINS_CONST_PWR 0x05
#define POWER_SOURCE_EMERG_MAINS_XFER_SW   0x06
#define POWER_SOURCE_PRIMARY               0x7F
#define POWER_SOURCE_SECONDARY             0x80

typedef struct {
    uint8_t              deviceEnable;
    char                 manuName[32];
    char                 modelId[32];
    hal_zigbee_attribute attr_infos[14];
} zigbee_basic_cluster;

void basic_cluster_add_to_endpoint(zigbee_basic_cluster *cluster,
                                   hal_zigbee_endpoint *endpoint);

void basic_cluster_callback_attr_write_trampoline(uint16_t attribute_id);

void basic_cluster_set_power_source(uint8_t power_source_id);

#endif
