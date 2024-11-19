from machine import Pin, PWM, SoftI2C
import time
from time import sleep
import math
#from as5600 import *


# Constants for AS5600
AS5600_I2C_ADDR = 0x36
ANGLE_REG = 0x0E

# PID constants
# Adjusting PID parameters
Kp = 11  # Try increasing slightly
Ki = 0.01  # Add a small integral gain
Kd = 0.1  # Adjust derivative gain as needed

# Initialize I2C for AS5600
i2c = SoftI2C(scl=Pin(15), sda=Pin(14))


nFault = Pin(6, Pin.IN)   # not-Fault input

nSleep = Pin(7, Pin.OUT)  # not-Sleep output
nSleep.on()               # set to on to not activate sleep mode

nReset = Pin(8, Pin.OUT)  # not-Reset output
nReset.on()               # set to on to not reset the driver


# definition of enable and PWM pins
en = Pin(5, Pin.OUT)  # enable pin
en.on()               # set to on to enable the driver
# Initialize PWM for motor phases
phase_a_pwm = PWM(Pin(2))
phase_b_pwm = PWM(Pin(3))
phase_c_pwm = PWM(Pin(4))

# Set frequency for PWM channels
frequency = 25000  # Typical for BLDC motors
for pwm in [phase_a_pwm, phase_b_pwm, phase_c_pwm]:
    pwm.freq(frequency)
    pwm.duty_u16(0)  # Initialize with 0 duty cycle

# Function to read angle from AS5600
def read_angle():
    angle_raw = i2c.readfrom_mem(AS5600_I2C_ADDR, ANGLE_REG, 2)
    angle = (angle_raw[0] << 8 | angle_raw[1]) & 0x0FFF
    return (angle / 4096) * 360  # Convert to degrees

# PID Control Logic
previous_error = 0
integral = 0

#60756 so that max duty cycle only reaches 7.4V
def pid_control(target, current):
    global previous_error, integral
    error = target - current
    integral += error
    derivative = error - previous_error
    output = (Kp * error) + (Ki * integral) + (Kd * derivative)
    previous_error = error
    return max(0, min(60756, int(output)))  # Limit to PWM range

# Function for 6-step commutation
def set_motor_phases(angle, pwm_output):
    step = int((angle % 360) / 60)  # Divide 360 degrees by 6 steps
    if step == 0:
        phase_a_pwm.duty_u16(int(pwm_output))  # Phase A to V+
        phase_b_pwm.duty_u16(0)                # Phase B to Ground
        phase_c_pwm.duty_u16(0)                # Phase C Floating
    elif step == 1:
        phase_a_pwm.duty_u16(int(pwm_output))  # Phase A to V+
        phase_b_pwm.duty_u16(0)                # Phase B Floating
        phase_c_pwm.duty_u16(int(pwm_output))  # Phase C to Ground
    elif step == 2:
        phase_a_pwm.duty_u16(0)                # Phase A to Floating
        phase_b_pwm.duty_u16(int(pwm_output))  # Phase B to V+
        phase_c_pwm.duty_u16(int(pwm_output))  # Phase C to Ground
    elif step == 3:
        phase_a_pwm.duty_u16(0)                # Phase A to Floating
        phase_b_pwm.duty_u16(int(pwm_output))  # Phase B to V+
        phase_c_pwm.duty_u16(0)                # Phase C Floating
    elif step == 4:
        phase_a_pwm.duty_u16(int(pwm_output))  # Phase A to V+
        phase_b_pwm.duty_u16(0)                # Phase B Floating
        phase_c_pwm.duty_u16(int(pwm_output))  # Phase C to Ground
    elif step == 5:
        phase_a_pwm.duty_u16(0)                # Phase A to Floating
        phase_b_pwm.duty_u16(0)                # Phase B to Floating
        phase_c_pwm.duty_u16(int(pwm_output))  # Phase C to V+

# Main loop for closed-loop control with haptic feedback
target_position = 0

# Adjust main loop for fine control
while True:

    # Read current position
    current_position = int(read_angle())
    print("Angle:", current_position)

#     # Haptic feedback logic (adjust threshold as needed)
#     for pos in [0, 120, 240]:
#         if abs(current_position - pos) < 2:
#             target_position = pos
#             break

    # PID Control for motor
#     pwm_output = pid_control(target_position, current_position)
    pwm_output = 50000
    print('PWM:', pwm_output)

    # Set motor phases based on angle and output
    set_motor_phases(current_position, pwm_output)

    time.sleep(0.01)  # Delay for control loop stability