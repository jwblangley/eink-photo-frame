.PHONY: compile upload monitor format

BOARD = esp32:esp32:XIAO_ESP32S3:CDCOnBoot=default,FlashSize=8M,PSRAM=opi
BAUD = 115200
USB_PORT = /dev/ttyACM0
ARDUINO_DIR = ./arduino

WAVESHARE_DOWNLOAD_DIR = lib.tmp

./arduino/src/waveshare/10in2g/ESP32/ESP32.ino:
	mkdir --parents $(WAVESHARE_DOWNLOAD_DIR)
	curl 'https://files.waveshare.com/wiki/10.2inch%20e-Paper%20HAT%20(G)/10in2_e-Paper_G.zip' >$(WAVESHARE_DOWNLOAD_DIR)/10.2inch_e-Paper_G.zip
	unzip $(WAVESHARE_DOWNLOAD_DIR)/10.2inch_e-Paper_G.zip "ESP32/*" -d ./arduino/src/waveshare/10in2g/
	rm -rf $(WAVESHARE_DOWNLOAD_DIR)

./arduino/src/waveshare/10in2g/ESP32/.patched: ./arduino/src/waveshare/10in2g/ESP32/ESP32.ino
	patch ./arduino/src/waveshare/10in2g/ESP32/DEV_Config.h ./DEV_config.h.patch
	touch ./arduino/src/waveshare/10in2g/ESP32/.patched

compile: ./arduino/src/waveshare/10in2g/ESP32/.patched
	arduino-cli compile --fqbn $(BOARD) $(ARDUINO_DIR)

upload:
	arduino-cli upload -p $(USB_PORT) --fqbn $(BOARD) $(ARDUINO_DIR)

monitor:
	arduino-cli monitor -p $(USB_PORT) --config baudrate=$(BAUD) --config dtr=on --config rts=on

format:
	find $(ARDUINO_DIR) -type f | xargs clang-format -i
