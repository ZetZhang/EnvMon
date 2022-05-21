#ifndef __DELAY_SWITCHING_HPP__
#define __DELAY_SWITCHING_HPP__

// one timer per light need
class TimerDelay 
{
public:
    bool workable();
    TimerDelay(unsigned long interval);
    void reset();

private:
    const long __interval;
    unsigned long __previousMillis;
    unsigned long __currentMilli;
};

TimerDelay::TimerDelay(unsigned long interval)
    : __interval(interval)
    , __previousMillis(millis())
    , __currentMilli(0)
{
}

bool TimerDelay::workable() {
    __currentMilli = millis();
    if (__currentMilli - __previousMillis >= __interval) {
        __previousMillis = __currentMilli;
        return true;
    }
    return false;
}

void TimerDelay::reset() {
    __previousMillis = millis();
    __currentMilli = 0;
}

#endif // __DELAY_SWITCHING_HPP__
