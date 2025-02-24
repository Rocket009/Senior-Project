from common.I2CInterfaces import BaseI2C, I2CError
from smbus2 import SMBus

class I2CDriver(BaseI2C):
    """I2C Driver for RPI Zero W"""

    def __init__(self, bus=None):
        self._smbus = SMBus(bus=bus)

    def readfrom(self, addr: int, nbytes: int, force=None) -> bytes:
        try:
            if nbytes == 1:
                return bytes(self._smbus.read_byte(addr), force=force)
            elif nbytes > 1:
                data = bytearray()
                for _ in range(nbytes):
                     data.append(self._smbus.read_byte(addr, force=force))
                return bytes(data)
            else:
                 raise ValueError("nbytes must be > 0")
        except OSError:
             raise I2CError(f"Cannot read from I2C device addr: {addr}")

    def readfrom_reg(self, addr: int, reg: int, nbytes: int, force=None) -> bytes:
            try:
                data = bytearray()
                list_bytes = self._smbus.read_i2c_block_data(addr, reg, nbytes, force=force)
                for b in list_bytes:
                     data.append(b)
                return bytes(data)
            except OSError:
                 raise I2CError(f"Cannot read from I2C device addr: {addr} reg: {reg}")

    def writeto(self, addr: int, buf: bytes | bytearray, force=None):
            try:
                if len(buf) == 1:
                    self._smbus.write_byte(addr, buf[0], force=force)
                else:
                    self._smbus.write_i2c_block_data(addr, buf[0], buf[1:], force=force)
            except OSError:
                raise I2CError(f"Cannot write to I2C device addr: {addr}")

    def writeto_reg(self, addr: int, reg: int, buf: bytes | bytearray, force=None):
        try:
            if len(buf) == 1:
                self._smbus.write_byte_data(addr, reg, buf[0], force=force)
            elif len(buf) > 1:
                   self._smbus.write_i2c_block_data(addr, reg, buf, force=force)
            else:
                raise ValueError("Buf size must be greater than 0")
        except OSError:
            raise I2CError(f"Cannot write to I2C device addr: {addr} reg: {reg}")
              


            
