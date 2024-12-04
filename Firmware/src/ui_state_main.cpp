#include "ui_common.h"

namespace UiStateMain
{
	UiCommon::UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};

	Settings::MeasuringType _measuring_type = Settings::MeasuringType::NONE;
	float _drip_rate = 0.0f;
	bool _update_screen  = false;
	uint32_t _last_draw_ts = 0;
	uint32_t _droplet_indc_draw_ts = 0;
	bool _droplet_indc_on = false;

	void draw_battery_icon(uint8_t x, uint8_t y, uint8_t perc, bool is_charging)
	{
		if(is_charging)
		{
			UiCommon::get_display()->drawBitmap(x, y, epd_bitmap_battery_charging, 12, 11, 1);
		}
		else
		{
			UiCommon::get_display()->drawBitmap(x, y + 2, epd_bitmap_battery_shell, 12, 7, 1);        
			if(perc > 100) perc = 100;
			uint8_t level = perc / 25 + 1;
			if(level > 4) level = 4;
			switch(level)
			{
				case 4:
					UiCommon::get_display()->drawLine(x + 8, y + 4, x + 8, y + 6, 1);
				case 3:
					UiCommon::get_display()->drawLine(x + 6, y + 4, x + 6, y + 6, 1);
				case 2:
					UiCommon::get_display()->drawLine(x + 4, y + 4, x + 4, y + 6, 1);
					break;
				case 1:
					break;        
				default:
					break;
			}
		}
		
		UiCommon::get_display()->setTextSize(1);
		UiCommon::get_display()->setCursor(x + 14, y + 2);
		UiCommon::get_display()->print(perc);
		UiCommon::get_display()->print("%");
	}

	void draw_measuring_selector(Settings::MeasuringType m_t)
	{
		UiCommon::get_display()->setCursor(2, 2);
		UiCommon::get_display()->setTextSize(1);
		UiCommon::get_display()->print("M20 m60 GTT");
		int x = 0;
		if(m_t == Settings::MeasuringType::MACRO) x = 0;
		else if(m_t == Settings::MeasuringType::MICRO) x = 24;
		else if(m_t == Settings::MeasuringType::GTT) x = 48;
		UiCommon::get_display()->drawRect(x, 0, 21, 11, 1);
	}

	void droplet_detected_handler()
	{   
		_droplet_indc_draw_ts = millis();
		_update_screen = true;
		_droplet_indc_on = true;
	}

	void draw()
	{
		UiCommon::get_display()->clearDisplay();

		if(millis() - _droplet_indc_draw_ts <= DROPLEPT_INDICATOR_BLINK_DURATION_MS)
		{
			UiCommon::get_display()->drawBitmap(7, 53, epd_bitmap_droplet, 7, 11, 1);
		}

		draw_measuring_selector(_measuring_type);
		draw_battery_icon(90, 0, (uint8_t) PowerManager::get_batt_perc(), PowerManager::is_charging());
		
		UiCommon::get_display()->setTextSize(4);
		switch(_measuring_type)
		{
			case Settings::MeasuringType::GTT:
				_drip_rate = DropletDetector::get_drops_per_min();
				break;
			case Settings::MeasuringType::MACRO:
				_drip_rate = DropletDetector::get_ml_per_min();
				break;
			case Settings::MeasuringType::MICRO:
				_drip_rate = DropletDetector::get_ml_per_min(true);
				break;
			default:
				break;
		}

		int dr_cursor_x = 30;
		if(_drip_rate >= 100.0f) dr_cursor_x = 6;
		else if(_drip_rate >= 10.0f) dr_cursor_x = 18;

		UiCommon::get_display()->setCursor(dr_cursor_x, 17);

		if(_drip_rate == INFINITY)
		{
			UiCommon::get_display()->print(0.0f, 1);
		}
		else
		{
			UiCommon::get_display()->print(_drip_rate, 1);
		}	

		UiCommon::get_display()->setTextSize(1);	
		if(_measuring_type == Settings::MeasuringType::GTT)
		{
			UiCommon::get_display()->setCursor(39, 50);
			UiCommon::get_display()->print("drops/min");
		}
		else
		{
			UiCommon::get_display()->setCursor(47, 50);
			UiCommon::get_display()->print("ml/min");
		}
		UiCommon::get_display()->display();
	}

	void btn_up_click()
	{
		if(_measuring_type == Settings::MeasuringType::MACRO) _measuring_type = Settings::MeasuringType::MICRO;
		else if(_measuring_type == Settings::MeasuringType::MICRO) _measuring_type = Settings::MeasuringType::GTT;
		else if(_measuring_type == Settings::MeasuringType::GTT) _measuring_type = Settings::MeasuringType::MACRO;
		else return;
		_update_screen = true;
		Settings::set_measuring_type(_measuring_type);
		Settings::save();
	}

	void btn_ok_click()
	{
		change_state(UiStateSettings::get_data());
	}

	void btn_ok_long_press()
	{
		UiCommon::shutdown("USER ACTION");
	}

	void btn_dn_click()
	{
		change_state(UiStateInfo::get_data());
	}

	void on_enter()
	{
		_data.entered_ts = millis();
		UiCommon::attach_btn_ok_click(btn_ok_click);
		UiCommon::attach_btn_up_click(btn_up_click);
		UiCommon::attach_btn_dn_click(btn_dn_click);
		UiCommon::attach_btn_ok_lpress(btn_ok_long_press);
		_measuring_type = Settings::get_measuring_type();
		DropletDetector::enable();
		draw();
	}	

	void tick()
	{
		noInterrupts();
		if(_update_screen)
		{
			_last_draw_ts = 0;
			_update_screen = false;
		}
		interrupts();

		if(millis() - _droplet_indc_draw_ts > DROPLEPT_INDICATOR_BLINK_DURATION_MS && _droplet_indc_on)
		{
			_last_draw_ts = 0;
			_droplet_indc_on = false;
		}

		if(millis() - _last_draw_ts > DISPLAY_REF_PERIOD_MS)
		{
			draw();
			_last_draw_ts = millis();
		}
	}

	void on_exit()
	{
		DropletDetector::disable();
		UiCommon::flush_display();
	}

	UiCommon::UiStateData* get_data()
	{
		return &_data;
	}
}