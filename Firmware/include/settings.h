#ifndef SETTINGS_H
#define SETTINGS_H

#include "common.h"
#include "ui.h"

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
};

const Data default_settings = 
{
	.shutdown_inactivity_minutes = 10,
	.measuring_type = MeasuringType::MACRO,
	.batt_cutoff = 3.5f,
	.contrast_level = 5,
	.droplet_threshold = 300,
	.droplet_influence = 10
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

}

#endif