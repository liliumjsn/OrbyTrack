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
    // DropletDetector::init();
    Ui::init();
}

void loop()
{
	PowerManager::handle();
    Ui::handle();
}
