#include "ui.h"
#include "OneButton.h"
#include "pinout.h"

#define SH110X_NO_SPLASH

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "ui_bitmaps.h"
#include "power_manager.h"
#include "droplet_detector.h"
#include "settings.h"

namespace Ui
{

OneButton _btn_up(PIN_BTN_UP, true);
OneButton _btn_ok(PIN_BTN_OK, true, false);
OneButton _btn_dn(PIN_BTN_DN, true);
UiStateData* _cur_state = NULL;
UiStateData* _next_state = NULL;
Adafruit_SH1107 _display = Adafruit_SH1107(SCREEN_HEIGHT, SCREEN_WIDTH, &Wire);

void (*btn_up_click_state_cb)() = NULL;
void (*btn_dn_click_state_cb)() = NULL;
void (*btn_ok_click_state_cb)() = NULL;
void (*btn_ok_lpress_state_cb)() = NULL;

struct BlinkItem
{
    uint16_t half_period_ms = 0;
    uint32_t last_change_ts = 0;
    void (*process_on_period)(const char* txt);
    void (*process_off_period)(const char* txt);
    bool is_off_period = false;
};

void btn_up_click(void* btn)
{
    PowerManager::reset_sleep_timer();
    if(btn_up_click_state_cb != NULL) btn_up_click_state_cb();
}

void btn_dn_click(void* btn)
{
    PowerManager::reset_sleep_timer();
    if(btn_dn_click_state_cb != NULL) btn_dn_click_state_cb();
}

void btn_ok_click(void* btn)
{
    PowerManager::reset_sleep_timer();
    if(btn_ok_click_state_cb != NULL) btn_ok_click_state_cb();
}

void btn_ok_lpress(void* btn)
{
    PowerManager::reset_sleep_timer();
    if(btn_ok_lpress_state_cb != NULL) btn_ok_lpress_state_cb();
}

void process_blink_item(BlinkItem* item, bool blink, const char* txt, bool* update_scr)
{
    if(!blink)
    {
        item->process_on_period(txt);
        return;
    }

    if(millis() - item->last_change_ts > item->half_period_ms)
    {
        item->is_off_period = !item->is_off_period;
        item->last_change_ts = millis();
    }

    if(item->is_off_period)
    {
        item->process_off_period(txt);
    }
    else
    {
        item->process_on_period(txt);
    }
    
}

void show_logo()
{
	_display.clearDisplay();
	_display.drawBitmap(0, 15, epd_bitmap_logo, 128, 28, 1);
	_display.setTextSize(1);
	_display.setCursor(14, 50);
	_display.print(ORBYTRACK_APP_VERSION);
	_display.print(" by LiliumJSN");
	_display.display();
}

void detach_all_btn()
{
    btn_up_click_state_cb = NULL;
    btn_dn_click_state_cb = NULL;
    btn_ok_click_state_cb = NULL;
    btn_ok_lpress_state_cb = NULL;
}

void flush_display()
{
	_display.clearDisplay();
	_display.display();
}

namespace UiStateBoot
{
	UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};

	void on_enter()
	{
		_data.entered_ts = millis();
		detach_all_btn();
		show_logo();
	}

	void tick()
	{
		if(millis() - _data.entered_ts > BOOT_LOGO_DURATION_MS)
		{
			change_state(UiStateMain::get_data());
		}
	}

	void on_exit()
	{
		flush_display();
	}

	UiStateData* get_data()
	{
		return &_data;
	}
}

namespace UiStateWokeup
{
	UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};
	
	void on_enter()
	{
		_data.entered_ts = millis();
        _display.setContrast(Settings::get_contrast_level());
		detach_all_btn();
		show_logo();
	}	

	void tick()
	{
		if(millis() - _data.entered_ts > WAKEUP_LOGO_DURATION_MS)
		{
			change_state(UiStateMain::get_data());
		}
	}

	void on_exit()
	{
		flush_display();
	}

	UiStateData* get_data()
	{
		return &_data;
	}
}

namespace UiStateMain
{
	UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};

	Settings::MeasuringType _measuring_type = Settings::MeasuringType::NONE;
	float _drip_rate = 0.0f;
	bool _update_screen  = false;
	uint32_t _last_draw_ts = 0;

	void draw_battery_icon(uint8_t x, uint8_t y, uint8_t perc, bool is_charging)
	{
		if(is_charging)
		{
			_display.drawBitmap(x, y, epd_bitmap_battery_charging, 12, 11, 1);
		}
		else
		{
			_display.drawBitmap(x, y + 2, epd_bitmap_battery_shell, 12, 7, 1);        
			if(perc > 100) perc = 100;
			uint8_t level = perc / 25 + 1;
			if(level > 4) level = 4;
			switch(level)
			{
				case 4:
					_display.drawLine(x + 8, y + 4, x + 8, y + 6, 1);
				case 3:
					_display.drawLine(x + 6, y + 4, x + 6, y + 6, 1);
				case 2:
					_display.drawLine(x + 4, y + 4, x + 4, y + 6, 1);
					break;
				case 1:
					break;        
				default:
					break;
			}
		}
		
		_display.setTextSize(1);
		_display.setCursor(x + 14, y + 2);
		_display.print(perc);
		_display.print("%");
	}

	void draw_measuring_selector(Settings::MeasuringType m_t)
	{
		_display.setCursor(2, 2);
		_display.setTextSize(1);
		_display.print("M20 m60 GTT");
		int x = 0;
		if(m_t == Settings::MeasuringType::MACRO) x = 0;
		else if(m_t == Settings::MeasuringType::MICRO) x = 24;
		else if(m_t == Settings::MeasuringType::GTT) x = 48;
		_display.drawRect(x, 0, 21, 11, 1);
	}

	void draw()
	{
		_display.clearDisplay();

		draw_measuring_selector(_measuring_type);
		draw_battery_icon(90, 0, (uint8_t) PowerManager::get_batt_perc(), PowerManager::is_charging());
		
		_display.setCursor(0,17);
		_display.setTextSize(4);
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

		if(_drip_rate == INFINITY)
		{
			_display.print(0.0f, 1);
		}
		else
		{
			_display.print(_drip_rate, 1);
		}	

		_display.setTextSize(1);	
		if(_measuring_type == Settings::MeasuringType::GTT)
		{
			_display.setCursor(39, 50);
			_display.print("drops/min");
		}
		else
		{
			_display.setCursor(47, 50);
			_display.print("ml/min");
		}
		_display.display();
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
		change_state(UiStateSleep::get_data());
	}

	void btn_dn_click()
	{
		change_state(UiStateInfo::get_data());
	}

	void on_enter()
	{
		_data.entered_ts = millis();
        btn_up_click_state_cb = btn_up_click;
        btn_dn_click_state_cb = btn_dn_click;
        btn_ok_click_state_cb = btn_ok_click;
        btn_ok_lpress_state_cb = btn_ok_long_press;
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

		if(millis() - _last_draw_ts > DISPLAY_REF_PERIOD_MS)
		{
			draw();
			_last_draw_ts = millis();
		}
	}

	void on_exit()
	{
        DropletDetector::disable();
		flush_display();
	}

	UiStateData* get_data()
	{
		return &_data;
	}
}

namespace UiStateInfo
{
	UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};	
	uint32_t _last_draw_ts = 0;

	void draw()
	{
		_display.clearDisplay();
		_display.setTextSize(1);
		_display.setCursor(0, 0);
		_display.print("Info");
		_display.drawLine(0, 9, 80, 9, 1);
		// display.print("PowerState:");
		// display.print((int)power_state);
		_display.setCursor(0, 12);
		_display.print("BAT_V:");
		_display.print(PowerManager::get_batt_volt(), 3);

        _display.setCursor(0, 22);
		_display.print("CHG_V:");
		_display.print(PowerManager::get_chg_volt(), 3);

		_display.setCursor(0, 32);
		_display.print("POW_STATE:");
		switch(PowerManager::get_power_state())
		{
			case PowerManager::PowerState::CHARGING:
				_display.print("CHG");
				break;
			case PowerManager::PowerState::CHARGING_DONE:
				_display.print("CHG_DONE");
				break;
			case PowerManager::PowerState::DISCHARGING:
				_display.print("DIS_CHG");
				break;
			default:
				break;
		}
		_display.setCursor(0, 42);
		_display.print("SLEEP_IN:");
		_display.print(PowerManager::get_sec_until_sleep());
        _display.print("s");

		_display.setCursor(0, 52);
		_display.print("FW_VER:");
		_display.print(ORBYTRACK_APP_VERSION);


		_display.display();
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
        btn_up_click_state_cb = btn_click;
        btn_dn_click_state_cb = btn_click;
        btn_ok_click_state_cb = btn_click;
        btn_ok_lpress_state_cb = btn_ok_long_press;
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
		flush_display();
	}

	UiStateData* get_data()
	{
		return &_data;
	}
}

namespace UiStateSettings
{
	UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};	
	uint32_t _last_draw_ts = 0;
	bool _update_screen  = false;
	uint8_t _selector_index = 0;
	uint8_t _page_index = 0;
	bool _item_selected = false;

    BlinkItem setting_title = 
    {
        .half_period_ms = 300,
        .last_change_ts = millis(),
        .process_on_period = [](const char* txt){
            _display.setTextColor(SH110X_WHITE);
            _display.print(txt);
        },
        .process_off_period = [](const char* txt){
            _display.setTextColor(SH110X_BLACK);
            _display.print(txt);
        }
    };

	struct SettingsItem
	{
		uint8_t index = 0;
		uint8_t page = 0;
		float offset;
		float min;
		float max;
		float val;
		uint8_t decimals;
		void (*on_load)(SettingsItem* item) = NULL;
		void (*on_save)(SettingsItem* item) = NULL;
		const char* name = NULL;
		const char* units = "";
	};

	SettingsItem items[] = 
	{
		{
			.offset = 1.0f,
			.min = 5.0f,
			.max = 40.0f,
			.val = (float) Settings::default_settings.shutdown_inactivity_minutes,
			.decimals = 0,
			.on_load = [](SettingsItem* item){item->val = Settings::get_shutdown_inactivity_minutes();},
			.on_save = [](SettingsItem* item){Settings::set_shutdown_inactivity_minutes((uint16_t) item->val);},
			.name = "TIME_OFF",
			.units = "mins"
		},
		{
			.offset = 0.05f,
			.min = 3.3f,
			.max = 3.6f,
			.val = Settings::default_settings.batt_cutoff,
			.decimals = 2,
			.on_load = [](SettingsItem* item){item->val = Settings::get_batt_cutoff();},
			.on_save = [](SettingsItem* item){Settings::set_batt_cutoff( item->val);},
			.name = "BATT_V_OFF",
			.units = "V"
		},
		{
			.offset = 4,
			.min = 1,
			.max = 255,
			.val = (float) Settings::default_settings.contrast_level,
			.decimals = 0,
			.on_load = [](SettingsItem* item){item->val = Settings::get_contrast_level();},
			.on_save = [](SettingsItem* item){
                    Settings::set_contrast_level((uint8_t) item->val);
                    _display.setContrast(Settings::get_contrast_level());
                },
			.name = "SCREEN_DIM"
		},
		{
			.offset = 1.0f,
			.min = 50.0f,
			.max = 500.0f,			
			.val = (float) Settings::default_settings.droplet_threshold,
			.decimals = 0,
			.on_load = [](SettingsItem* item){item->val = Settings::get_droplet_threshold_level();},
			.on_save = [](SettingsItem* item){Settings::set_droplet_threshold_level((uint16_t) item->val);},
			.name = "DROP_THLD"
		},
		{
			.offset = 5.0f,
			.min = 0.0f,
			.max = 100.0f,			
			.val = (float) Settings::default_settings.droplet_influence,
			.decimals = 0,
			.on_load = [](SettingsItem* item){item->val = Settings::get_droplet_influence_level();},
			.on_save = [](SettingsItem* item){Settings::set_droplet_influence_level((uint16_t) item->val);},
			.name = "DROP_INFL"
		}
	};

	void draw_selector()
	{
		int x1 = 0;
		int y1 = (_selector_index + 1) * 10 + 3;
		int x2 = 0;
		int y2 = (_selector_index + 1) * 10 + 10 - 3;
		int x3 = 4;
		int y3 = (_selector_index + 1) * 10 + 5;
		_display.drawLine(x1, y1, x2, y2, 1);
		_display.drawLine(x1, y1, x3, y3, 1);
		_display.drawLine(x2, y2, x3, y3, 1);
	}

	void draw()
	{
		_display.clearDisplay();
		_display.setTextSize(1);
		_display.setCursor(0, 0);
		_display.print("Settings");
		_display.drawLine(0, 9, 80, 9, 1);
		draw_selector();
		for(int i = 0; i < 5; i++)
		{
			_display.setCursor(6, (i+1) * 10 + 2);
            bool blink = _item_selected && _selector_index == i;
            process_blink_item(&setting_title, blink, items[i].name, &_update_screen);
            _display.setTextColor(SH110X_WHITE);
            _display.print(":");
			_display.print(items[i].val, items[i].decimals);
			_display.print(items[i].units);	
		}

		_display.display();
	}

	void btn_ok_long_press()
	{
		change_state(UiStateMain::get_data());
	}

	void btn_up_click()
	{
		if(!_item_selected)
		{
			if(_selector_index == 0) return;
			_selector_index--;
		}
		else
		{
			items[_selector_index].val += items[_selector_index].offset;
            if(items[_selector_index].val > items[_selector_index].max)
            {
                items[_selector_index].val = items[_selector_index].max;
            }
		}
		_update_screen = true;
	}

	void btn_dn_click()
	{
		if(!_item_selected)
		{
			if(_selector_index == 4) return;
			_selector_index++;
		}
		else
		{
			items[_selector_index].val -= items[_selector_index].offset;
            if(items[_selector_index].val < items[_selector_index].min)
            {
                items[_selector_index].val = items[_selector_index].min;
            }
		}
		_update_screen = true;
	}

	void btn_ok_click()
	{
		if(!_item_selected)
		{
			_item_selected = true;
		}
		else
		{
			items[_selector_index].on_save(&items[_selector_index]);
			Settings::save();
			_item_selected = false;
		}
		_update_screen = true;
	}

	void on_enter()
	{
		_data.entered_ts = millis();
        btn_up_click_state_cb = btn_up_click;
        btn_dn_click_state_cb = btn_dn_click;
        btn_ok_click_state_cb = btn_ok_click;
        btn_ok_lpress_state_cb = btn_ok_long_press;
		for(int i = 0; i < 5; i++)
		{
			items[i].on_load(&items[i]);
		}
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

		if(millis() - _last_draw_ts > 300)
		{
			draw();
			_last_draw_ts = millis();
		}
	}

	void on_exit()
	{
		flush_display();
	}

	UiStateData* get_data()
	{
		return &_data;
	}
}

namespace UiStateSleep
{
	UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};

	void on_enter()
	{
		_data.entered_ts = millis();
		_display.clearDisplay();
		_display.setTextSize(1);
		_display.setCursor(15, 27);
		_display.print("Shutting down...");
		_display.display();
	}

	void tick()
	{
		if(millis() - _data.entered_ts > SLEEP_LOGO_DURATION_MS)
		{
			flush_display();
			PowerManager::sleep();
			change_state(UiStateWokeup::get_data());
		}
	}

	void on_exit()
	{
		flush_display();
	}

	UiStateData* get_data()
	{
		return &_data;
	}
}

void init()
{
	_display.begin(DISPLAY_I2C_ADDR, true);
    _display.setRotation(3);
    _display.setTextColor(SH110X_WHITE);
	_display.setContrast(Settings::get_contrast_level());
	_display.clearDisplay();
	_display.display();
    _btn_up.attachClick(btn_up_click, &_btn_up);
	_btn_dn.attachClick(btn_dn_click, &_btn_dn);
	_btn_ok.attachClick(btn_ok_click, &_btn_ok);
	_btn_ok.attachLongPressStart(btn_ok_lpress, &_btn_ok);
	change_state(UiStateBoot::get_data());
}

void handle()
{
	_btn_dn.tick();
	_btn_ok.tick();
	_btn_up.tick();

	if(_next_state != _cur_state)
	{
		if(_cur_state != NULL &&  _cur_state->on_exit != NULL) _cur_state->on_exit();
		_cur_state = _next_state;
		if(_cur_state != NULL &&  _cur_state->on_enter != NULL) _cur_state->on_enter();
	}

	if(_cur_state != NULL &&  _cur_state->tick != NULL) _cur_state->tick();	
}

void change_state(UiStateData* ui_state)
{
	if(ui_state == NULL) return;
	_next_state = ui_state;	
}

}