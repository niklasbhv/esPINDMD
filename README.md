<p align="center">
    <img src="/images/esPINDMD_logo.png">
</p>

# esPINDMD

The esPINDMD is a clock and GIF player combination for playing Pinball animations.

## Features

As of now, not all of these features are implemented.

esPINDMD supports the following features:

- Support for ESP32, ESP32-S2 and ESP32-S3
- Easy to assemble and setup
- Sequential or indexed based GIF file browsing
- Wi-Fi and Ethernet support
- Support for MQTT including Home Assistant auto discovery
- Support for over the air updates
- Support for additional sensors via STEMMA QT / QWIC

## Setup the development environment

To setup this project, a git patch has to be applied before building using the following command from the root directory of the project.

```
bash scripts/apply_patches.sh
```

Afterwards, the project can be build like a normal platform.io project.

## Setup the esPINDMD

The setup process will involve connecting the esPINDMD to a network and loading it up with GIFs for displaying.

## Hardware

The esPINDMD is designed around but not limited to the Adafruit Metro lineup as well as the Sparkfun IoT RedBoard lineup. For the respective version of the ESP32, the following boards are officially supported:

- Sparkfun IoT RedBoard ESP32
- Adafruit Metro ESP32-S2
- Adafruit Metro ESP32-S3

The following pinouts are designed to be used with these boards as well as the Adafruit RGB Matrix Shield. Every other board will have the default column for its pin definitions.

### Matrix Pinout

| HUB75 | ESP32  | ESP32-S2 | ESP32-S3 | Default |
|:-----:|:------:|:--------:|:--------:|:-------:|
| R1    | GPIO4  | GPIO7    | GPIO2    |         |
| G1    | GPIO13 | GPIO8    | GPIO3    |         |
| B1    | GPIO14 | GPIO9    | GPIO4    |         |
| R2    | GPIO16 | GPIO10   | GPIO5    |         |
| G2    | GPIO17 | GPIO11   | GPIO6    |         |
| B2    | GPIO25 | GPIO12   | GPIO7    |         |
| E     | NC     | NC       | NC       | NC      |
| A     | GPIO33 | GPIO17   | GPIO14   |         |
| B     | GPIO32 | GPIO18   | GPIO15   |         |
| C     | Broken | GPIO1    | GPIO16   |         |
| D     | Broken | GPIO2    | GPIO17   |         |
| CLK   | GPIO26 | GPIO13   | GPIO8    |         |
| LAT   | GPIO5  | GPIO15   | GPIO10   |         |
| OE    | GPIO27 | GPIO14   | GPIO9    |         |

### SD Reader Pinout

| SD Reader | ESP32  | ESP32-S2 | ESP32-S3 | Default |
|:---------:|:------:|:--------:|:--------:|:-------:|
| MISO      | GPIO23 | N/A      | GPIO6    |         |
| MOSI      | GPIO19 | N/A      | GPIO5    |         |
| CLK       | GPIO18 | N/A      | GPIO7    |         |
| CS        | GPIO5  | N/A      | GPIO4    |         |

## License

This project is licensed under the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).
