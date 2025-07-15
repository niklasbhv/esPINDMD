<p align="center">
    <img src="/images/esPINDMD_logo.png">
</p>

# esPINDMD

The esPINDMD is a clock and GIF player combination for playing Pinball animations.

## Features

As of now, not all of these features are implemented.

### Support for ESP32, ESP32-S3

### Wi-Fi and Ethernet support

### MQTT Integration

The esPINDMD integrates a MQTT client for communication with a MQTT broker, including support for Home Assistant auto discovery.

## Setup

To setup this project, a git patch has to be applied before building using the following command from the root directory of the project.

```
bash scripts/apply_patches.sh
```

Afterwards, the project can be build like a normal platform.io project.

## Hardware

### Matrix Pinout

| HUB75 | ESP32  | ESP32-S3 |
|:-----:|:------:|:--------:|
| R1    |        | GPIO36   |
| G1    |        | GPIO37   |
| B1    |        | GPIO38   |
| R2    |        | GPIO40   |
| G2    |        | GPIO41   |
| B2    |        | GPIO42   |
| E     | Unused | Unused   |
| A     |        | GPIO0    |
| B     |        | GPIO1    |
| C     |        | GPIO2    |
| D     |        | GPIO3    |
| CLK   |        | GPIO15   |
| LAT   |        | GPIO18   |
| OE    |        | GPIO16   |

### SD Reader Pinout

| SD Reader | ESP32 | ESP32-S3 |
|:---------:|:-----:|:--------:|
| MISO      |       | GPIO6    |
| MOSI      |       | GPIO5    |
| CLK       |       | GPIO7    |
| CS        |       | GPIO4    |

### Ethernet Controller Pinpout

| Ethernet Controller | ESP32 | ESP32-S3 |
|:-------------------:|:-----:|:--------:|
| MISO                |       | GPIO11   |
| MOSI                |       | GPIO12   |
| CLK                 |       | GPIO13   |
| CS                  |       | GPIO14   |
| INT                 |       | GPIO10   |
| RST                 |       | GPIO9    |

## License

This project is licensed under the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).
