"""Module for I2C interface and Execptions"""
from abc import ABC

class BaseI2C(ABC):
    """interface for I2C implementations"""

    def __init__(self, bus=None, freq=None):
        raise NotImplementedError

    def readfrom(self, addr: int, nbytes: int) -> bytes:
        raise NotImplementedError

    def writeto(self, addr: int, buf: bytes | bytearray):
        raise NotImplementedError

class I2CError(Exception):
    """raised when cant comunicate with I2C device"""
