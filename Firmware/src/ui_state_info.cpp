#include "ui_common.h"

namespace UiStateInfo
{
	UiCommon::UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};	
	uint32_t _last_draw_ts = 0;

	void draw()
	{
		UiCommon::get_display()->clearDisplay();
		UiCommon::get_display()->setTextSize(1);
		UiCommon::get_display()->setCursor(0, 0);
		UiCommon::get_display()->print("Info");
		UiCommon::get_display()->drawLine(0, 9, 80, 9, 1);
		// display.print("PowerState:");
		// display.print((int)power_state);
		UiCommon::get_display()->setCursor(0, 12);
		UiCommon::get_display()->print("BAT_V:");
		UiCommon::get_display()->print(PowerManager::get_batt_volt(), 3);

        UiCommon::get_display()->setCursor(0, 22);
		UiCommon::get_display()->print("CHG_V:");
		UiCommon::get_display()->print(PowerManager::get_chg_volt(), 3);

		UiCommon::get_display()->setCursor(0, 32);
		UiCommon::get_display()->print("CHG_S:");
		switch(PowerManager::get_power_state())
		{
			case PowerManager::PowerState::CHARGING:
				UiCommon::get_display()->print("CHG");
				break;
			case PowerManager::PowerState::CHARGING_DONE:
				UiCommon::get_display()->print("DONE");
				break;
			case PowerManager::PowerState::DISCHARGING:
				UiCommon::get_display()->print("DIS");
				break;
			default:
				break;
		}
		UiCommon::get_display()->setCursor(0, 42);
		UiCommon::get_display()->print("FW_VER:");
		UiCommon::get_display()->print(ORBYTRACK_APP_VERSION);

        UiCommon::get_display()->drawBitmap(93, 0, epd_bitmap_logo, 23, 23, 1);


		UiCommon::get_display()->display();
	}

	void btn_ok_long_press()
	{
		change_state(UiStateSleep::get_data());
	}

	void btn_click()
	{
		change_state(UiStateMain::get_data());
	}

	void on_enter()
	{
		_data.entered_ts = millis();
        UiCommon::attach_btn_dn_click(btn_click);
        UiCommon::attach_btn_ok_click(btn_click);
        UiCommon::attach_btn_up_click(btn_click);
        UiCommon::attach_btn_ok_lpress(btn_ok_long_press);
		draw();
	}

	void tick()
	{
		if(millis() - _last_draw_ts > DISPLAY_REF_PERIOD_MS)
		{
			draw();
			_last_draw_ts = millis();
		}
	}

	void on_exit()
	{
		UiCommon::flush_display();
	}

	UiCommon::UiStateData* get_data()
	{
		return &_data;
	}
}