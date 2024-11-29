#include "power_manager.h"
#include <ArduinoLowPower.h>
#include "avdweb_AnalogReadFast.h"
#include <ArduinoLowPower.h>
#include "pinout.h"
#include "settings.h"

namespace PowerManager
{

uint32_t _last_ref_ts_ms = 0;
float _batt_volt = 0.0f;
float _chg_volt = 0.0f;
float _batt_perc = -1.0f;
PowerState _cur_power_state = PowerState::NONE;
bool _woken = false;
bool _was_sleeping = false;
void (*_btn_ok_cb)() = NULL;
uint32_t _sleep_timer_last_ts_ms = 0;

void reset_sleep_timer()
{
    _sleep_timer_last_ts_ms = millis();
}

bool is_time_to_sleep()
{    
    if(get_power_state() != PowerState::DISCHARGING) return false;
    else if(get_batt_volt() < Settings::get_batt_cutoff() && millis() > 20000)
    {
        return true;
    }
    else if(millis() - _sleep_timer_last_ts_ms > Settings::get_shutdown_inactivity_minutes() * 60 * 1000)
    {
        return true;
    }
    return false;
}

uint32_t get_sec_until_sleep()
{
    return (Settings::get_shutdown_inactivity_minutes() * 60) - (millis() - _sleep_timer_last_ts_ms) / 1000;
}

void set_oled_12v_en(bool enable)
{
	digitalWrite(PIN_OLED_PANEL_EN, enable); //high is on
}

void on_wakeup()
{
	if(!_was_sleeping) return;
	_woken = true;
    reset_sleep_timer();
	_was_sleeping = false;
}

void init()
{
	pinMode(PIN_BAT_LVL, INPUT);
	pinMode(PIN_OLED_PANEL_EN, OUTPUT);
	pinMode(PIN_CHG, INPUT);
	set_oled_12v_en(true);
	LowPower.attachInterruptWakeup(PIN_BTN_OK, on_wakeup, FALLING);
}

void sleep()
{
	set_oled_12v_en(false);
	noInterrupts();
	_was_sleeping = true;
	interrupts();
	LowPower.deepSleep();
}

void change_power_state(PowerState state)
{
    if(_cur_power_state == state) return;

    _cur_power_state = state;
    reset_sleep_timer();
}

void calc_power_info()
{
	// disable interrupts in order not to conflict with PT read
	noInterrupts();
	float new_batt_volt = analogReadFast(PIN_BAT_LVL);
	interrupts();
	new_batt_volt *= 2.00755f; // fine tune based on crappy 100K/100K divider on PIN9
	new_batt_volt *= 3.300f;
	new_batt_volt /= 4095;
	// apply influence filter to minimize flickering
	if(_batt_volt > 0.0f) _batt_volt = _batt_volt * (1 - BATT_VOLT_INFLUENCE) + new_batt_volt * BATT_VOLT_INFLUENCE;
	else _batt_volt = new_batt_volt;

	// needs to be a delay between analogReadFast
	delayMicroseconds(20);
	noInterrupts();
	float chg_volt = analogReadFast(PIN_CHG);
	interrupts();
	chg_volt *= 2;
	chg_volt *= 3.3;
	chg_volt /= 4095;
    _chg_volt = chg_volt;

	if(chg_volt > CHG_FLOATING_MAX)
	{
        change_power_state(PowerState::CHARGING_DONE);
	}
	else if(chg_volt > CHG_FLOATING_MIN)change_power_state(PowerState::DISCHARGING); // floating
	else change_power_state(PowerState::CHARGING);

	float new_batt_perc = 123.0f - 123.0f/pow(1.0f + pow(_batt_volt/3.7f, 80.0f), 0.165f);
	if(_batt_perc < 0)
	{
		_batt_perc = new_batt_perc;
	}
	else
	{
		float filtered_batt_perc = _batt_perc * (1 - BATT_PERC_INFLUENCE) + new_batt_perc * BATT_PERC_INFLUENCE;
		if(_cur_power_state == PowerState::CHARGING_DONE)
		{
			_batt_perc = 100.0f;
		}
		else if(_cur_power_state == PowerState::DISCHARGING)
		{
			if(filtered_batt_perc < _batt_perc) _batt_perc = filtered_batt_perc;
		}
		else if(_cur_power_state == PowerState::CHARGING)
		{
			if(filtered_batt_perc > _batt_perc) _batt_perc = filtered_batt_perc;
		}
	}
}

void handle()
{
	if(millis() - _last_ref_ts_ms > DISPLAY_REF_PERIOD_MS)
	{
		calc_power_info();
		noInterrupts();
		if(_woken){
			_woken = false;
			interrupts();
			set_oled_12v_en(true);
		}
		interrupts();
	}
}

PowerState get_power_state()
{
	return _cur_power_state;
}

bool is_charging()
{
	return get_power_state() == PowerState::CHARGING ? true : false;
}

float get_batt_volt()
{
	return _batt_volt;
}

float get_chg_volt()
{
	return _chg_volt;
}

uint8_t get_batt_perc()
{
	return (uint8_t) _batt_perc;
}

}