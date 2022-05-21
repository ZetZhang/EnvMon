#ifndef __LIGHTING_UTILS_HPP__
#define __LIGHTING_UTILS_HPP__

#include "delay_switching.hpp"

#define LED_PIN 13
#define Rpin 22
#define Gpin 23
#define Bpin 24

enum Color 
{
    DARK = 0,  // 0,0,0
    WHITE = 1, // 255,255,255
    RED,       // 255,0,0
    ORANGE,    // 255,165,0
    YELLOW,    // 255,255,0
    GREEN,     // 0,255,0
    CYAN,      // 0,255,255
    BLUE,      // 0,0,255
    PURPLE,    // 128.0,128
    PINK,      // 255,192,203
};

struct PinLightStruct 
{
    int _pinBit;
    int _pinCounter;
};

struct PowLightStruct 
{
    int _powBit;
    int _powCounter;
};

struct ColorLightStruct 
{
    Color _color;
    int _colorCounter;
};

class LightingUtils 
{
private:
    static LightingUtils* __lightInstance;
    const int __color_0[3] = { 255, 255, 255 };
    const int __color_1[3] = { 255, 0, 0 }; // 1
    const int __color_2[3] = { 191, 63, 0 };
    const int __color_3[3] = { 127, 127, 0 };
    const int __color_4[3] = { 63, 191, 0 };
    const int __color_5[3] = { 0, 255, 0 }; // 2
    const int __color_6[3] = { 0, 191, 63 };
    const int __color_7[3] = { 0, 127, 128 };
    const int __color_8[3] = { 0, 63, 191 };
    const int __color_9[3] = { 0, 0, 255 }; // 3
    const int __color_10[3] = { 63, 0, 191 };
    const int __color_11[3] = { 127, 0, 127 };
    const int __color_12[3] = { 191, 0, 63 };

    ColorLightStruct __colorLS;
    PinLightStruct __pinLS;
    PowLightStruct __powLS;
    int __swtichNum;
    int __swtichNumNoDelay;
    bool __flipState;
    TimerDelay __rgbDelay;
    TimerDelay __pinDelay;
    TimerDelay __powDelay;
    TimerDelay __flipTimer;

public:
    LightingUtils();

    static void LightingUtilsInit();
    static LightingUtils* LightingUtilsInstance();

    // RGB
    void stopRGBColor();
    void setColor(int r, int g, int b);
    void setColor(Color color);
    void setColorWithDelay(Color color, int duration);
    void startColorGradient();
    void startColorGradientDelay(int duration);
    void startColorBreath(Color color);
    void startColorBreathDelay(Color color, int duration);
    Color getColor();

    // LED PIN
    void setPinOn();
    void setPinOff();
    void startPinBreath();
    void startPinBreathDelay(int duration);
     void stopPinBreath();
    bool getPinState();

    // LED POWER
    void setPowOn();
    void setPowOff();
    void startPowBreath();
    void startPowBreathDelay(int duration);
     void stopPowBreath();
    bool getPowState();

private:
    void __colorScheduling();
    void __colorSchedulingDelay(int duration);
    bool __colorGradientTo(const int* from, const int* to);
    void __colorGradientToDelay(const int* from, const int* to, int duration);
    void __colorReverse(int color[3]);
    void __colorChoose(Color color, int colorSetting[3]);
};

LightingUtils* LightingUtils::__lightInstance = NULL;

LightingUtils::LightingUtils()
    : __colorLS { _color : Color::DARK, _colorCounter : 0 }
    , __pinLS { _pinBit : false, _pinCounter : 0 }
    , __powLS { _powBit : false, _powCounter : 0 }
    , __swtichNum(0)
    , __swtichNumNoDelay(0)
    , __flipState(true)
    , __rgbDelay(3)
    , __pinDelay(3)
    , __powDelay(3)
    , __flipTimer(3)
{
}

void LightingUtils::stopRGBColor() { 
    setColor(Color::DARK); 
}

void LightingUtils::__colorChoose(Color color, int colorSetting[3]) {
    switch (color) {
        case DARK:
            colorSetting[0] = 0, colorSetting[1] = 0, colorSetting[2] = 0;
            break;
        case WHITE:
            colorSetting[0] = 255, colorSetting[1] = 255, colorSetting[2] = 255;
            break;
        case RED:
            colorSetting[0] = 255, colorSetting[1] = 0, colorSetting[2] = 0;
            break;
        case ORANGE:
            colorSetting[0] = 255, colorSetting[1] = 165, colorSetting[2] = 0;
            break;
        case YELLOW:
            colorSetting[0] = 255, colorSetting[1] = 255, colorSetting[2] = 0;
            break;
        case GREEN:
            colorSetting[0] = 0, colorSetting[1] = 255, colorSetting[2] = 0;
            break;
        case CYAN:
            colorSetting[0] = 0, colorSetting[1] = 255, colorSetting[2] = 255;
            break;
        case BLUE:
            colorSetting[0] = 0, colorSetting[1] = 0, colorSetting[2] = 255;
            break;
        case PURPLE:
            colorSetting[0] = 128, colorSetting[1] = 0, colorSetting[2] = 128;
            break;
        case PINK:
            colorSetting[0] = 255, colorSetting[1] = 192, colorSetting[2] = 203;
            break;
        default: // DARK
            colorSetting[0] = 0, colorSetting[1] = 0, colorSetting[2] = 0;
            break;
    }
    __colorLS._color = color;
    __colorReverse(colorSetting);
}

void LightingUtils::startColorBreath(Color color) {
    if (!__flipTimer.workable())
        return;
    int colorSetting[3];
    __colorChoose(color, colorSetting);
    __flipState = (__flipState) ? __colorGradientTo(colorSetting, __color_0)
                                : !__colorGradientTo(__color_0, colorSetting);
}

void LightingUtils::startColorBreathDelay(Color color, int duration) {
    int colorSetting[3];
    __colorChoose(color, colorSetting);
    __colorGradientToDelay(colorSetting, __color_0, duration / 2);
    __colorGradientToDelay(__color_0, colorSetting, duration / 2);
}

void LightingUtils::LightingUtilsInit() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    pinMode(LED_PWR, OUTPUT);
    digitalWrite(LED_PWR, HIGH);

    // Smooth RGB
    pinMode(Rpin, OUTPUT);
    pinMode(Gpin, OUTPUT);
    pinMode(Bpin, OUTPUT);

    digitalWrite(Rpin, HIGH);
    digitalWrite(Gpin, HIGH);
    digitalWrite(Bpin, HIGH);
}

LightingUtils* LightingUtils::LightingUtilsInstance() {
    if (__lightInstance == NULL)
        __lightInstance = new LightingUtils;
    return __lightInstance;
}

void LightingUtils::startColorGradient() {
    if (__rgbDelay.workable())
        __colorScheduling();
}

void LightingUtils::startColorGradientDelay(int duration) {
    __colorSchedulingDelay(duration);
}

void LightingUtils::setColor(int r, int g, int b) {
    analogWrite(Rpin, r);
    analogWrite(Gpin, g);
    analogWrite(Bpin, b);
}

void LightingUtils::setColor(Color color) {
    int colorSetting[3];
    __colorChoose(color, colorSetting);
    setColor(colorSetting[0], colorSetting[1], colorSetting[2]);
}

void LightingUtils::setColorWithDelay(Color color, int duration) {
    setColor(color);
    delay(duration);
}

void LightingUtils::__colorScheduling() {
    int transition;
    switch (__swtichNumNoDelay) {
        case 0:
            transition = !__colorGradientTo(__color_1, __color_5);
            break;
        case 1:
            transition = !__colorGradientTo(__color_5, __color_9);
            break;
        case 2:
            transition = !__colorGradientTo(__color_9, __color_1);
            break;
    }
    if (transition && ++__swtichNumNoDelay > 2)
        __swtichNumNoDelay = 0;
}

void LightingUtils::__colorSchedulingDelay(int duration) {
    switch (__swtichNum++ % 12) {
        case 0:
            __colorGradientToDelay(__color_1, __color_2, duration);
            break;
        case 1:
            __colorGradientToDelay(__color_2, __color_3, duration);
            break;
        case 2:
            __colorGradientToDelay(__color_3, __color_4, duration);
            break;
        case 3:
            __colorGradientToDelay(__color_4, __color_5, duration);
            break;
        case 4:
            __colorGradientToDelay(__color_5, __color_6, duration);
            break;
        case 5:
            __colorGradientToDelay(__color_6, __color_7, duration);
            break;
        case 6:
            __colorGradientToDelay(__color_7, __color_8, duration);
            break;
        case 7:
            __colorGradientToDelay(__color_8, __color_9, duration);
            break;
        case 8:
            __colorGradientToDelay(__color_9, __color_10, duration);
            break;
        case 9:
            __colorGradientToDelay(__color_10, __color_11, duration);
            break;
        case 10:
            __colorGradientToDelay(__color_11, __color_12, duration);
            break;
        case 11:
            __colorGradientToDelay(__color_12, __color_1, duration);
            break;
    }
}

void LightingUtils::__colorReverse(int color[3]) {
    color[0] = 255 - color[0];
    color[1] = 255 - color[1];
    color[2] = 255 - color[2];
}

bool LightingUtils::__colorGradientTo(const int* from, const int* to) {
    if (__colorLS._colorCounter++ < 256) {
        int r = int(from[0] + (to[0] - from[0]) * __colorLS._colorCounter / 255);
        int g = int(from[1] + (to[1] - from[1]) * __colorLS._colorCounter / 255);
        int b = int(from[2] + (to[2] - from[2]) * __colorLS._colorCounter / 255);
        setColor(r, g, b);
        return true;
    }
    __colorLS._colorCounter = 1;
    return false;
}

void LightingUtils::__colorGradientToDelay(const int* from, const int* to, int duration) {
    int perDelay = duration / 64;
    for (int i = 1; i < 64; i++) {
        int r = int(from[0] + (to[0] - from[0]) * i / 255);
        int g = int(from[1] + (to[1] - from[1]) * i / 255);
        int b = int(from[2] + (to[2] - from[2]) * i / 255);
        setColor(r, g, b);
        delay(perDelay);
    }
}

Color LightingUtils::getColor() { 
    return __colorLS._color; 
}

// FIXME: digitalWrite & analogWrite conflit
// LED PIN (1, 255)
void LightingUtils::setPinOn() {
    __pinLS._pinBit = true;
    digitalWrite(LED_PIN, HIGH);
    analogWrite(LED_PIN, 255);
}

void LightingUtils::setPinOff() {
    __pinLS._pinBit = false;
    digitalWrite(LED_PIN, LOW); // can't set when low
    analogWrite(LED_PIN, 1);
}

void LightingUtils::startPinBreath() {
    if (!__pinDelay.workable())
        return;
    int stat = getPinState();
    if (stat && __pinLS._pinCounter < 256) {
        analogWrite(LED_PIN, __pinLS._pinCounter++);
    } else if (!stat && __pinLS._pinCounter >= 0) {
        analogWrite(LED_PIN, __pinLS._pinCounter--);
    } else if (stat && __pinLS._pinCounter == 256) {
        setPinOff();
    } else {
        setPinOn();
    }
}

void LightingUtils::startPinBreathDelay(int duration) {
    for (int i = 0; i < 256; i++) {
        analogWrite(LED_PIN, i);
        delay(duration / 2 / 256);
    }
    for (int i = 255; i >= 0; i--) {
        analogWrite(LED_PIN, i);
        delay(duration / 2 / 256);
    }
}

void LightingUtils::stopPinBreath() {
  setPinOff();
}

bool LightingUtils::getPinState() { 
    return __pinLS._pinBit; 
}

// FIXME: what the fuck
// LED POWER (1, 255)
void LightingUtils::setPowOn() {
    __powLS._powBit = true;
    digitalWrite(LED_PWR, HIGH);
    // analogWrite(LED_PWR, 255);
}

void LightingUtils::setPowOff() {
    __powLS._powBit = false;
    digitalWrite(LED_PWR, LOW);  // can't set when low
    // analogWrite(LED_PWR, 1);
}

void LightingUtils::startPowBreath() {
    if (!__powDelay.workable())
        return;
    int stat = getPowState();
    if (stat && __powLS._powCounter < 256) {
        analogWrite(LED_PWR, __powLS._powCounter++);
    } else if (!stat && __powLS._powCounter >= 0) {
        analogWrite(LED_PWR, __powLS._powCounter--);
    } else if (stat && __powLS._powCounter == 256) {
        setPowOff();
    } else {
        setPowOn();
    }
}

void LightingUtils::startPowBreathDelay(int duration) {
    for (int i = 0; i < 256; i++) {
        analogWrite(LED_PWR, i);
        delay(duration / 2 / 256);
    }
    for (int i = 255; i >= 0; i--) {
        analogWrite(LED_PWR, i);
        delay(duration / 2 / 256);
    }
}

void LightingUtils::stopPowBreath() {
  setPowOff();
}

bool LightingUtils::getPowState() { 
    return __powLS._powBit; 
}

#endif // __LIGHTING_UTILS_HPP__
