from time import sleep_ms
while True:
    for i in range(0, 360, 5):
        print(f"Angle:{i}")
        sleep_ms(500)