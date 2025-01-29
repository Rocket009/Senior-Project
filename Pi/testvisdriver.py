from VisualizerDriver import VisualizerDriver
from time import sleep_ms
from I2CDriver import I2CDriver

BARS_EN = (1, 2, 9)
try:
    v = VisualizerDriver(I2CDriver(1))
    while True:
        for b in BARS_EN:
            for l in range(11):
                v.set_bar(b, l)
                if l % 2 == 0:
                    v.set_brightness(1.0)
                else:
                    v.set_brightness(0.5)
                sleep_ms(200)

except KeyboardInterrupt:
    pass