import pyaudio
import numpy as np
import serial
import time

ser = serial.Serial("COM9", 115200, timeout=1)
# Configuration
FORMAT = pyaudio.paInt16  # 16-bit audio
CHANNELS = 1              # Mono for simplicity
RATE = 44100              # Sample rate in Hz
CHUNK = 1024              # Buffer size
DURATION = 10             # Recording duration in seconds

# Define EQ bands in Hz
BANDS = [
    (20, 60, 0.1),     # Sub-bass
    (60, 125, 0.1),    # Bass
    (125, 250, 0.1),   # Low midrange
    (250, 500, 0.1),  # Midrange
    (500, 1000, 0.1), # Upper midrange
    (1000, 2000, 0.5), # Presence
    (2000, 4000, 0.5),# Brilliance
    (4000, 8000, 1), # Air (optional extended)
    (8000, 16000, 1), #Ultra Highs (Above audible)
]

# Find the loopback device
p = pyaudio.PyAudio()
loopback_device_index = None
for i in range(p.get_device_count()):
    device_info = p.get_device_info_by_index(i)
    if "loopback" in device_info["name"].lower() or "stereo mix" in device_info["name"].lower():
        loopback_device_index = i
        break

if loopback_device_index is None:
    print("Loopback device not found. Ensure it is enabled in your sound settings.")
    exit(1)

# Open audio stream
stream = p.open(format=FORMAT,
                channels=CHANNELS,
                rate=RATE,
                input=True,
                frames_per_buffer=CHUNK,
                input_device_index=2)

print("Capturing audio and performing FFT...")

# Frequency bins for the FFT
freq_bins = np.fft.rfftfreq(CHUNK, d=1/RATE)

def moving_average(data):
    ema = np.zeros_like(data)
    alpha = 0.3
    ema[0] = data[0]
    for t in range(1, len(data)):
        ema[t] = alpha * data[t] + (1 - alpha) * ema[t - 1]
    return ema

def map_val(x, maxx, minn):
    return (x - minn) * (10 - 0) / (maxx - minn) + 0
# Function to compute magnitude of FFT in EQ bands
def normalize_eq(eq_magnitudes, reference=1.0):
    # Logarithmic scaling for better visual effect
    eq_magnitudes = np.log1p(eq_magnitudes)  # log(1 + x) to handle small values
    max_value = np.log1p(reference)
    normalized = (eq_magnitudes / max_value) * 100  # Scale to [0, 100]
    return np.clip(normalized, 0, 100)  # Clamp values between 0 and 100

def compute_eq_bands(fft_magnitude, freq_bins, bands):
    eq_magnitudes = []
    for band in bands:
        band_mask = (freq_bins >= band[0]) & (freq_bins < band[1])
        band_magnitude = np.mean(fft_magnitude[band_mask]  ) if np.any(band_mask) else 0
        # eq_magnitudes.append(10 * np.log10(band_magnitude / fft_magnitude.max()))
        eq_magnitudes.append(10 * np.log10(band_magnitude ** 2))
    return eq_magnitudes

try:
    while True:
        # Read audio data
        data = stream.read(CHUNK, exception_on_overflow=False)
        audio_data = np.frombuffer(data, dtype=np.int16)
        
        # Perform FFT
        fft_result = np.fft.rfft(audio_data)
        fft_magnitude = np.abs(fft_result)
        # fft_magnitude = 10 * np.log10(fft_magnitude / fft_magnitude.mean())
        
        # Compute EQ band magnitudes
        eq_magnitudes = compute_eq_bands(fft_magnitude, freq_bins, BANDS)
        m = max(eq_magnitudes)
        mm = min(eq_magnitudes)
        # Print results
        # print(f"EQ Magnitudes: {['{:.2f}'.format(mag) for mag in (eq_magnitudes)]}")
        da = ",".join(map_val(eq_magnitudes, m, mm).astype(int).astype(str)) + "\n"
        print(da, end="")
        ser.write(da.encode())
        time.sleep(0.05)
        
except KeyboardInterrupt:
    print("\nStopped.")

finally:
    # Cleanup
    stream.stop_stream()
    stream.close()
    p.terminate()
    ser.close()

print("Finished.")
