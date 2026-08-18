# Eink Photo Frame

ESP32-S3 eink photo frame

## Features

* Display photos on an eink photoframe from a collection of photos served over the network

## Running

### Microcontroller

This project is set up to use an XIAO ESP32-S3 development board configured with 24 pin eink paper display

Never connect your computer to the ESP32 board over USB when external power is still being supplied.
If you are using capacitors, ensure they are also discharged.

1. Follow <https://docs.arduino.cc/arduino-cli/getting-started> to get started - the board type is `esp32:esp32:XIAO_ESP32S3`
1. Install FastLED plugin
    ```bash
    arduino-cli lib update-index
    arduino-cli install "FastLED"
    ```
1. Run `make compile` to compile the arduino code
1. Run `make upload` to upload the compiled binary to the esp32 board. You may need to configure `USB_PORT` in the `Makefile` - this can be found using `arduino-cli board list`
1. To monitor the Serial output over USB, run `make monitor`
