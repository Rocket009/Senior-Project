"""Module for I2C interface and Execptions"""
from abc import ABC

class BaseI2C(ABC):
    """interface for I2C implementations"""

    def __init__(self, bus=None, freq=None):
        raise NotImplementedError

    def readfrom(self, addr: int, nbytes: int) -> bytes:
        """
        Reads from the address a number of bytes nbytes must be > 0
        Reads on seperate transactions. To read on one transaction use
        ``readfrom_reg``
        """        
        raise NotImplementedError

    def writeto(self, addr: int, buf: bytes | bytearray):
        """
        Writes to the addr the buffer of bytes if len of bytes is more than one then
        writes in one transaction
        """
        raise NotImplementedError

    def readfrom_reg(self, addr: int, reg: int, nbytes: int, force=None) -> bytes:
        """
        Reads from register at addr the number of bytes does so in one transaction
        """
        raise NotImplementedError

    def writeto_reg(self, addr: int, reg: int, buf: bytes | bytearray, force=None):
        """
        Writes to a register at addr the bytes in buf does so in one transaction
        """
        raise NotImplementedError

class I2CError(Exception):
    """raised when cant comunicate with I2C device"""
