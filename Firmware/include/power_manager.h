#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "common.h"

namespace PowerManager
{

enum class PowerState
{
	NONE = 0,
	DISCHARGING,
	CHARGING,
	CHARGING_DONE
};

void init();
void handle();
PowerState get_power_state();
float get_batt_volt();
uint8_t get_batt_perc();
void attach_wakeup_cb();
bool is_charging();
void sleep();
void reset_sleep_timer();
bool is_time_to_sleep();
uint32_t get_sec_until_sleep();
float get_chg_volt();

}

#endif