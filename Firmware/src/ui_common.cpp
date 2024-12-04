#include "ui_common.h"
#include "OneButton.h"
#include "pinout.h"

namespace UiCommon
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

void attach_btn_ok_click(void (*btn_cb)())
{       
    btn_ok_click_state_cb = btn_cb;
}

void attach_btn_up_click(void (*btn_cb)())
{
    btn_up_click_state_cb = btn_cb;
}

void attach_btn_dn_click(void (*btn_cb)())
{
    btn_dn_click_state_cb = btn_cb;
}

void attach_btn_ok_lpress(void (*btn_cb)())
{
    btn_ok_lpress_state_cb = btn_cb;
}

Adafruit_SH1107* get_display()
{
    return &_display;
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
	_display.drawBitmap(0, 15, epd_bitmap_logo_name, 128, 28, 1);
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

void shutdown(const char* msg)
{
    UiStateSleep::set_shutdown_msg(msg);
    change_state(UiStateSleep::get_data());
}

}