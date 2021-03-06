#ifndef __PREPARATORY_WORK_HPP__
#define __PREPARATORY_WORK_HPP__

// DEBUG
#define DEBUG_MODEL
// RELEASE
//#undef DEBUG_MODEL

// include
#include <ArduinoBLE.h>

#include "lighting_utils.hpp"
#include "delay_switching.hpp"
#include "sensor_acquisition.hpp"

// known

// Characteristic User Description
#define DESCRIPTION_NAMED_UUID "2901"
#define SERVICE_NAMED_UUID "1800"

// service
#define TEMPERATURE_SERVICE_UUID "fe2c0000-0730-4a71-b132-4917c2bb832d"
#define HUMIDITY_SERVICE_UUID "b1490000-5b47-44d0-88fd-5397b5511263"
#define PRESSURE_SERVICE_UUID "41ec0000-6818-4108-80e8-82bd95504b7e"
#define SAMPLE_SERVICE_UUID "855f0000-9f0f-49b7-88f7-d7f66145f461"
#define LIGHT_INTENSITY_SERVICE_UUID "b4c20000-8684-4ba3-b784-e0b4a499a042"
#define CONTROL_SERVICE_UUID "6c880000-6ca3-4775-9b56-c6ac4d0c1f72"
#define SENSOR_THRESHOLD_SERVICE_UUID "75d40000-e036-4297-bea3-d9ea16d570e4"

// Characteristic
#define TEMPERATURE_CHARACTERISTIC_UUID "fe2c0001-0730-4a71-b132-4917c2bb832d"
#define HUMIDITY_CHARACTERISTIC_UUID "b1490001-5b47-44d0-88fd-5397b5511263"
#define PRESSURE_CHARACTERISTIC_UUID "41ec0001-6818-4108-80e8-82bd95504b7e"
#define SAMPLE_CHARACTERISTIC_UUID "855f0001-9f0f-49b7-88f7-d7f66145f461"
#define LIGHT_INTENSITY_CHARACTERISTIC_UUID "b4c20001-8684-4ba3-b784-e0b4a499a042"
#define CONTROL_NOTICE_CHARACTERISTIC_UUID "6c880001-6ca3-4775-9b56-c6ac4d0c1f72"
#define SENSOR_THRESHOLD_CHARACTERISTIC_UUID "75d40001-e036-4297-bea3-d9ea16d570e4"

// Descriptor
#define TEMPERATURE_DESCRIPTOR_UUID "fe2c0002-0730-4a71-b132-4917c2bb832d"
#define HUMIDITY_DESCRIPTOR_UUID "b1490002-5b47-44d0-88fd-5397b5511263"
#define PRESSURE_DESCRIPTOR_UUID "41ec0002-6818-4108-80e8-82bd95504b7e"
#define SAMPLE_DESCRIPTOR_UUID "855f0002-9f0f-49b7-88f7-d7f66145f461"
#define LIGHT_INTENSITY_DESCRIPTOR_UUID "b4c20002-8684-4ba3-b784-e0b4a499a042"
#define CONTROL_DESCRIPTOR_UUID "6c880002-6ca3-4775-9b56-c6ac4d0c1f72"
#define SENSOR_THRESHOLD_DESCRIPTOR_UUID "75d40002-e036-4297-bea3-d9ea16d570e4"

BLEService tempService(TEMPERATURE_SERVICE_UUID);
BLEService humidService(HUMIDITY_SERVICE_UUID);
BLEService pressService(PRESSURE_SERVICE_UUID);
BLEService sampService(SAMPLE_SERVICE_UUID);
BLEService lightService(LIGHT_INTENSITY_SERVICE_UUID);
BLEService controlService(CONTROL_SERVICE_UUID);
BLEService thresholdService(SENSOR_THRESHOLD_SERVICE_UUID);

BLEFloatCharacteristic tempCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEFloatCharacteristic humidCharacteristic(HUMIDITY_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEFloatCharacteristic pressCharacteristic(PRESSURE_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEByteCharacteristic sampCharacteristic(SAMPLE_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEByteCharacteristic lightIntensityCharacteristic(LIGHT_INTENSITY_CHARACTERISTIC_UUID, BLERead | BLENotify);
BLEByteCharacteristic controlNoticeCharacteristic(CONTROL_NOTICE_CHARACTERISTIC_UUID, BLERead | BLENotify | BLEWrite);
BLECharacteristic sensorThresholdCharacteristic(SENSOR_THRESHOLD_CHARACTERISTIC_UUID, BLERead | BLENotify | BLEWrite, 14, true);

BLEDescriptor infoDescriptor(DESCRIPTION_NAMED_UUID, "Arduino Nano 33 BLE SENSE");
BLEDescriptor tempDescriptor(DESCRIPTION_NAMED_UUID, "temperature message");
BLEDescriptor humidDescriptor(DESCRIPTION_NAMED_UUID, "humidity message");
BLEDescriptor pressDescriptor(DESCRIPTION_NAMED_UUID, "pressure message");
BLEDescriptor sampDescriptor(DESCRIPTION_NAMED_UUID, "sample message");
BLEDescriptor lightIntensityDescriptor(DESCRIPTION_NAMED_UUID, "light intensity");
BLEDescriptor controlNoticeDescriptor(DESCRIPTION_NAMED_UUID, "control notice");
BLEDescriptor sensorThresholdDescriptor(DESCRIPTION_NAMED_UUID, "sensor threshold");

// handle function
void blePeripheralConnectHandler(BLEDevice central);
void blePeripheralDisconnectHandler(BLEDevice central);

// struct
// downThreshold, upThreshold, downThresholdSentry, upThresholdSentry
struct SensorThreshold 
{
    float* gTT; // 4
    float* gHT; // 4
    float* gPT; // 4
    int* gST;   // 2
};

struct ControlNotice 
{
    bool light_flip;
    bool light_flip_control; // true: manual, false: automatic
    bool smart_reminder_flip;
    SensorWhich warning;
    SensorThreshold* sensor_threshold;
};

struct MessageBuffer 
{
    bool shake;
    float temperature;
    float humidity;
    float pressure;
    byte light_intensity;
    byte _gesture;
    byte sample;
    byte proximity;
    bool changed_control_notice;
    ControlNotice* buffer_control_notice;

    bool light_flip_load();
    void light_flip_store(bool val);
    bool light_flip_control_load();
    void light_flip_control_store(bool val);
    bool smart_reminder_flip_load();
    void smart_reminder_flip_store(bool val);
    SensorWhich warning_load();
    void warning_store(SensorWhich sWhich);

    void control_notice_reset();
};

bool MessageBuffer::light_flip_load() {
    return buffer_control_notice->light_flip;
}

void MessageBuffer::light_flip_store(bool val) {
    changed_control_notice = true;
    buffer_control_notice->light_flip = val;
}

bool MessageBuffer::light_flip_control_load() {
    return buffer_control_notice->light_flip_control;
}

void MessageBuffer::light_flip_control_store(bool val) {
    changed_control_notice = true;
    buffer_control_notice->light_flip_control = val;
}

bool MessageBuffer::smart_reminder_flip_load() {
    return buffer_control_notice->smart_reminder_flip;
}

void MessageBuffer::smart_reminder_flip_store(bool val) {
    changed_control_notice = true;
    buffer_control_notice->smart_reminder_flip = val;
}

SensorWhich MessageBuffer::warning_load() {
    return buffer_control_notice->warning;
}

void MessageBuffer::warning_store(SensorWhich sWhich) {
    changed_control_notice = true;
    buffer_control_notice->warning = sWhich;
}

void MessageBuffer::control_notice_reset() { 
    changed_control_notice = false; 
}

// struct pointer
LightingUtils* lU = NULL;
SensorThreshold* sThreshold = NULL;
ControlNotice* cNotice = NULL;
MessageBuffer* mBuffer = NULL;

// initializer
static void messageBufferInit() {
    sThreshold = (struct SensorThreshold*)malloc(sizeof(struct SensorThreshold));
    cNotice = (struct ControlNotice*)malloc(sizeof(struct ControlNotice));
    mBuffer = (struct MessageBuffer*)malloc(sizeof(struct MessageBuffer));

    sThreshold->gTT = NULL;
    sThreshold->gHT = NULL;
    sThreshold->gPT = NULL;
    sThreshold->gST = NULL;

    cNotice->light_flip = false;
    cNotice->light_flip_control = false;
    cNotice->smart_reminder_flip = true;
    cNotice->warning = SensorWhich::NOBODY;
    cNotice->sensor_threshold = sThreshold;

    mBuffer->shake = false;
    mBuffer->temperature = 0.0f;
    mBuffer->humidity = 0.0f;
    mBuffer->pressure = 0.0f;
    mBuffer->light_intensity = 0;
    mBuffer->_gesture = -1;
    mBuffer->sample = 0;
    mBuffer->proximity = 0;
    mBuffer->changed_control_notice = -1;
    mBuffer->buffer_control_notice = cNotice;
}

void blePeripheralConnectHandler(BLEDevice central);
void blePeripheralDisconnectHandler(BLEDevice central);
void recvNoticeHandler(BLEDevice central, BLECharacteristic characteristic);
void recvThresholdHandler(BLEDevice central, BLECharacteristic characteristic);

void ble_services_init() {
    if (!BLE.begin()) {
#ifdef DEBUG_MODEL
        Serial.println("starting Bluetooth?? Low Energy module failed!");
#endif
        while (1)
            ;
    } else {
        BLE.setLocalName("Arduino_Nano_33_BLE_SENSE");
        BLE.setAppearance(0x5696);
        BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
        BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

        tempService.addCharacteristic(tempCharacteristic);
        humidService.addCharacteristic(humidCharacteristic);
        pressService.addCharacteristic(pressCharacteristic);
        sampService.addCharacteristic(sampCharacteristic);
        lightService.addCharacteristic(lightIntensityCharacteristic);
        controlService.addCharacteristic(controlNoticeCharacteristic);
        thresholdService.addCharacteristic(sensorThresholdCharacteristic);

        tempCharacteristic.addDescriptor(tempDescriptor);
        humidCharacteristic.addDescriptor(humidDescriptor);
        pressCharacteristic.addDescriptor(pressDescriptor);
        sampCharacteristic.addDescriptor(sampDescriptor);
        lightIntensityCharacteristic.addDescriptor(lightIntensityDescriptor);
        controlNoticeCharacteristic.addDescriptor(controlNoticeDescriptor);
        sensorThresholdCharacteristic.addDescriptor(sensorThresholdDescriptor);

        BLE.setAdvertisedServiceUuid(SERVICE_NAMED_UUID);
        BLE.addService(tempService);
        BLE.addService(humidService);
        BLE.addService(pressService);
        BLE.addService(sampService);
        BLE.addService(lightService);
        BLE.addService(controlService);
        BLE.addService(thresholdService);
        BLE.advertise();
    }

    LightingUtils::LightingUtilsInit();
    SensorAcquisition::SensorAcquisitionInit();
    messageBufferInit();

    lU = LightingUtils::LightingUtilsInstance();

    // BLESubscribed, BLEUnsubscribed, BLERead, BLEWritten
    controlNoticeCharacteristic.setEventHandler(BLEWritten, recvNoticeHandler);
    sensorThresholdCharacteristic.setEventHandler(BLEWritten, recvThresholdHandler);
}

#endif // __PREPARATORY_WORK_HPP__
