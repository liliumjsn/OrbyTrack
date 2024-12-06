#ifndef SETTINGS_H
#define SETTINGS_H

#include "common.h"

namespace Settings
{

enum class MeasuringType
	{
		NONE = 0,
		MACRO,
		MICRO,
		GTT
	};

struct Data
{
	uint16_t shutdown_inactivity_minutes;
	MeasuringType measuring_type;
	float batt_cutoff;
	uint8_t contrast_level;
	uint16_t droplet_threshold;
	uint16_t droplet_influence;
    uint16_t droplet_span_min;
    uint16_t droplet_span_max;
    uint16_t droplet_gap_min;
};

const Data default_settings = 
{
	.shutdown_inactivity_minutes = 10,
	.measuring_type = MeasuringType::MACRO,
	.batt_cutoff = 3.5f,
	.contrast_level = 5,
	.droplet_threshold = 300,
	.droplet_influence = 10,
    .droplet_span_min = 5,
    .droplet_span_max= 40,	
    .droplet_gap_min = 20
};

void init();
RetRes load();
RetRes save();

uint16_t get_shutdown_inactivity_minutes();
void set_shutdown_inactivity_minutes(uint16_t val);

MeasuringType get_measuring_type();
void set_measuring_type(MeasuringType val);

float get_batt_cutoff();
void set_batt_cutoff(float val);

uint8_t get_contrast_level();
void set_contrast_level(uint8_t val);

uint16_t get_droplet_threshold_level();
void set_droplet_threshold_level(uint16_t val);

uint16_t get_droplet_influence_level();
void set_droplet_influence_level(uint16_t val);

uint16_t get_droplet_span_min();
void set_droplet_span_min(uint16_t val);

uint16_t get_droplet_span_max();
void set_droplet_span_max(uint16_t val);

uint16_t get_droplet_gap_min();
void set_droplet_gap_min(uint16_t val);

}

#endif