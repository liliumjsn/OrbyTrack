#include <Arduino.h>
#include "droplet_detector.h"
#include "ui_common.h"
#include "power_manager.h"
#include "pinout.h"
#include "settings.h"

void setup()
{
	Serial.begin(115200);
    delay(5000); // Making sure it doesn't get stuck in sleep without USB
    Serial.print("OrbyTrack ");
    Serial.print(ORBYTRACK_APP_VERSION);
    Serial.println(" by LiliumJSN");
    Settings::init();
    PowerManager::init();
    DropletDetector::init();
    DropletDetector::set_detection_cb([](DropletDetector::DetectedDroplet *latest_detected_droplet){
        PowerManager::reset_sleep_timer();
        Serial.println("DP");
        UiStateMain::droplet_detected_handler();
    });
    UiCommon::init();
}

void loop()
{
	PowerManager::handle();
    UiCommon::handle();
    PowerManager::SleepReason sleep_reason = PowerManager::is_time_to_sleep();
    switch(sleep_reason)
    {
        case PowerManager::SleepReason::NONE:
            break;
        case PowerManager::SleepReason::LOW_BAT:
            UiCommon::shutdown("LOW BATTERY");
            break;
       case PowerManager::SleepReason::INACTIVITY:
            UiCommon::shutdown("INACTIVITY");
            break;
    }
    DropletDetector::handle();
}
