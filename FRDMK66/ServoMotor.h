#ifndef __SERVOMOTOR_H__
#define __SERVOMOTOR_H__

#include "mbed.h"

#define SERVO_CENTER_PULSE  0.0015f     // 1.5ms for neutral (center)
#define SERVO_MIN_PULSE     0.0011f     // 1.1ms for -45 degrees
#define SERVO_MAX_PULSE     0.0019f     // 1.9ms for +45 degrees
#define SERVO_PERIOD        0.020f      // 20ms period (50Hz)

class ServoMotor
{
private:
    PwmOut  servo;
    float   angle;

    void setServoAngle (void)
    {
        if (angle < -45.0f) angle = -45.0f;
        if (angle >  45.0f) angle =  45.0f;

        // Convert angle (-45° to +45°) to pulse width (1.1ms to 1.9ms)
        float pulse_width = SERVO_CENTER_PULSE + 
                            ((angle / 45.0f) * 
                            (SERVO_MAX_PULSE - SERVO_CENTER_PULSE));

        servo.pulsewidth(pulse_width);
    }

public:
    ServoMotor  (PinName pin) : 
    servo       (pin),
    angle       (0.0f)
    {
        servo.period(SERVO_PERIOD);
        setServoAngle();
    }

    void operator= (const float &new_angle)
    {
        if (angle != new_angle)
        {
            angle = new_angle;
            setServoAngle();
        }
    }
};

#endif