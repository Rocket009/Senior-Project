from common.Exceptions import I2CError
from smbus2 import SMBus, i2c_msg


# configured for single-ended input and internal ref off and A/D converter power down between conversions
# must add the channel selections for c2, c1, c0 in bit positions 6, 5, 4 respectivly
COMMAND_BYTE = 1 << 7
REF_VOLTAGE = 5

class ADCDriver:
    def __init__(self, sm: SMBus, address=0x48):
        self._sm = sm
        self._addr = address
    
    def sample_channel(self, chan: int, float_type=True) -> int | float:
        if chan > 7 or chan < 0:
            raise ValueError("Channel num must be between 0 and 7")
        write_msg = i2c_msg.write(self._addr, [COMMAND_BYTE | chan << 4])
        read_msg = i2c_msg.read(self._addr, 1)
        try:
            self._sm.i2c_rdwr(write_msg, read_msg)
            adc_val = int.from_bytes(read_msg.buf[0], byteorder="big")
            if float_type:
                res = REF_VOLTAGE / 255
                return res * adc_val
            else:
                return adc_val
        except OSError:
            raise I2CError("Could not r/w to ADC Driver")

        

