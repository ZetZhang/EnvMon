#include <Thread.h>
#include <ThreadController.h>
#include <StaticThreadController.h>

#include "sensor_service.hpp"

ThreadController mainLoopControl = ThreadController();

Thread bleThread = Thread();
Thread rgbThread = Thread();
Thread sensorThread = Thread();

void setup() {
    Serial.begin(9600);
    while (!Serial);

    ble_services_init();

    rgbThread.enabled = true;
    rgbThread.onRun(colorGradient);

    sensorThread.enabled = true;
    sensorThread.onRun(sensorInfo);

    bleThread.enabled = true;
    bleThread.onRun(services_start_work);

    mainLoopControl.add(&rgbThread);
    mainLoopControl.add(&bleThread);
    mainLoopControl.add(&sensorThread);
}

void loop() {
    mainLoopControl.run();
}
