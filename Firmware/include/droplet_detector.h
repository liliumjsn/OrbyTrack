#ifndef DROPLET_DETECTOR_H
#define DROPLET_DETECTOR_H

#include "common.h"

namespace DropletDetector
{

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