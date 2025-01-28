from abc import ABC
from smbus2 import SMBus

class BaseI2C(ABC):
    def __init__(self, bus=None, freq=None):
        raise NotImplementedError

    def readfrom(self, addr: int, nbytes: int) -> bytes:
        raise NotImplementedError

    def writeto(self, addr: int, buf: bytes | bytearray):
        raise NotImplementedError

class I2CDriver(BaseI2C):
    def __init__(self, bus=None):
        self._smbus = SMBus(bus=bus)

    def readfrom(self, addr: int, nbytes: int) -> bytes:
        data = bytearray()
        for _ in range(nbytes):
            data.append(self._smbus.read_byte(addr))
        return bytes(data)

    def writeto(self, addr: int, buf: bytes | bytearray):
        for b in buf:
            self._smbus.write_byte(addr, int(b))

            
