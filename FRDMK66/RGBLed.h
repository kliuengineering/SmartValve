// RGBLed.h
#ifndef __RGBLED_H__
#define __RGBLED_H__

#include "mbed.h"

class RGBLed {
private:
    PwmOut red;
    PwmOut green;
    PwmOut blue;

public:
    RGBLed(PinName r, PinName g, PinName b)
        : red(r), green(g), blue(b) {
        red.period_ms(2);
        green.period_ms(2);
        blue.period_ms(2);
        setColor(0.0f, 0.0f, 0.0f);  // Off
    }

    void setColor(float r, float g, float b) {
        red.write(r);
        green.write(g);
        blue.write(b);
    }
};

#endif
