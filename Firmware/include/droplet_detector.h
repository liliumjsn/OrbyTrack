#ifndef DROPLET_DETECTOR_H
#define DROPLET_DETECTOR_H

#include "common.h"

namespace DropletDetector
{

// Peak width max 70ms
// Searching for 20ms zeros, 70ms peaks, 20ms zeros pattern

const uint8_t DROPLET_EVAL_ZERO_SPAN_MIN = 40; // 20 ms @ 0.5ms sampling rate
const uint8_t DROPLET_EVAL_NONZERO_SPAN_MAX = 80; // 40 ms @ 0.5ms sampling rate
const uint8_t DROPLET_EVAL_NONZERO_SPAN_MIN = 10; // 7 ms @ 0.5ms sampling rate
const uint8_t DROPLET_MACRO_PER_ML = 20;
const uint8_t DROPLET_MICRO_PER_ML = 60;

enum class DropletDetectionState
{
	NONE = 0,
	LEADING_ZEROS,
	NON_ZEROS,
    TRAILING_ZEROS,
	DETECTED
};

struct DetectedDroplet
{
	uint32_t leading_zeros = 0;
	uint8_t trailing_zeros = 0;
	uint8_t highs = 0;
	uint8_t lows = 0;
	uint8_t span = 0;
	uint32_t drop_index = 0;
	uint32_t ts = 0;
	uint32_t period_ms = 0;
    DropletDetectionState detection_state = DropletDetectionState::NONE;
};

float get_drops_per_min();
float get_drops_per_hour();
float get_ml_per_min(bool is_micro = false);
float get_ml_per_hour(bool is_micro = false);
void init();
void enable();
void disable();
void handle();
bool set_detection_cb(void (*detection_cb)(DetectedDroplet* latest_detected_droplet));

}

#endif