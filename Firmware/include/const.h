#ifndef CONST_H
#define CONST_H

#include "common.h"

#define ORBYTRACK_APP_VERSION "v1.0"

const int SCREEN_WIDTH = 128;  // OLED display width, in pixels
const int SCREEN_HEIGHT = 64; // OLED display height, in pixels

const float CHG_FLOATING_MAX = 2.7f;
const float CHG_FLOATING_MIN = 0.5f;
const float BATT_VOLT_INFLUENCE = 0.05f;
const float BATT_PERC_INFLUENCE = 0.05f;

const size_t PT_READ_BUFF_SIZE = 150;
const uint16_t PT_TIMER_INTERVAL_US = 100;
const uint16_t PT_SAMPLING_INTERVAL_US = 500;
const uint16_t DROPLET_PERIOD_MAX_MS = 50000;

const uint16_t BOOT_LOGO_DURATION_MS = 7000;
const uint16_t WAKEUP_LOGO_DURATION_MS = 4000;
const uint16_t SLEEP_LOGO_DURATION_MS = 3000;
const uint8_t DISPLAY_I2C_ADDR = 0x3C;
const uint16_t DISPLAY_REF_PERIOD_MS = 1000;

const uint32_t STORAGE_MAGIC_BYTES_VALAUE = 0xA55B00B5;

#endif