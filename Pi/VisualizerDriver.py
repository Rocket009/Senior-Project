from enum import Enum
from common.Exceptions import ModuleNotFoundError, I2CError
from smbus2 import SMBus


BLINK_CMD = 0x80
DISPLAYON = 0x01
DISPLAYOFF = 0x00
CMD_BRIGHTNESS = 0xE0
OSCILATOR_ON = 0x21
DISPLAY_SETUP = 0x80
DISPLAY_DATA_PTR = 0x00

RETRY_MAX = 10
RETRY_WAIT_SEC = 1.0
HIGH = 1
LOW = 0

class DriverState(Enum):
    NOT_INIT = 0
    INIT = 1
    TRANSMITING = 2


class VisualizerDriver:

    def __init__(self, sm: SMBus, address=0x70):
        self._addr = address
        self._sm = sm
        self._state = DriverState.NOT_INIT
        self._temp = bytearray(1)
        self._buffer = bytearray(17)
        self._buffer[0] = DISPLAY_DATA_PTR

    def init_visualizer(self):
        try:
            self._state = DriverState.TRANSMITING
            # turn on oscilator
            self._sm.write_byte(self._addr, OSCILATOR_ON)
            # set brightness to max
            self._sm.write_byte(self._addr, CMD_BRIGHTNESS | 0xF)
            # set turn on display and set blink rate to 0 Hz
            self._sm.write_byte(self._addr, DISPLAY_SETUP | DISPLAYON | 0 << 1)
            self._state = DriverState.INIT
        except OSError:
            self._state = DriverState.NOT_INIT
            raise ModuleNotFoundError("Could not write to Visualizer")

    def display_set(self, value: bool):
        if self._state is DriverState.NOT_INIT:
            self.init_visualizer()
        try:
            self._state = DriverState.TRANSMITING
            if value:
                self._sm.write_byte(self._addr, DISPLAY_SETUP | DISPLAYON)
            else:
                self._sm.write_byte(self._addr, DISPLAY_SETUP | DISPLAYOFF)
            self._state = DriverState.INIT
        except OSError:
            self._state = DriverState.NOT_INIT
            raise I2CError
    
    def _set_pixel(self, x: int, y: int, val: int | bool):
        if x > 7 or y > 15:
            raise ValueError("X and Y must be between 0-7 and 0-15 respectivly")
        addr = 2 * x + y // 8
        mask = 1 << y % 8
        if val:
            # set the bit
            self._buffer[addr + 1] |= mask
        else:
            # clear the bit
            self._buffer[addr + 1] &= ~mask
    
    def _write_buffer(self):
        if self._state is DriverState.NOT_INIT:
            self.init_visualizer()
        try:
            self._state = DriverState.TRANSMITING
            self._sm.write_i2c_block_data(self._addr, self._buffer[0], self._buffer[1:])
            self._state = DriverState.INIT
        except OSError:
            self._state = DriverState.NOT_INIT
            raise I2CError

    def set_bar(self, bar_num: int, level: int):
        """Sets the bar bar_num to a level between 0-10"""
        if level > 11 or level < 0:
            raise ValueError("Level must be between 0-10")

        if bar_num < 8:
            # clear the bar
            for i in range(10):
                self._set_pixel(bar_num, i, LOW)
            # set the bar
            for i in range(level):
                self._set_pixel(bar_num, i, HIGH)

        elif bar_num >= 8 and bar_num < 10:
            new_bar_num = (bar_num - 8) * 2
            # clear the bar
            for i in range(new_bar_num, new_bar_num + 2):
                for j in range(10, 16):
                    self._set_pixel(i, j, LOW)

            if level > 6:
                for j in range(10, 16):
                    self._set_pixel(new_bar_num, j, HIGH)
                for j in range(10, 10 + (level - 6)):
                    self._set_pixel(new_bar_num + 1, j, HIGH)
            else:
                for j in range(10, 10 + level):
                    self._set_pixel(new_bar_num, j, HIGH) 
        else:
            raise ValueError("Bar num must be inbetween 0-9")

        self._write_buffer()

    def set_brightness(self, level: float):
        if not 0.0 <= level <= 1.0:
            raise ValueError("Brightness must be a decimal number in the range: 0.0-1.0")
        if self._state is DriverState.NOT_INIT:
            self.init_visualizer()
        xbright = round(15 * level)
        xbright = xbright & 0x0F
        try:
            self._sm.write_byte(self._addr, CMD_BRIGHTNESS | xbright)
        except OSError:
            self._state = DriverState.NOT_INIT
            raise I2CError