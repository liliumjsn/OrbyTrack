#ifndef UI_H
#define UI_H

#include "common.h"

namespace Ui
{

struct UiStateData
{
    uint32_t entered_ts = 0;
    void (*on_enter)() = NULL;
    void (*tick)() = NULL;
    void (*on_exit)() = NULL;
};

namespace UiStateBoot
{
    void on_enter();
	void tick();
    void on_exit();
    UiStateData* get_data();
}

namespace UiStateMain
{
    void on_enter();
	void tick();
    void on_exit();
    UiStateData* get_data();
}

namespace UiStateInfo
{
    void on_enter();
	void tick();
    void on_exit();
    UiStateData* get_data();
}

namespace UiStateSettings
{
    void on_enter();
	void tick();
    void on_exit();
    UiStateData* get_data();
}

namespace UiStateSleep
{
    void on_enter();
	void tick();
    void on_exit();
    UiStateData* get_data();
}

namespace UiStateWokeup
{
    void on_enter();
	void tick();
    void on_exit();
    UiStateData* get_data();
}

void init();
void handle();
void change_state(UiStateData* ui_state);

}

#endif