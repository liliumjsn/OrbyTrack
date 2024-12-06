#include "settings.h"
#define FLASH_DEBUG       0
#include <FlashStorage_SAMD.h>
#include <CRC32.h>

// Reserve a portion of flash memory to store an "int" variable
// and call it "my_flash_store".


namespace Settings
{

FlashStorage(setting_shutdown_inactivity_minutes, uint16_t);
FlashStorage(setting_measuring_type, MeasuringType);
FlashStorage(setting_batt_cutoff, float);
FlashStorage(setting_contrast_level, uint8_t);
FlashStorage(setting_droplet_threshold, uint16_t);
FlashStorage(setting_droplet_influence, uint16_t);
FlashStorage(setting_droplet_span_min, uint16_t);
FlashStorage(setting_droplet_span_max, uint16_t);
FlashStorage(setting_droplet_gap_min, uint16_t);
FlashStorage(magic_bytes, uint32_t);

Data current_settings = {};

void init()
{
	if(load() != RetRes::OK)
	{
		Serial.println("Settings load failed or first run");
		memcpy(&current_settings, &default_settings, sizeof(Data));
		save();
	}
}

void print_current_settings()
{
	Serial.println("============ Current Settings ============");
	Serial.print("setting_shutdown_inactivity_minutes: ");
	Serial.println(current_settings.shutdown_inactivity_minutes);
	Serial.print("setting_measuring_type: ");
	Serial.println((uint16_t) current_settings.measuring_type);
	Serial.print("setting_batt_cutoff: ");
	Serial.println(current_settings.batt_cutoff);
	Serial.print("setting_contrast_level: ");
	Serial.println(current_settings.contrast_level);
	Serial.print("setting_droplet_threshold: ");
	Serial.println(current_settings.droplet_threshold);
	Serial.print("setting_droplet_influence: ");
	Serial.println(current_settings.droplet_influence);
    Serial.print("setting_droplet_span_min: ");
	Serial.println(current_settings.droplet_span_min);
    Serial.print("setting_droplet_span_max: ");
	Serial.println(current_settings.droplet_span_max);
    Serial.print("setting_droplet_gap_min: ");
	Serial.println(current_settings.droplet_gap_min);
	Serial.println("==========================================");
}

RetRes load()
{
	setting_shutdown_inactivity_minutes.read(current_settings.shutdown_inactivity_minutes);
	setting_measuring_type.read(current_settings.measuring_type);
	setting_batt_cutoff.read(current_settings.batt_cutoff);
	setting_contrast_level.read(current_settings.contrast_level);
	setting_droplet_threshold.read(current_settings.droplet_threshold);
	setting_droplet_influence.read(current_settings.droplet_influence);
    setting_droplet_gap_min.read(current_settings.droplet_gap_min);
    setting_droplet_span_max.read(current_settings.droplet_span_max);
    setting_droplet_span_min.read(current_settings.droplet_span_min);

	uint32_t magic_bytes_read;
	magic_bytes.read(magic_bytes_read);
	Serial.print("magic_bytes_read: ");
	Serial.println(magic_bytes_read);

	print_current_settings();

	if(magic_bytes_read != STORAGE_MAGIC_BYTES_VALAUE) return RetRes::ERROR;

	return RetRes::OK;
}

RetRes save()
{
	setting_shutdown_inactivity_minutes.write(current_settings.shutdown_inactivity_minutes);
	setting_measuring_type.write(current_settings.measuring_type);
	setting_batt_cutoff.write(current_settings.batt_cutoff);
	setting_contrast_level.write(current_settings.contrast_level);
	setting_droplet_threshold.write(current_settings.droplet_threshold);
	setting_droplet_influence.write(current_settings.droplet_influence);
    setting_droplet_gap_min.write(current_settings.droplet_gap_min);
    setting_droplet_span_max.write(current_settings.droplet_span_max);
    setting_droplet_span_min.write(current_settings.droplet_span_min);

	uint32_t tmp = STORAGE_MAGIC_BYTES_VALAUE;
	magic_bytes.write(tmp);
	uint32_t magic_bytes_read;
	magic_bytes.read(magic_bytes_read);
	Serial.print("magic_bytes_read: ");
	Serial.println(magic_bytes_read);

	print_current_settings();

	if(magic_bytes_read != STORAGE_MAGIC_BYTES_VALAUE) return RetRes::ERROR;

	return RetRes::OK;
}

uint16_t get_shutdown_inactivity_minutes()
{
	return current_settings.shutdown_inactivity_minutes;
}

void set_shutdown_inactivity_minutes(uint16_t val)
{
	current_settings.shutdown_inactivity_minutes = val;
}

MeasuringType get_measuring_type()
{
	return current_settings.measuring_type;
}

void set_measuring_type(MeasuringType val)
{
	current_settings.measuring_type = val;
}

float get_batt_cutoff()
{
	return current_settings.batt_cutoff;
}

void set_batt_cutoff(float val)
{
	current_settings.batt_cutoff = val;
}

uint8_t get_contrast_level()
{
	return current_settings.contrast_level;
}

void set_contrast_level(uint8_t val)
{
	current_settings.contrast_level = val;
}

uint16_t get_droplet_threshold_level()
{
	return current_settings.droplet_threshold;
}

void set_droplet_threshold_level(uint16_t val)
{
	current_settings.droplet_threshold = val;
}

uint16_t get_droplet_influence_level()
{
	return current_settings.droplet_influence;
}

void set_droplet_influence_level(uint16_t val)
{
	current_settings.droplet_influence = val;
}

uint16_t get_droplet_span_min()
{
    return current_settings.droplet_span_min;
}

void set_droplet_span_min(uint16_t val)
{
    current_settings.droplet_span_min = val;
}

uint16_t get_droplet_span_max()
{
    return current_settings.droplet_span_max;
}

void set_droplet_span_max(uint16_t val)
{
    current_settings.droplet_span_max = val;
}

uint16_t get_droplet_gap_min()
{
    return current_settings.droplet_gap_min;
}

void set_droplet_gap_min(uint16_t val)
{
    current_settings.droplet_gap_min = val;
}

}