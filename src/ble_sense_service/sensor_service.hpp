#ifndef __SENSOR_SERVICE_HPP__
#define __SENSOR_SERVICE_HPP__

#include "preparatory_work.hpp"

// timer
static TimerDelay _sensorDelay(500);
static TimerDelay _serviceDelay(1000);
static TimerDelay _connTimeoutDelay(10000);

static bool _connectFlag = false;
static bool _isConnected = false;
static bool _sensorOnceFlag = false;
static bool _activeDisconnection = false;
static int _sensorEnable = true;
static int _proximityOldNum = -1;
static int _proximityResetOldNum = -1;

static void _controlNDisplay() {
#ifdef DEBUG_MODEL
    Serial.println("Temperature = " + String(mBuffer->temperature) + " °C");
    Serial.println("Humidity = " + String(mBuffer->humidity) + " %");
    Serial.println("Pressure = " + String(mBuffer->pressure) + " kPa");
    Serial.println("Sample: " + String(mBuffer->sample) + " dB");
    Serial.println("Proximity on: " + String(mBuffer->proximity));
#endif

    if (mBuffer->proximity >= 0 && mBuffer->proximity < 10) {
        if (_proximityResetOldNum != -1) {
            _proximityResetOldNum = -1;
            if (mBuffer->light_flip_control_load())
                mBuffer->light_flip_control_store(false);
#ifdef DEBUG_MODEL
            Serial.println("Automatic Reset");
#endif
        } else {
            _proximityResetOldNum = mBuffer->proximity;
        }
    } else if (mBuffer->proximity >= 10 && mBuffer->proximity < 180) {
        if (_proximityOldNum != -1) {
            _proximityOldNum = -1;

            if (!mBuffer->light_flip_control_load())
                mBuffer->light_flip_control_store(true);

            if (mBuffer->light_flip_load()) {
                mBuffer->light_flip_store(false);
#ifdef DEBUG_MODEL
                Serial.println("Manually turn off bulb");
#endif
            } else {
                mBuffer->light_flip_store(true);
#ifdef DEBUG_MODEL
                Serial.println("Manually turn on bulb");
#endif
            }
        } else {
            _proximityOldNum = mBuffer->proximity;
        }
    }

    if (mBuffer->light_intensity != -1) {
#ifdef DEBUG_MODEL
        Serial.println("a is " + String(mBuffer->light_intensity) + ", is "
            + (mBuffer->light_intensity > 15 ? "Day" : "Night"));
#endif
        if (!mBuffer->light_flip_control_load()) {
            if (mBuffer->light_intensity > 20) {
                mBuffer->light_flip_store(false);
#ifdef DEBUG_MODEL
                Serial.println("Automatic bulb off");
#endif
            } else {
                mBuffer->light_flip_store(true);
#ifdef DEBUG_MODEL
                Serial.println("Automatic bulb on");
#endif
            }
        }
    }
    if (mBuffer->shake) {
#ifdef DEBUG_MODEL
        Serial.println("shake it!!!");
#endif
        if (_isConnected && !_activeDisconnection) {
            _activeDisconnection = true;
        } else if (!_connectFlag) {
            // connecting: entry BLE loop and set the led
            _connectFlag = true;
            // connecting: sensor suspended
            // _sensorEnable = false;
            _connTimeoutDelay.reset();
        }
    }

#ifdef DEBUG_MODEL
    Serial.println("gTT: (" + String(sThreshold->gTT[0]) + ", "
        + String(sThreshold->gTT[1]) + ", " + String(sThreshold->gTT[2]) + ", "
        + String(sThreshold->gTT[3]) + ")" + ",\ngHT: ("
        + String(sThreshold->gHT[0]) + ", " + String(sThreshold->gHT[1]) + ", "
        + String(sThreshold->gHT[2]) + ", " + String(sThreshold->gHT[3]) + ")"
        + ",\ngPT: (" + String(sThreshold->gPT[0]) + ", "
        + String(sThreshold->gPT[1]) + ", " + String(sThreshold->gPT[2]) + ", "
        + String(sThreshold->gPT[3]) + ")" + ",\ngST: ("
        + String(sThreshold->gST[0]) + ", " + String(sThreshold->gST[1]) + ")");

    switch (SensorAcquisition::sensorRiskArbitration()) {
        case SensorWhich::TEMP:
            Serial.println("Temperature warning!!!");
            break;
        case SensorWhich::HUMID:
            Serial.println("Humidity warning!!!");
            break;
        case SensorWhich::PRESS:
            Serial.println("Pressure warning!!!");
            break;
        case SensorWhich::SAMP:
            Serial.println("Sample warning!!!");
            break;
        default:
            Serial.println("NOBODY warning!!!");
            break;
    }
    Serial.println("connectFlag: " + String(_connectFlag) + ", isConnected: " + String(_isConnected));
    Serial.println("pin light: " + String(lU->getPinState()) + ", pow light: " + String(lU->getPowState()));
    Serial.println("-----------------------------------------------------------");
#endif
}

static void _colorReminderAction() {
    mBuffer->warning_store(SensorAcquisition::sensorRiskArbitration());
    switch (mBuffer->warning_load()) {
        case SensorWhich::TEMP:
            lU->startColorBreath(Color::RED);
            break;
        case SensorWhich::HUMID:
            lU->startColorBreath(Color::CYAN);
            break;
        case SensorWhich::PRESS:
            lU->startColorBreath(Color::GREEN);
            break;
        case SensorWhich::SAMP:
            lU->startColorBreath(Color::ORANGE);
            break;
        default:
            lU->startColorBreath(Color::DARK);
            break;
    }
}

void colorGradient() {
    // RGB
    if (_connectFlag) // try to connect
        lU->startColorBreath(Color::BLUE);
    else if (mBuffer->smart_reminder_flip_load())
        _colorReminderAction();
    else
        lU->startColorGradient();
    // POW_LED
    if (mBuffer->light_flip_load())
        lU->setPowOn();
    else
        lU->setPowOff();
    // PIN_LED
    if (_connectFlag)
        lU->startPinBreath(); // should stopPinBreath()
    else if (_isConnected)
        lU->setPinOn();
    else
        lU->setPinOff();
}

void sensorInfo() {
    if (!_sensorEnable)
        return;
    if (!_sensorDelay.workable())
        return;

    // message get
    // SensorThreshold
    sThreshold->gTT = (float*)SensorAcquisition::getTemperatureThreshold();
    sThreshold->gHT = (float*)SensorAcquisition::getHumidityThreshold();
    sThreshold->gPT = (float*)SensorAcquisition::getPressureThreshold();
    sThreshold->gST = (int*)SensorAcquisition::getSampleThreshold();
    // MessageBuffer
    mBuffer->shake = SensorAcquisition::getShake();
    mBuffer->temperature = SensorAcquisition::getTemperature(CELSIUS);
    mBuffer->humidity = SensorAcquisition::getHumidity();
    mBuffer->pressure = SensorAcquisition::getPressure(KILOPASCAL);
    mBuffer->light_intensity = SensorAcquisition::getLightIntensity();
    mBuffer->sample = SensorAcquisition::getSample();
    mBuffer->proximity = SensorAcquisition::getProximity();

    // message display
    _controlNDisplay();
}

void blePeripheralConnectHandler(BLEDevice central) {
#ifdef DEBUG_MODEL
    Serial.print("Connected event, central: ");
    Serial.println(central.address());
#endif

    // transition
    _isConnected = true;
    _connectFlag = false;

    _sensorOnceFlag = false;

    cNotice->smart_reminder_flip = true;
    // sensor rework
    // _sensorEnable = true;
}

void blePeripheralDisconnectHandler(BLEDevice central) {
#ifdef DEBUG_MODEL
    Serial.print("Disconnected event, central: ");
    Serial.println(central.address());
#endif

    _isConnected = false;
}

void recvNoticeHandler(BLEDevice central, BLECharacteristic characteristic) {
    byte _controlByte;
#ifdef DEBUG_MODEL
    Serial.println("Recv notice...");
#endif
    if (controlNoticeCharacteristic.value()) {
        controlNoticeCharacteristic.readValue(_controlByte);
        cNotice->light_flip_control = (_controlByte & 0x02);
        if (_controlByte & 0x02)
            cNotice->light_flip = (_controlByte & 0x01) ? true : false;

        mBuffer->smart_reminder_flip_store(_controlByte & 0x04);
    } else {
#ifdef DEBUG_MODEL
        Serial.println("Recv notice error...");
#endif
    }
}

static byte _sensorThresholdBuffer[14];

void recvThresholdHandler(BLEDevice central, BLECharacteristic characteristic) {
#ifdef DEBUG_MODEL
    Serial.println("Recv sensor threshold...");
#endif
    if (sensorThresholdCharacteristic.value()) {
        sensorThresholdCharacteristic.readValue(_sensorThresholdBuffer, 14);
        sThreshold->gTT[0] = _sensorThresholdBuffer[0];
        sThreshold->gTT[1] = _sensorThresholdBuffer[1];
        sThreshold->gTT[2] = _sensorThresholdBuffer[2];
        sThreshold->gTT[3] = _sensorThresholdBuffer[3];
        sThreshold->gHT[0] = _sensorThresholdBuffer[4];
        sThreshold->gHT[1] = _sensorThresholdBuffer[5];
        sThreshold->gHT[2] = _sensorThresholdBuffer[6];
        sThreshold->gHT[3] = _sensorThresholdBuffer[7];
        sThreshold->gPT[0] = _sensorThresholdBuffer[8];
        sThreshold->gPT[1] = _sensorThresholdBuffer[9];
        sThreshold->gPT[2] = _sensorThresholdBuffer[10];
        sThreshold->gPT[3] = _sensorThresholdBuffer[11];
        sThreshold->gST[0] = _sensorThresholdBuffer[12];
        sThreshold->gST[1] = _sensorThresholdBuffer[13];
    } else {
#ifdef DEBUG_MODEL
        Serial.println("Recv sensor threshold error...");
#endif
    }
}

static void _sendToBLE() {
    byte _controlByte = 0;

    if (tempCharacteristic.subscribed())
        tempCharacteristic.writeValue(mBuffer->temperature);
    if (humidCharacteristic.subscribed())
        humidCharacteristic.writeValue(mBuffer->humidity);
    if (pressCharacteristic.subscribed())
        pressCharacteristic.writeValue(mBuffer->pressure);
    if (sampCharacteristic.subscribed())
        sampCharacteristic.writeValue(mBuffer->sample);
    if (lightIntensityCharacteristic.subscribed())
        lightIntensityCharacteristic.writeValue(mBuffer->light_intensity);

    if (mBuffer->changed_control_notice) {
        if (mBuffer->light_flip_load())
            _controlByte |= 0x01;
        if (mBuffer->light_flip_control_load())
            _controlByte |= 0x02;
        if (mBuffer->smart_reminder_flip_load())
            _controlByte |= 0x04;
        if (controlNoticeCharacteristic.subscribed())
            controlNoticeCharacteristic.writeValue(_controlByte);
        mBuffer->control_notice_reset();
    }
}


static void _sendSensorThresholdOnce() {
    if (!_sensorOnceFlag && sensorThresholdCharacteristic.subscribed()) {
        _sensorThresholdBuffer[0] = sThreshold->gTT[0] ;
        _sensorThresholdBuffer[1] = sThreshold->gTT[1] ;
        _sensorThresholdBuffer[2] = sThreshold->gTT[2] ;
        _sensorThresholdBuffer[3] = sThreshold->gTT[3] ;
        _sensorThresholdBuffer[4] = sThreshold->gHT[0] ;
        _sensorThresholdBuffer[5] = sThreshold->gHT[1] ;
        _sensorThresholdBuffer[6] = sThreshold->gHT[2] ;
        _sensorThresholdBuffer[7] = sThreshold->gHT[3] ;
        _sensorThresholdBuffer[8] = sThreshold->gPT[0] ;
        _sensorThresholdBuffer[9] = sThreshold->gPT[1] ;
        _sensorThresholdBuffer[10] = sThreshold->gPT[2];
        _sensorThresholdBuffer[11] = sThreshold->gPT[3];
        _sensorThresholdBuffer[12] = sThreshold->gST[0];
        _sensorThresholdBuffer[13] = sThreshold->gST[1];
        sensorThresholdCharacteristic.writeValue(_sensorThresholdBuffer, 14);
        _sensorOnceFlag = true;
    }
}

void demo_services_start_work() {
    if (_connectFlag || _isConnected) {
        BLEDevice central = BLE.central();
        if (central && central.connected()) {
            if (_serviceDelay.workable()) {
                _sendSensorThresholdOnce();
                _sendToBLE();
            }
            // FIXME
            // if (_activeDisconnection) {
            //     central.disconnect();
            //     central.connect();
            // }
        } else if (_connTimeoutDelay.workable()) { // active disconnection
            _connectFlag = false;
            _isConnected = false;
            _sensorOnceFlag = false;
        }
    }
}

#endif // __SENSOR_SERVICE_HPP__
