from tkinter import * 
from tkinter import ttk 
from tkinter.ttk import Progressbar
import random
import time
import smbus


bus = smbus.SMBus(0)
address = 0x36
angle_register = 0x0E

# Creating tkinter window  
root = Tk()  
root.geometry("800x480")  
root.configure(bg='grey')
style = ttk.Style(root)
style.theme_use('clam')  # Use a theme that allows styling

style.configure('Custom.Vertical.TProgressbar',
                troughcolor='black',   # Track background color
                background='green',    # Bar fill color
                thickness=30)          # Adjust thickness (not always supported)
# Create 5 vertical sliders
sliders = [Progressbar(root, style ="Custom.Vertical.TProgressbar",orient=VERTICAL, length=350, mode='determinate') for _ in range(5)]
# container = Frame(root)
# container.pack(side=BOTTOM, pady=10)
# frame = Frame(container, bd=3, relief="solid", padx=10, pady=10)  # Box around each slider
# frame.pack(side=LEFT, padx=20, pady=10)  # Spacing between boxes


# Pack sliders side by side
for slider in sliders:
    slider.pack(side=LEFT, padx=65, pady=50)

# Function to update slider values
def update_sliders():
    for slider in sliders:
        slider['value'] = bus.read_byte_data(address, angle_register)#random.randint(0, 100)  # Replace with actual sensor data
    root.after(100, update_sliders)  # Schedule next update

# Start updating sliders
update_sliders()

# Start GUI event loop
root.mainloop()