#ifndef __SENSOR_ACQUISITION_HPP__
#define __SENSOR_ACQUISITION_HPP__

#include <PDM.h>
#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
#include <Arduino_LSM9DS1.h>
#include <Arduino_APDS9960.h>

#include "delay_switching.hpp"

using float_pair = float[4];
using int_pair = int[2];

enum SensorState 
{ 
    SAFE = 0, 
    LOWER,
    HIGHER 
};

enum SensorWhich 
{ 
    NOBODY = 0, 
    TEMP, 
    HUMID, 
    PRESS, 
    SAMP 
};

class SensorAcquisition 
{
private:
    union SensorValue_ 
    {
        float _no;
        float _temp;
        float _humid;
        float _press;
        int _samp;
    };

    struct SensorSave_ 
    {
        SensorWhich _which;
        SensorValue_* _value;
    };
    //  static SensorAcquisition* __sensorInstance;
    static short __sampleBuffer[512];
    static volatile int __samplesRead;
    // downThreshold, upThreshold, downThresholdSentry, upThresholdSentry
    static float __temperatureThreshold[4];
    static float __humidityThreshold[4];
    static float __pressureThreshold[4];
    static int __sampleThreshold[2];
    static TimerDelay __RiskNoticeHold;
    static SensorValue_ __sV;
    static SensorSave_ __sS;

public:
    static void SensorAcquisitionInit();
    //  static SensorAcquisition* SensorAcquisitionInstance();

    // get value
    static float getTemperature(int unit);
    static float getHumidity();
    static float getPressure(int unit);
    static int getLightIntensity();
    static int getGesture();
    static int getSample();
    static int getShake();
    static int getProximity();

    // get risk state
    static SensorState getTemperatureState(float temperature);
    static SensorState getHumidityState(float humidity);
    static SensorState getPressureState(float pressure);
    static SensorState getSampleState(int sample);

    static SensorWhich sensorRiskArbitration();

    static decltype(__temperatureThreshold)* getTemperatureThreshold();
    static decltype(__humidityThreshold)* getHumidityThreshold();
    static decltype(__pressureThreshold)* getPressureThreshold();
    static decltype(__sampleThreshold)* getSampleThreshold();

    // reminder
    static void setTemperatureThreshold(
        float dt, float ut, float dts, float uts);
    static void setHumidityThreshold(float dt, float ut, float dts, float uts);
    static void setPressureThreshold(float dt, float ut, float dts, float uts);
    static void setSampleThreshold(int dt, int ut);

private:
    SensorAcquisition() = delete;
    static void __onPDMdata();
};

// SensorAcquisition* SensorAcquisition::__sensorInstance = NULL;
short SensorAcquisition::__sampleBuffer[512] = { 0 };
volatile int SensorAcquisition::__samplesRead = 0;
// float SensorAcquisition::__temperatureThreshold[4] =
// {15.0, 37.0, 18.0, 34.0}; float SensorAcquisition::__humidityThreshold[4] =
// {20.0, 50.0, 23.0, 47.0}; float SensorAcquisition::__pressureThreshold[4] =
// {80.0, 110.0, 83.0, 107.0}; int SensorAcquisition::__sampleThreshold[2] =
// {80, 75};
float SensorAcquisition::__temperatureThreshold[4] = { 15.0, 36.0, 18.0, 34.0 };
float SensorAcquisition::__humidityThreshold[4] = { 20.0, 60.0, 23.0, 59.0 };
float SensorAcquisition::__pressureThreshold[4] = { 80.0, 100.0, 83.0, 99.5 };
int SensorAcquisition::__sampleThreshold[2] = { 65, 60 };
TimerDelay SensorAcquisition::__RiskNoticeHold(5000);
SensorAcquisition::SensorValue_ SensorAcquisition::__sV { _no : 0.0f };
SensorAcquisition::SensorSave_ SensorAcquisition::__sS { _which : SensorWhich::NOBODY, _value : &__sV };

// FIXME
SensorState SensorAcquisition::getTemperatureState(float temperature) {
    if (temperature < __temperatureThreshold[0])
        return SensorState::LOWER;
    else if (temperature > __temperatureThreshold[1])
        return SensorState::HIGHER;
    else
        return SensorState::SAFE;
}

// FIXME
SensorState SensorAcquisition::getHumidityState(float humidity) {
    if (humidity < __humidityThreshold[0])
        return SensorState::LOWER;
    else if (humidity > __humidityThreshold[1])
        return SensorState::HIGHER;
    else
        return SensorState::SAFE;
}

// FIXME
SensorState SensorAcquisition::getPressureState(float pressure) {
    if (pressure < __pressureThreshold[0])
        return SensorState::LOWER;
    else if (pressure > __pressureThreshold[1])
        return SensorState::HIGHER;
    else
        return SensorState::SAFE;
}

// FIXME
SensorState SensorAcquisition::getSampleState(int sample) {
    if (sample > __sampleThreshold[0])
        return SensorState::HIGHER;
    else
        return SensorState::SAFE;
}

SensorWhich SensorAcquisition::sensorRiskArbitration() {
    if (!__RiskNoticeHold.workable())
        return __sS._which;
    float curTemperature = getTemperature(CELSIUS);
    float curHumidity = getHumidity();
    float curPressure = getPressure(KILOPASCAL);
    int curSample = getSample();
    float deviation = -1.0f;
    float cDev = -1.0f;
    SensorWhich which = SensorWhich::NOBODY;

    if (getTemperatureState(curTemperature) != SensorState::SAFE) {
        deviation = (getTemperatureState(curTemperature) == SensorState::HIGHER
                            ? (curTemperature - __temperatureThreshold[1])
                            : (__temperatureThreshold[0] - curTemperature))
            / (__temperatureThreshold[1] - __temperatureThreshold[0]);

        which = SensorWhich::TEMP;
    }
    if (getHumidityState(curHumidity) != SensorState::SAFE) {
        cDev = (getHumidityState(curHumidity) == SensorState::HIGHER
                       ? (curHumidity - __humidityThreshold[1])
                       : (__humidityThreshold[0] - curHumidity))
            / (__humidityThreshold[1] - __humidityThreshold[0]);

        if (cDev > deviation) {
            deviation = cDev;
            which = SensorWhich::HUMID;
        }
    }
    if (getPressureState(curPressure) != SensorState::SAFE) {
        cDev = (getPressureState(curPressure) == SensorState::HIGHER
                       ? (curPressure - __pressureThreshold[1])
                       : (__pressureThreshold[0] - curPressure))
            / (__pressureThreshold[1] - __pressureThreshold[0]);

        if (cDev > deviation) {
            deviation = cDev;
            which = SensorWhich::PRESS;
        }
    }
    if (getSampleState(curSample) != SensorState::SAFE) {
        which = SensorWhich::SAMP;
    }

    if (which != SensorWhich::NOBODY) {
        __sS._which = which;
    } else {
        switch (__sS._which) {
        case SensorWhich::TEMP:
            __sS._which = (curTemperature < __temperatureThreshold[2]
                              || curTemperature > __temperatureThreshold[3])
                ? __sS._which
                : SensorWhich::NOBODY;
            break;
        case SensorWhich::HUMID:
            __sS._which = (curHumidity < __humidityThreshold[2]
                              || curHumidity > __humidityThreshold[3])
                ? __sS._which
                : SensorWhich::NOBODY;
            break;
        case SensorWhich::PRESS:
            __sS._which = (curPressure < __pressureThreshold[2]
                              || curPressure > __pressureThreshold[3])
                ? __sS._which
                : SensorWhich::NOBODY;
            break;
        case SensorWhich::SAMP:
            __sS._which = (curSample > __sampleThreshold[1])
                ? __sS._which
                : SensorWhich::NOBODY;
            break;
        default:
            __sS._which = SensorWhich::NOBODY;
            break;
        }
    }
    return __sS._which;
}

float_pair* SensorAcquisition::getTemperatureThreshold() {
    return &__temperatureThreshold;
}

float_pair* SensorAcquisition::getHumidityThreshold() {
    return &__humidityThreshold;
}

float_pair* SensorAcquisition::getPressureThreshold() {
    return &__pressureThreshold;
}

int_pair* SensorAcquisition::getSampleThreshold() { 
    return &__sampleThreshold; 
}

void SensorAcquisition::setTemperatureThreshold(float dt, float ut, float dts, float uts) {
    // TODO: assert
    __temperatureThreshold[0] = dt;
    __temperatureThreshold[1] = ut;
    __temperatureThreshold[2] = dts;
    __temperatureThreshold[3] = uts;
}

void SensorAcquisition::setHumidityThreshold(float dt, float ut, float dts, float uts) {
    // TODO: assert
    __humidityThreshold[0] = dt;
    __humidityThreshold[1] = ut;
    __humidityThreshold[2] = dts;
    __humidityThreshold[3] = uts;
}

void SensorAcquisition::setPressureThreshold(float dt, float ut, float dts, float uts) {
    // TODO: assert
    __pressureThreshold[0] = dt;
    __pressureThreshold[1] = ut;
    __pressureThreshold[2] = dts;
    __pressureThreshold[3] = uts;
}

void SensorAcquisition::setSampleThreshold(int dt, int ut) {
    // TODO: assert
    __sampleThreshold[0] = dt;
    __sampleThreshold[1] = ut;
}

//声音采集，使用单通道数据
//计算分贝：选取值最大的一个作为当前的环境噪声，对该最大值乘5，再取以10为底的对数，乘24即为当前的环境噪声（分贝）。
void SensorAcquisition::__onPDMdata() {
    int bytesAvailable = PDM.available();     //查询可用字节数；
    PDM.read(__sampleBuffer, bytesAvailable); //存入缓冲区；
    __samplesRead = bytesAvailable / 2;
}

void SensorAcquisition::SensorAcquisitionInit() {
    // 环境温度湿度
    if (!HTS.begin()) {
        Serial.println("Failed to initialize humidity temperature sensor!");
        while (1)
            ;
    }

    // 大气压强
    if (!BARO.begin()) {
        Serial.println("Failed to initialize pressure sensor!");
        while (1)
            ;
    }

    // 日照强度
    if (!APDS.begin()) {
        Serial.println("Error initializing APDS9960 sensor!");
    }
    APDS.setGestureSensitivity(100);
    //  APDS.setLEDBoost(3);

    // 环境噪声
    PDM.onReceive(__onPDMdata); //初始化麦克风MP34DT05；
    PDM.begin(1, 16000);

    // 摇一摇
    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while (1)
            ;
    }
}

// SensorAcquisition* SensorAcquisition::SensorAcquisitionInstance() {
//   if (__sensorInstance == NULL)
//     __sensorInstance = new SensorAcquisition();
//   return __sensorInstance;
// }

float SensorAcquisition::getTemperature(int unit) {
    // CELSIUS | FAHRENHEIT
    return HTS.readTemperature(unit);
}

float SensorAcquisition::getHumidity() { 
    return HTS.readHumidity(); 
}

float SensorAcquisition::getPressure(int unit) {
    // PSI | MILLIBAR | KILOPASCAL
    return BARO.readPressure(unit);
}

int SensorAcquisition::getLightIntensity() {
    int count = 1;
    int _r = 0, _g = 0, _b = 0, a = 0;
    while (!APDS.colorAvailable() || count <= 3) {
        delay(5 * count++);
    }
    if (APDS.colorAvailable()) {
        APDS.readColor(_r, _g, _b, a);
        return a;
    }
    return -1;
}

int SensorAcquisition::getGesture() {
    if (!APDS.begin()) {
        Serial.println("Error initializing APDS9960 sensor!");
    }
    if (APDS.gestureAvailable()) {
        int gesture = APDS.readGesture();
        return gesture;
    }
    return -1;
}

int SensorAcquisition::getProximity() {
    int count = 1;
    if (APDS.proximityAvailable()) {
        int proximity = APDS.readProximity();
        return proximity;
    }
    return -1;
}

int SensorAcquisition::getSample() {
    while (!__samplesRead)
        ;
    uint32_t sound_max = 0;
    int i;
    for (i = 0; i < __samplesRead; i++) {
        if (__sampleBuffer[i] < 0)
            __sampleBuffer[i] = -__sampleBuffer[i];
        if (__sampleBuffer[i] > sound_max)
            sound_max = __sampleBuffer[i];
    }
    uint32_t sound = 24 * log10(sound_max * 5); //得出分贝；
    return sound;
}

int SensorAcquisition::getShake() {
    float x, y, z;
    if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(x, y, z);
        if (abs(x) > 2 || abs(y) > 2 || abs(z) > 2)
            return 1;
    }
    return 0;
}

#endif // __SENSOR_ACQUISITION_HPP__
