#include "droplet_detector.h"
#include <string.h>
#include "avdweb_AnalogReadFast.h"
#include <RingBuf.h>
#include "pinout.h"
#include "common.h"
#include "settings.h"

#define USING_TIMER_TC3         true      // Only TC3 can be used for SAMD51
#define USING_TIMER_TC4         false     // Not to use with Servo library
#define USING_TIMER_TC5         false
#define USING_TIMER_TCC         false
#define USING_TIMER_TCC1        false
#define USING_TIMER_TCC2        false

#include "SAMDTimerInterrupt.h"

namespace DropletDetector
{

DetectedDroplet _latest_detected_droplet = {};
RingBuf<uint16_t, PT_READ_BUFF_SIZE> _pt_read_buff;
SAMDTimer _pt_read_timer(TIMER_TC3);
void (*_detection_cb)(DetectedDroplet* latest_detected_droplet) = NULL;
bool _is_enabled = false;

void set_ir_led_state(bool state)
{
	digitalWrite(PIN_IR_LED, state);
}

bool set_detection_cb(void (*detection_cb)(DetectedDroplet* latest_detected_droplet))
{
	if(_detection_cb == NULL) _detection_cb = detection_cb;
	else return false;
	return true;
}

void pt_read_timer_isr()
{
	static uint16_t bg_val = 0;
	static uint8_t tick_counter = 0;

	switch(tick_counter)
	{
		case 0:
			
			break;
		case 1:
			break;
		case 2:
            bg_val = analogReadFast(PIN_IR_PT);
            set_ir_led_state(true);
			break;
		case 3:
			break;
		case 4:
			uint16_t pt_val = bg_val - analogReadFast(PIN_IR_PT);
			if(_is_enabled) _pt_read_buff.push(&pt_val);
			set_ir_led_state(false);
			break;
	}
	
	tick_counter++;
	if(tick_counter > 4) tick_counter = 0;
}

void detected_droplet_init(DetectedDroplet *droplet)
{
	droplet->highs = 0;
	droplet->leading_zeros = 0;
	droplet->lows = 0;
	droplet->span = 0;
	droplet->trailing_zeros = 0;
	droplet->detection_state = DropletDetectionState::LEADING_ZEROS;
}

void init()
{
	pinMode(PIN_IR_PT, INPUT);
    pinMode(PIN_IR_LED, OUTPUT);
	analogReadResolution(12);
	detected_droplet_init(&_latest_detected_droplet);
	set_ir_led_state(false);
	_pt_read_timer.disableTimer();
	_pt_read_timer.attachInterruptInterval(PT_TIMER_INTERVAL_US, pt_read_timer_isr);
}

void enable()
{
	_is_enabled = true;	
    detected_droplet_init(&_latest_detected_droplet);
    _latest_detected_droplet.period_ms = 0;
    _pt_read_timer.enableTimer();
}

void disable()
{
	_is_enabled = false;
	_pt_read_timer.disableTimer();
	set_ir_led_state(false);
}

float get_drops_per_min()
{
	if(_latest_detected_droplet.period_ms == 0) return 0.0f;
	if(_latest_detected_droplet.period_ms > DROPLET_PERIOD_MAX_MS) return 0.0f;
    if(millis() - _latest_detected_droplet.ts > DROPLET_PERIOD_MAX_MS) return 0.0f;

	float freq = 1000.0f/(float)_latest_detected_droplet.period_ms;
	return freq * 60.0f;
}

float get_drops_per_hour()
{
	return get_drops_per_min() * 60;
}

float get_ml_per_min(bool is_micro)
{
	if(is_micro) return get_drops_per_min() / (float) DROPLET_MICRO_PER_ML;
	return get_drops_per_min() / (float) DROPLET_MACRO_PER_ML;	
}

float get_ml_per_hour(bool is_micro)
{
	if(is_micro) return get_drops_per_hour() / (float) DROPLET_MICRO_PER_ML;
	return get_drops_per_hour() / (float) DROPLET_MACRO_PER_ML;	
}

bool detect_droplet(uint16_t new_pt_sample, uint32_t ts, DetectedDroplet *droplet, uint16_t threshold, uint16_t influence)
{
	static DetectedDroplet detecting_droplet = {};
	if(detecting_droplet.detection_state == DropletDetectionState::NONE)
	{
		detected_droplet_init(&detecting_droplet);
	}

	static uint16_t last_filtered_pt = 0;

	int peak_value = 0;
	uint16_t filtered_pt = (last_filtered_pt * (100 - influence) + new_pt_sample * influence) / 100;
	last_filtered_pt = filtered_pt;
	if(new_pt_sample > filtered_pt + threshold) peak_value = 1;
	else if(new_pt_sample < filtered_pt - threshold) peak_value = -1;
	else peak_value = 0;

	switch(detecting_droplet.detection_state)
	{
		case DropletDetectionState::LEADING_ZEROS:
			if(peak_value == 0)
			{
				detecting_droplet.leading_zeros++;
				break;
			}
			else
			{
				if(detecting_droplet.leading_zeros >= Settings::get_droplet_gap_min())
				{
					detecting_droplet.detection_state = DropletDetectionState::NON_ZEROS;
					// Serial.println("St:NZ");
				}
				else
				{
					detecting_droplet.leading_zeros = 0;
					break;
				}
			}        
		case DropletDetectionState::NON_ZEROS:
			detecting_droplet.span++;
			if(peak_value == 1)
			{
				detecting_droplet.highs++;
				detecting_droplet.trailing_zeros = 0;
				break;
			}
			else if(peak_value == -1)
			{
				detecting_droplet.lows++;
				detecting_droplet.trailing_zeros = 0;
				break;
			}
			else if(peak_value == 0)
			{
				detecting_droplet.trailing_zeros++;
			}
			if(detecting_droplet.trailing_zeros < Settings::get_droplet_gap_min()) break;
			else detecting_droplet.detection_state = DropletDetectionState::TRAILING_ZEROS;
		case DropletDetectionState::TRAILING_ZEROS:
			detecting_droplet.span -= detecting_droplet.trailing_zeros;
			if(detecting_droplet.span > Settings::get_droplet_span_min() && detecting_droplet.span < Settings::get_droplet_span_max())
			{
				detecting_droplet.detection_state = DropletDetectionState::DETECTED;
			}
			else
			{
				detected_droplet_init(&detecting_droplet);
				break;
			}
		case DropletDetectionState::DETECTED:
			if(detecting_droplet.ts != 0)
			{
				uint32_t new_period = ts - detecting_droplet.ts;
				detecting_droplet.period_ms = (50 * new_period + 50 * detecting_droplet.period_ms) / 100;
			}
			detecting_droplet.ts = ts;
			detecting_droplet.drop_index++;
			memcpy(droplet, &detecting_droplet, sizeof(DetectedDroplet));
			detected_droplet_init(&detecting_droplet);
			return true;
		case DropletDetectionState::NONE:
			break;
	}
	return false;
}

void handle()
{
	uint16_t pt_val = 0;
	while(_is_enabled && _pt_read_buff.lockedPop(pt_val))
	{
		if(DropletDetector::detect_droplet(pt_val, millis(), &_latest_detected_droplet, Settings::get_droplet_threshold_level(), Settings::get_droplet_influence_level()))
		{
			if(_detection_cb != NULL) _detection_cb(&_latest_detected_droplet);
            break;
		}
	}
    if(_pt_read_buff.isFull())
    {
        Serial.println("OF");
    }
}
	
}