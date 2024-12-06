#include "droplet_detector.h"
#include <string.h>
#include "avdweb_AnalogReadFast.h"
#include <RingBuf.h>
#include "pinout.h"
#include "common.h"
#include "settings.h"

#define USING_TIMER_TC3         true
#define USING_TIMER_TC4         false
#define USING_TIMER_TC5         false
#define USING_TIMER_TCC         false
#define USING_TIMER_TCC1        false
#define USING_TIMER_TCC2        false

#include "SAMDTimerInterrupt.h"

namespace DropletDetector
{

DetectedDroplet _latest_detected_droplet = {};
DetectedDroplet _cur_detected_droplet = {};
RingBuf<uint16_t, PT_READ_BUFF_SIZE> _pt_read_buff;
SAMDTimer _pt_read_timer(TIMER_TC3);
void (*_detection_cb)(DetectedDroplet* latest_detected_droplet) = NULL;
bool _is_enabled = false;
uint16_t _bg_val = 0;
uint8_t _tick_counter = 0;

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
	switch(_tick_counter)
	{
		case 0:
			
			break;
		case 1:
			break;
		case 2:
            _bg_val = analogReadFast(PIN_IR_PT);
            set_ir_led_state(true);
			break;
		case 3:
			break;
		case 4:
			uint16_t pt_val = _bg_val - analogReadFast(PIN_IR_PT);
			if(_is_enabled) _pt_read_buff.push(&pt_val);
			set_ir_led_state(false);
			break;
	}
	
	_tick_counter++;
	if(_tick_counter > 4) _tick_counter = 0;
}

void detected_droplet_init(DetectedDroplet *droplet)
{
	droplet->highs = 0;
	droplet->leading_zeros = 0;
	droplet->lows = 0;
	droplet->span = 0;
	droplet->trailing_zeros = 0;
	droplet->detection_state = DropletDetectionState::LEADING_ZEROS;
    droplet->period_ms = 0;
    droplet->ts = 0;
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
    detected_droplet_init(&_cur_detected_droplet);
    _bg_val = 0;
    _tick_counter = 0;
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

uint16_t droplet_ms_to_sample_count(uint16_t ms)
{
    return ((1000 * ms / PT_SAMPLING_INTERVAL_US));
}

bool detect_droplet(uint16_t new_pt_sample, uint16_t threshold, uint16_t influence)
{
	static uint16_t last_filtered_pt = 0;

	int peak_value = 0;
	
	if(new_pt_sample > last_filtered_pt + threshold) peak_value = 1;
	else if(new_pt_sample < last_filtered_pt - threshold) peak_value = -1;
	else peak_value = 0;
    last_filtered_pt = (last_filtered_pt * (100 - influence) + new_pt_sample * influence) / 100;

	switch(_cur_detected_droplet.detection_state)
	{
		case DropletDetectionState::LEADING_ZEROS:
			if(peak_value == 0)
			{
				_cur_detected_droplet.leading_zeros++;
				break;
			}
			else
			{
				if(_cur_detected_droplet.leading_zeros >= droplet_ms_to_sample_count(Settings::get_droplet_gap_min()))
				{
					_cur_detected_droplet.detection_state = DropletDetectionState::NON_ZEROS;
					// Serial.println("St:NZ");
				}
				else
				{
					_cur_detected_droplet.leading_zeros = 0;
					break;
				}
			}        
		case DropletDetectionState::NON_ZEROS:
			_cur_detected_droplet.span++;
			if(peak_value == 1)
			{
				_cur_detected_droplet.highs++;
				_cur_detected_droplet.trailing_zeros = 0;
				break;
			}
			else if(peak_value == -1)
			{
				_cur_detected_droplet.lows++;
				_cur_detected_droplet.trailing_zeros = 0;
				break;
			}
			else if(peak_value == 0)
			{
				_cur_detected_droplet.trailing_zeros++;
			}
			if(_cur_detected_droplet.trailing_zeros < droplet_ms_to_sample_count(Settings::get_droplet_gap_min())) break;
			else _cur_detected_droplet.detection_state = DropletDetectionState::TRAILING_ZEROS;
		case DropletDetectionState::TRAILING_ZEROS:
			_cur_detected_droplet.span -= _cur_detected_droplet.trailing_zeros;
			if(_cur_detected_droplet.span > droplet_ms_to_sample_count(Settings::get_droplet_span_min()) && _cur_detected_droplet.span < droplet_ms_to_sample_count(Settings::get_droplet_span_max()))
			{
				_cur_detected_droplet.detection_state = DropletDetectionState::DETECTED;
			}
			else
			{
				detected_droplet_init(&_cur_detected_droplet);
				break;
			}
		case DropletDetectionState::DETECTED:
			if(_latest_detected_droplet.ts != 0)
			{
				uint32_t new_period = millis() - _latest_detected_droplet.ts;
                if(_latest_detected_droplet.period_ms == 0) _cur_detected_droplet.period_ms = new_period;
				else _cur_detected_droplet.period_ms = (50 * new_period + 50 * _latest_detected_droplet.period_ms) / 100;
			}
			_cur_detected_droplet.ts = millis();
			memcpy(&_latest_detected_droplet, &_cur_detected_droplet, sizeof(DetectedDroplet));
			detected_droplet_init(&_cur_detected_droplet);
			return true;
		case DropletDetectionState::NONE:
			break;
	}
	return false;
}

void handle()
{
	uint16_t pt_val = 0;
	while(_is_enabled && !_pt_read_buff.isEmpty() && _pt_read_buff.lockedPop(pt_val))
	{
		if(DropletDetector::detect_droplet(pt_val, Settings::get_droplet_threshold_level(), Settings::get_droplet_influence_level()))
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