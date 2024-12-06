#include "ui_common.h"

namespace UiStateSettings
{
	UiCommon::UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};	
	uint32_t _last_draw_ts = 0;
	bool _update_screen  = false;
	uint8_t _selector_index = 0;
	uint8_t _page_index = 0;
	bool _is_item_selected = false;

	UiCommon::BlinkItem setting_title = 
	{
		.half_period_ms = 300,
		.last_change_ts = millis(),
		.process_on_period = [](const char* txt){
			UiCommon::get_display()->setTextColor(SH110X_WHITE);
			UiCommon::get_display()->print(txt);
		},
		.process_off_period = [](const char* txt){
			UiCommon::get_display()->setTextColor(SH110X_BLACK);
			UiCommon::get_display()->print(txt);
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

	SettingsItem _items[] = 
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
					UiCommon::get_display()->setContrast(Settings::get_contrast_level());
				},
			.name = "SCREEN_CONT"
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
		},
		{
			.offset = 1.0f,
			.min = 1.0f,
			.max = 100.0f,			
			.val = (float) Settings::default_settings.droplet_influence,
			.decimals = 0,
			.on_load = [](SettingsItem* item){
				item->val = Settings::get_droplet_span_min();
				item->max = Settings::get_droplet_span_max() - item->offset;
			},
			.on_save = [](SettingsItem* item){
				Settings::set_droplet_span_min((uint16_t) item->val);
				item->max = Settings::get_droplet_span_max() - item->offset;
				},
			.name = "DROP_SPAN_MIN",
			.units = "ms"
		},
		{
			.offset = 1.0f,
			.min = 1.0f,
			.max = 100.0f,			
			.val = (float) Settings::default_settings.droplet_influence,
			.decimals = 0,
			.on_load = [](SettingsItem* item){
				item->val = Settings::get_droplet_span_max();
				item->min = Settings::get_droplet_span_min() + item->offset;
			},
			.on_save = [](SettingsItem* item){
				Settings::set_droplet_span_max((uint16_t) item->val);
				item->min = Settings::get_droplet_span_min() + item->offset;
			},
			.name = "DROP_SPAN_MAX",
			.units = "ms"
		},
		{
			.offset = 1.0f,
			.min = 1.0f,
			.max = 100.0f,			
			.val = (float) Settings::default_settings.droplet_influence,
			.decimals = 0,
			.on_load = [](SettingsItem* item){item->val = Settings::get_droplet_gap_min();},
			.on_save = [](SettingsItem* item){Settings::set_droplet_gap_min((uint16_t) item->val);},
			.name = "DROP_GAP_MIN",
			.units = "ms"
		}
	};

	const uint8_t ITEMS_COUNT = (sizeof(_items)/sizeof(SettingsItem));
	const uint8_t PAGE_COUNT = ((ITEMS_COUNT + SETTINGS_ITEMS_PER_PAGE - 1) / SETTINGS_ITEMS_PER_PAGE);

	uint8_t item_idx(uint8_t selector_index)
	{
		return (_page_index - 1) * SETTINGS_ITEMS_PER_PAGE + selector_index;
	}

	uint8_t items_in_page()
	{
		uint8_t page_items_count = SETTINGS_ITEMS_PER_PAGE;
		if(_page_index == PAGE_COUNT) page_items_count = ITEMS_COUNT - (_page_index - 1) * SETTINGS_ITEMS_PER_PAGE;
		return page_items_count;
	}

	void draw_selector()
	{
		int x1 = 0;
		int y1 = (_selector_index + 1) * 10 + 3;
		int x2 = 0;
		int y2 = (_selector_index + 1) * 10 + 10 - 3;
		int x3 = 4;
		int y3 = (_selector_index + 1) * 10 + 5;
		UiCommon::get_display()->drawLine(x1, y1, x2, y2, 1);
		UiCommon::get_display()->drawLine(x1, y1, x3, y3, 1);
		UiCommon::get_display()->drawLine(x2, y2, x3, y3, 1);
	}

	void draw_sidebar()
	{
		uint8_t x_off = SCREEN_WIDTH - 1;
		uint8_t bar_bg_y1 = 12;
		uint8_t bar_bg_y2 = SCREEN_HEIGHT - 1;
		uint8_t bar_bg_delta = bar_bg_y2 - bar_bg_y1;
		UiCommon::get_display()->drawLine(x_off, bar_bg_y1, x_off, bar_bg_y2, 1);

		uint8_t bar_y1 = (_page_index - 1) * (bar_bg_delta / PAGE_COUNT) + bar_bg_y1;
		uint8_t bar_y2 = (_page_index) * (bar_bg_delta / PAGE_COUNT) + bar_bg_y1;

		if(_page_index == 1) bar_y1 = bar_bg_y1;
		if(_page_index == PAGE_COUNT) bar_y2 = bar_bg_y2;

		UiCommon::get_display()->drawLine(x_off - 1, bar_y1, x_off - 1, bar_y2, 1);
		UiCommon::get_display()->drawLine(x_off - 2, bar_y1, x_off - 2, bar_y2, 1);
	}

	void draw()
	{
		UiCommon::get_display()->clearDisplay();
		UiCommon::get_display()->setTextSize(1);
		UiCommon::get_display()->setCursor(0, 0);
		UiCommon::get_display()->print("Settings");
		UiCommon::get_display()->drawLine(0, 9, 80, 9, 1);
		UiCommon::get_display()->setCursor(104, 0);
		UiCommon::get_display()->print(_page_index);
		UiCommon::get_display()->print("/");
		UiCommon::get_display()->print(PAGE_COUNT);

		draw_selector();
		draw_sidebar();

		for(int i = 0; i < items_in_page(); i++)
		{
			UiCommon::get_display()->setCursor(6, (i + 1) * 10 + 2);
			bool blink = _is_item_selected && _selector_index == i;
			UiCommon::process_blink_item(&setting_title, blink, _items[item_idx(i)].name, &_update_screen);
			UiCommon::get_display()->setTextColor(SH110X_WHITE);
			UiCommon::get_display()->print(":");
			UiCommon::get_display()->print(_items[item_idx(i)].val, _items[item_idx(i)].decimals);
			UiCommon::get_display()->print(_items[item_idx(i)].units);	
		}

		UiCommon::get_display()->display();
	}

	void btn_ok_long_press()
	{
		change_state(UiStateMain::get_data());
	}

	void btn_up_click()
	{
		if(!_is_item_selected)
		{
			if(_selector_index == 0)
			{
				if(_page_index == 1) return;
				else
				{
					_page_index--;
					_selector_index = SETTINGS_ITEMS_PER_PAGE - 1;
				}
			}
			else _selector_index--;
		}
		else
		{
			SettingsItem* selected_item = &_items[item_idx(_selector_index)];
			selected_item->val += selected_item->offset;
			if(selected_item->val > selected_item->max)
			{
				selected_item->val = selected_item->max;
			}
		}
		_update_screen = true;
	}

	void btn_dn_click()
	{
		if(!_is_item_selected)
		{
			if(_selector_index == items_in_page() - 1)
			{
				if(_page_index == PAGE_COUNT) return;
				else
				{
					_page_index++;
					_selector_index = 0;
				}
			}
			else _selector_index++;
		}
		else
		{
			SettingsItem* selected_item = &_items[item_idx(_selector_index)];
			selected_item->val -= selected_item->offset;
			if(selected_item->val < selected_item->min)
			{
				selected_item->val = selected_item->min;
			}
		}
		_update_screen = true;
	}

	void btn_ok_click()
	{
		if(!_is_item_selected)
		{
			_is_item_selected = true;
		}
		else
		{
			_items[item_idx(_selector_index)].on_save(&_items[item_idx(_selector_index)]);
			Settings::save();
			_is_item_selected = false;
		}
		_update_screen = true;
	}

	void on_enter()
	{
		UiCommon::attach_btn_dn_click(btn_dn_click);
		UiCommon::attach_btn_ok_click(btn_ok_click);
		UiCommon::attach_btn_up_click(btn_up_click);
		UiCommon::attach_btn_ok_lpress(btn_ok_long_press);
		_page_index = 1;
		_selector_index = 0;
		for(int i = 0; i < ITEMS_COUNT; i++)
		{
			_items[i].on_load(&_items[i]);
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
		UiCommon::flush_display();
	}

	UiCommon::UiStateData* get_data()
	{
		return &_data;
	}
}