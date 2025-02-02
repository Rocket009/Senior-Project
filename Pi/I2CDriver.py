from common.I2CInterfaces import BaseI2C, I2CError
from smbus2 import SMBus

class I2CDriver(BaseI2C):
    """I2C Driver for RPI Zero W"""

    def __init__(self, bus=None):
        self._smbus = SMBus(bus=bus)

    def readfrom(self, addr: int, nbytes: int) -> bytes:
        data = bytearray()
        for _ in range(nbytes):
            try:
                data.append(self._smbus.read_byte(addr))
            except OSError:
                raise I2CError(f"Cannot read I2C device addr: {addr}")
        return bytes(data)

    def writeto(self, addr: int, buf: bytes | bytearray):
            try:
                if len(buf) == 1:
                    self._smbus.write_byte(addr, buf[0])
                else:
                    self._smbus.write_i2c_block_data(addr, buf[0], buf[1:])
            except OSError:
                raise I2CError(f"Cannot write to I2C device addr: {addr}")


            
