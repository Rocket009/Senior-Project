#include <Wire.h>
#include <Arduino.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define AS5600_I2C_ADDR 0x36
#define ANGLE_REG 0x0E

// PID constants
const float Kp = 11.0;
const float Ki = 0.01;
const float Kd = 0.1;

// Pin definitions
const int nFault = 6;
const int nSleep = 7;
const int nReset = 8;
const int en = 5;
const int phaseA = 2;
const int phaseB = 3;
const int phaseC = 4;

int previous_error = 0;
float integral = 0;

void setup() {
    stdio_init_all();
    Wire.begin();

    pinMode(nFault, INPUT);
    pinMode(nSleep, OUTPUT);
    pinMode(nReset, OUTPUT);
    pinMode(en, OUTPUT);
    pinMode(phaseA, OUTPUT);
    pinMode(phaseB, OUTPUT);
    pinMode(phaseC, OUTPUT);

    digitalWrite(nSleep, HIGH);
    digitalWrite(nReset, HIGH);
    digitalWrite(en, HIGH);

    pwm_set_wrap(pwm_gpio_to_slice_num(phaseA), 65535);
    pwm_set_wrap(pwm_gpio_to_slice_num(phaseB), 65535);
    pwm_set_wrap(pwm_gpio_to_slice_num(phaseC), 65535);
    
    pwm_set_chan_level(pwm_gpio_to_slice_num(phaseA), PWM_CHAN_A, 0);
    pwm_set_chan_level(pwm_gpio_to_slice_num(phaseB), PWM_CHAN_A, 0);
    pwm_set_chan_level(pwm_gpio_to_slice_num(phaseC), PWM_CHAN_A, 0);
}

int readAngle() {
    Wire.beginTransmission(AS5600_I2C_ADDR);
    Wire.write(ANGLE_REG);
    Wire.endTransmission(false);
    
    Wire.requestFrom(AS5600_I2C_ADDR, 2);
    if (Wire.available() == 2) {
        int angle = (Wire.read() << 8) | Wire.read();
        return (angle & 0x0FFF) * 360 / 4096;
    }
    return 0;
}

int pidControl(int target, int current) {
    int error = target - current;
    integral += error;
    int derivative = error - previous_error;
    int output = (Kp * error) + (Ki * integral) + (Kd * derivative);
    previous_error = error;
    return constrain(output, 0, 60756);
}

void setMotorPhases(int angle, int pwmOutput) {
    int step = (angle % 360) / 60;
    int duty = pwmOutput;
    
    switch (step) {
        case 0: pwm_set_chan_level(pwm_gpio_to_slice_num(phaseA), PWM_CHAN_A, duty); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseB), PWM_CHAN_A, 0); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseC), PWM_CHAN_A, 0); break;
        case 1: pwm_set_chan_level(pwm_gpio_to_slice_num(phaseA), PWM_CHAN_A, duty); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseB), PWM_CHAN_A, 0); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseC), PWM_CHAN_A, duty); break;
        case 2: pwm_set_chan_level(pwm_gpio_to_slice_num(phaseA), PWM_CHAN_A, 0); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseB), PWM_CHAN_A, duty); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseC), PWM_CHAN_A, duty); break;
        case 3: pwm_set_chan_level(pwm_gpio_to_slice_num(phaseA), PWM_CHAN_A, 0); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseB), PWM_CHAN_A, duty); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseC), PWM_CHAN_A, 0); break;
        case 4: pwm_set_chan_level(pwm_gpio_to_slice_num(phaseA), PWM_CHAN_A, duty); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseB), PWM_CHAN_A, 0); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseC), PWM_CHAN_A, duty); break;
        case 5: pwm_set_chan_level(pwm_gpio_to_slice_num(phaseA), PWM_CHAN_A, 0); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseB), PWM_CHAN_A, 0); pwm_set_chan_level(pwm_gpio_to_slice_num(phaseC), PWM_CHAN_A, duty); break;
    }
}

void loop() {
    int currentPosition = readAngle();
    printf("Angle: %d\n", currentPosition);

    int pwmOutput = 50000;
    printf("PWM: %d\n", pwmOutput);
    
    setMotorPhases(currentPosition, pwmOutput);
    sleep_ms(10);
}
