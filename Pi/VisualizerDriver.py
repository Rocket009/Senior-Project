from I2C_RPI_Interface import BaseI2C


_HT16K33_BLINK_CMD = const(0x80)
_HT16K33_BLINK_DISPLAYON = const(0x01)
_HT16K33_CMD_BRIGHTNESS = const(0xE0)
_HT16K33_OSCILATOR_ON = const(0x21)

RETRY_MAX = 10
RETRY_WAIT_SEC = 1.0
HIGH = 1
LOW = 0

class HT16K33:
    """The base class for all HT16K33-based backpacks and wings."""

    def __init__(self, i2c_obj: BaseI2C, address=0x70, auto_write=True, brightness=1.0):
        self.i2c_device = i2c_obj
        self._address = address
        self._temp = bytearray(1)
        self._buffer = bytearray(17)
        self._auto_write = auto_write
        self.fill(0)
        self._write_cmd(_HT16K33_OSCILATOR_ON)
        self._blink_rate = 0
        self._brightness = None
        self.blink_rate = 0
        self.brightness = brightness
        self.fill(0)

    def _write_cmd(self, byte):
        self._temp[0] = byte
        self.i2c_device.writeto(self._address, self._temp)

    @property
    def blink_rate(self) -> int:
        """The blink rate. Range 0-3."""
        return self._blink_rate

    @blink_rate.setter
    def blink_rate(self, rate: int):
        if not 0 <= rate <= 3:
            raise ValueError("Blink rate must be an integer in the range: 0-3")
        rate = rate & 0x03
        self._blink_rate = rate
        self._write_cmd(_HT16K33_BLINK_CMD | _HT16K33_BLINK_DISPLAYON | rate << 1)

    @property
    def brightness(self):
        """The brightness. Range 0.0-1.0"""
        return self._brightness

    @brightness.setter
    def brightness(self, brightness):
        if not 0.0 <= brightness <= 1.0:
            raise ValueError(
                "Brightness must be a decimal number in the range: 0.0-1.0"
            )

        self._brightness = brightness
        xbright = round(15 * brightness)
        xbright = xbright & 0x0F
        self._write_cmd(_HT16K33_CMD_BRIGHTNESS | xbright)

    @property
    def auto_write(self):
        """Auto write updates to the display."""
        return self._auto_write

    @auto_write.setter
    def auto_write(self, auto_write):
        if isinstance(auto_write, bool):
            self._auto_write = auto_write
        else:
            raise ValueError("Must set to either True or False.")

    def show(self):
        """Refresh the display and show the changes."""
        # Byte 0 is 0x00, address of LED data register. The remaining 16
        # bytes are the display register data to set.
        self.i2c_device.writeto(self._address, self._buffer)

    def fill(self, color):
        """Fill the whole display with the given color."""
        fill = 0xFF if color else 0x00
        for i in range(16):
            self._buffer[i + 1] = fill
        if self._auto_write:
            self.show()

    def set_pixel(self, x: int, y: int, val: int | bool):
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
        if self._auto_write:
            self.show()

    def _set_buffer(self, i, value):
        self._buffer[i + 1] = value  # Offset by 1 to move past register address.

    def _get_buffer(self, i):
        return self._buffer[i + 1]  # Offset by 1 to move past register address.

class VisualizerDriver:
    def __init__(self, i2c_obj: BaseI2C, address=0x70):
        self._ht = HT16K33(address=address, i2c_obj=i2c_obj, auto_write=False)

    def set_bar(self, bar_num: int, level: int):
        """Sets the bar bar_num to a level between 0-10"""
        if level > 11 or level < 0:
            raise ValueError("Level must be between 0-10")

        if bar_num < 8:
            # clear the bar
            for i in range(10):
                self._ht.set_pixel(bar_num, i, LOW)
            # set the bar
            for i in range(level):
                self._ht.set_pixel(bar_num, i, HIGH)

        elif bar_num >= 8 and bar_num < 10:
            new_bar_num = (bar_num - 8) * 2
            # clear the bar
            for i in range(new_bar_num, new_bar_num + 2):
                for j in range(10, 16):
                    self._ht.set_pixel(i, j, LOW)

            if level > 6:
                for j in range(10, 16):
                    self._ht.set_pixel(new_bar_num, j, HIGH)
                for j in range(10, 10 + (level - 6)):
                    self._ht.set_pixel(new_bar_num + 1, j, HIGH)
            else:
                for j in range(10, 10 + level):
                    self._ht.set_pixel(new_bar_num, j, HIGH) 
        else:
            raise ValueError("Bar num must be inbetween 0-9")

        self._ht.show()

    def set_brightness(self, level: float):
        if not 0.0 <= level <= 1.0:
            raise ValueError("Brightness must be a decimal number in the range: 0.0-1.0")
        self._ht.brightness = level