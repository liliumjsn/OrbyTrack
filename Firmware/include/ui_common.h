#ifndef UI_H
#define UI_H

#include "common.h"
#define SH110X_NO_SPLASH

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "settings.h"
#include "ui_bitmaps.h"
#include "power_manager.h"
#include "droplet_detector.h"

namespace UiCommon
{

struct UiStateData
{
    uint32_t entered_ts = 0;
    void (*on_enter)() = NULL;
    void (*tick)() = NULL;
    void (*on_exit)() = NULL;
};

struct BlinkItem
{
    uint16_t half_period_ms = 0;
    uint32_t last_change_ts = 0;
    void (*process_on_period)(const char* txt);
    void (*process_off_period)(const char* txt);
    bool is_off_period = false;
};

void init();
void handle();
void change_state(UiStateData* ui_state);
void process_blink_item(BlinkItem* item, bool blink, const char* txt, bool* update_scr);
void show_logo();
void detach_all_btn();
void flush_display();
Adafruit_SH1107* get_display();
void attach_btn_ok_click(void (*btn_cb)());
void attach_btn_up_click(void (*btn_cb)());
void attach_btn_dn_click(void (*btn_cb)());
void attach_btn_ok_lpress(void (*btn_cb)());
void shutdown(const char* msg = NULL);

}

namespace UiStateBoot
{
    void on_enter();
	void tick();
    void on_exit();
    UiCommon::UiStateData* get_data();
}

namespace UiStateMain
{
    void on_enter();
	void tick();
    void on_exit();
    void droplet_detected_handler();
    UiCommon::UiStateData* get_data();
}

namespace UiStateInfo
{
    void on_enter();
	void tick();
    void on_exit();
    UiCommon::UiStateData* get_data();
}

namespace UiStateSettings
{
    void on_enter();
	void tick();
    void on_exit();
    UiCommon::UiStateData* get_data();
}

namespace UiStateSleep
{
    void on_enter();
	void tick();
    void on_exit();
    void set_shutdown_msg(const char* msg);
    UiCommon::UiStateData* get_data();
}

namespace UiStateWokeup
{
    void on_enter();
	void tick();
    void on_exit();
    UiCommon::UiStateData* get_data();
}

#endif