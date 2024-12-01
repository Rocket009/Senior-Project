import sys
from PySide6.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QComboBox, QDial, QLabel, QPushButton
from PySide6.QtCore import Qt, QThread, Signal, Slot
from PySide6.QtGui import QFont
import serial
import serial.tools.list_ports
import time
import re
from WindowsVolumeController import WindowsAudioController
from WebAppThread import WebApp
import json
    
number_t = int | float
def map_value(x: number_t, in_min: number_t, in_max: number_t, out_min: number_t, out_max: number_t) -> float:
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

class SerialReader(QThread):
    data_received = Signal(str)  # Signal to send data back to the main thread

    def __init__(self, port, baudrate=115200):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.running = False

    def run(self):
        """Thread run method: reads data from the serial port."""
        self.ser = serial.Serial(self.port, self.baudrate, timeout=1)
        self.running = True
        while self.running:
            data = self.ser.read_until("\n").decode("utf-8").strip()
            self.data_received.emit(data)  # Emit the received data
            time.sleep(0.1)  # Small delay to avoid overwhelming the thread
        self.ser.close()

    def stop(self):
        """Stop the thread."""
        self.running = False

class SimpleGui(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Volume Controller Demo")
        self.setFixedSize(400, 400)
        self.volume_controller = WindowsAudioController()
        self.last_angle = 0
        # Layouts
        main_layout = QVBoxLayout()
        dropdown_layout = QHBoxLayout()
        
        # Dropdown 1
        self.dropdown1 = QComboBox()
        self.dropdown1.addItems(self.volume_controller.get_process_names())
        dropdown_layout.addWidget(self.dropdown1)
        self.dropdown1.currentIndexChanged.connect(self.on_process_change)

        self.dropdown2 = QComboBox()
        self.dropdown2.addItems(self.volume_controller.get_endpoints())
        dropdown_layout.addWidget(self.dropdown2)
        self.dropdown2.currentIndexChanged.connect(self.on_endpoint_change)
        
        self.dropdown3 = QComboBox()
        ports = serial.tools.list_ports.comports()
        self.dropdown3.addItems([port.device for port in ports])
        dropdown_layout.addWidget(self.dropdown3)
        
        # Large Dial
        self.dial = QDial()
        self.dial.setMinimum(0)
        self.dial.setMaximum(100)
        self.dial.setValue(50)
        self.dial.setNotchesVisible(True)
        self.dial.setFixedSize(200, 200)  # Make it large
        
        # Dial Value Label
        self.dial_label = QLabel("Value: 50")
        self.dial_label.setFont(QFont("Arial", 16))
        self.dial_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        # Connect dial to label
        self.dial.valueChanged.connect(self.update_label)
        
        # Connect Button
        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.connect_serial)

        # Add widgets to main layout
        main_layout.addLayout(dropdown_layout)
        main_layout.addWidget(self.dial_label)
        main_layout.addWidget(self.dial, alignment=Qt.AlignmentFlag.AlignCenter)
        main_layout.addWidget(self.connect_button)

        # Set main layout
        self.setLayout(main_layout)

        # Serial Thread
        self.serial_thread = None
        self.web_app_thread = WebApp()
        self.web_app_thread.recv_data.connect(self.handle_json)
        self.web_app_thread.set_start_callback(lambda: {"process": self.dropdown1.currentText(), "volume": self.dial.value()})
        self.web_app_thread.start()
    def on_process_change(self, index):
        selected_value = self.dropdown1.currentText()
        try:
            self.volume_controller.set_selected_process_name(selected_value)
        except ValueError:
            self.dropdown1.removeItem(index)

    def handle_json(self, data: dict):
        self.dial.setValue(int(data["volume"]))

    def on_endpoint_change(self, index):
        selected_value = self.dropdown2.currentText()
        self.volume_controller.set_endpoint_device(selected_value)
        self.dropdown1.clear()
        self.dropdown1.addItems(self.volume_controller.get_process_names())

    def connect_serial(self):
        """Start reading from the selected serial port."""
        port = self.dropdown3.currentText()
        if port:
            self.serial_thread = SerialReader(port)
            self.serial_thread.data_received.connect(self.handle_serial_data)
            self.serial_thread.start()
            self.connect_button.setText("Disconnect")
            self.connect_button.clicked.disconnect()
            self.connect_button.clicked.connect(self.disconnect_serial)

    def disconnect_serial(self):
        """Stop reading from the serial port."""
        if self.serial_thread:
            self.serial_thread.stop()
            self.serial_thread.wait()
            self.serial_thread = None
            self.connect_button.setText("Connect")
            self.connect_button.clicked.disconnect()
            self.connect_button.clicked.connect(self.connect_serial)

    def handle_serial_data(self, data):
        """Handle data received from the serial port."""
        try:
            mat = re.search(r"(\d+)", data)
            if mat:
                print(data)
                value = int(mat.group(0))
                value = map_value(value, 0, 360, 0, 100) 
                self.dial.setValue(int(value))
        except ValueError:
            print(f"Invalid data received: {data}")

    def update_label(self, value):
        """Update the label with the dial's current value."""
        self.dial_label.setText(f"Volume: {value}%")
        d = {"process": self.dropdown1.currentText(), "volume": value}
        self.web_app_thread.trans_data.emit(d)
        self.volume_controller.set_volume(value / 100)

# Run the application
if __name__ == "__main__":
    app = QApplication(sys.argv)
    gui = SimpleGui()
    gui.show()
    sys.exit(app.exec())
