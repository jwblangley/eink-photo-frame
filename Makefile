.PHONY: compile upload monitor format

BOARD = esp32:esp32:XIAO_ESP32S3:CDCOnBoot=default,FlashSize=8M,PSRAM=opi
BAUD = 115200
USB_PORT = /dev/ttyACM0
ARDUINO_DIR = ./arduino

compile: arduino/*.ino
	arduino-cli compile --fqbn $(BOARD) $(ARDUINO_DIR)

upload:
	arduino-cli upload -p $(USB_PORT) --fqbn $(BOARD) $(ARDUINO_DIR)

monitor:
	arduino-cli monitor -p $(USB_PORT) --config baudrate=$(BAUD) --config dtr=on --config rts=on

format:
	find $(ARDUINO_DIR) -type f | xargs clang-format -i
