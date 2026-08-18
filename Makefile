.PHONY: clean compile upload monitor

BOARD = esp32:esp32:XIAO_ESP32S3
BAUD = 115200
USB_PORT = /dev/ttyACM0
ARDUINO_DIR = ./arduino

compile: arduino/*.ino
	arduino-cli compile --fqbn $(BOARD) $(ARDUINO_DIR) --build-property build.extra_flags="-DESP32 -D__ESP32__"

upload:
	arduino-cli upload -p $(USB_PORT) --fqbn $(BOARD) $(ARDUINO_DIR)

monitor:
	picocom $(USB_PORT) -b $(BAUD) --imap lfcrlf
