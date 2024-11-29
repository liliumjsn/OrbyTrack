#include <Arduino.h>
#include "droplet_detector.h"
#include "ui.h"
#include "power_manager.h"
#include "pinout.h"
#include "settings.h"

void setup()
{
	Serial.begin(115200);
    delay(5000); // Making sure it doesn't get stuck in sleep without USB
    Serial.print(ORBYTRACK_APP_VERSION);
    Serial.println(" by LiliumJSN");
    Settings::init();
    PowerManager::init();
    DropletDetector::init();
    DropletDetector::set_detection_cb([](DropletDetector::DetectedDroplet *latest_detected_droplet){
        PowerManager::reset_sleep_timer();
        Serial.println("DP");
    });
    Ui::init();
}

void loop()
{
	PowerManager::handle();
    Ui::handle();
    if(PowerManager::is_time_to_sleep())
    {
        Ui::change_state(Ui::UiStateSleep::get_data());
    }
    DropletDetector::handle();
}
