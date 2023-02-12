#pragma once
#include "adc.h"

void battery_monitor_update(void);
uint32_t battery_monitor_raw_value(void);
uint32_t battery_monitor_mv_value(void);