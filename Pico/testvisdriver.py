from common.VisualizerDriver import VisualizerDriver
from common.I2CInterfaces import BaseI2C
from machine import I2C as PicoI2C
import sys

class I2C(PicoI2C, BaseI2C):
    pass


def map_val(x):
    return (x - (-1)) * (10 - 0) / (1 - (-1)) + 0

h = VisualizerDriver(I2C(0, freq=200000))
t = 0
while True:
    # for i in range(11):
    #     h.set_bar(0, i)
    #     sleep_ms(500)
    # for i in range(11):
    #     h.set_bar(1, i)
    #     sleep_ms(500)
    # for i in range(11):
    #     h.set_bar(8, i)
    #     sleep_ms(500)
    data = sys.stdin.readline()
    data = data.split(",")
    # h.set_bar(0, 2 + randint(0, 8)) 
    # h.set_bar(1, 2 + randint(0, 8)) 
    # h.set_bar(8, 2 + randint(0, 8)) 
    h.set_bar(0, int(data[0]))
    h.set_bar(1, int(data[2]))
    h.set_bar(8, int(data[4]))
    # sleep_ms(50)