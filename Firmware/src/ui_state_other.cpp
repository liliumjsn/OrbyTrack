#include "ui_common.h"

namespace UiStateBoot
{
	UiCommon::UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};

	void on_enter()
	{
		_data.entered_ts = millis();
		UiCommon::detach_all_btn();
		UiCommon::show_logo();
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
		UiCommon::flush_display();
	}

	UiCommon::UiStateData* get_data()
	{
		return &_data;
	}
}

namespace UiStateWokeup
{
	UiCommon::UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};
	
	void on_enter()
	{
		_data.entered_ts = millis();
        UiCommon::get_display()->setContrast(Settings::get_contrast_level());
		UiCommon::detach_all_btn();
		UiCommon::show_logo();
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
		UiCommon::flush_display();
	}

	UiCommon::UiStateData* get_data()
	{
		return &_data;
	}
}

namespace UiStateSleep
{
	UiCommon::UiStateData _data = {.on_enter = on_enter, .tick = tick, .on_exit = on_exit};

    const char* shutdown_msg = NULL;

    void set_shutdown_msg(const char* msg)
    {
        shutdown_msg = msg;
    }

	void on_enter()
	{
		_data.entered_ts = millis();
		UiCommon::get_display()->clearDisplay();
		UiCommon::get_display()->setTextSize(1);
        if(shutdown_msg != NULL)
        {
            int msg_len = strlen(shutdown_msg);
            int x_offs = (SCREEN_WIDTH - (msg_len * 6)) / 2; // TODO: Make generic centering offs function
            UiCommon::get_display()->setCursor(x_offs, 20);
            UiCommon::get_display()->print(shutdown_msg);
        }
		UiCommon::get_display()->setCursor(15, 30);
		UiCommon::get_display()->print("Shutting down...");
		UiCommon::get_display()->display();
	}

	void tick()
	{
		if(millis() - _data.entered_ts > SLEEP_LOGO_DURATION_MS)
		{
			UiCommon::flush_display();
			PowerManager::sleep();
			change_state(UiStateWokeup::get_data());
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