#pragma once
#include "data.h"

typedef void (*wifi_data_cb_t)(const UsageData* data);

void wifi_init(wifi_data_cb_t on_data);
void wifi_tick();
