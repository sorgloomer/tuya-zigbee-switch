#ifndef _DEVICE_INIT_H_
#define _DEVICE_INIT_H_

#include "base_components/network_indicator.h"
#include "hal/zigbee.h"

#include "config_nv.h"

void parse_config();
void init_reporting();
void handle_version_changes();

#endif
