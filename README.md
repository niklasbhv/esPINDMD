<p align="center">
    <img src="/images/esPINDMD_logo.png">
</p>

esPINDMD is a clock and GIF player combination for playing Pinball animations.

## Features

esPINDMD supports the following features:

- Support for ESP32 and ESP32-S3 based boards
- Easy to assemble and setup
- Random or sequential GIF playback
- Wi-Fi support including support for a fully local setup
- Support for MQTT including Home Assistant auto discovery
- Support for over the air updates compatible with Home Assistant

## Setup the development environment

The project is a regular [PlatformIO](https://platformio.org/) project. On Nix, `nix-shell` provides PlatformIO along with all other tools used by this project.

```
pio run -e adafruit_metro_esp32s3                 # build the firmware
pio run -e adafruit_metro_esp32s3 -t upload       # build and flash the firmware
pio device monitor                                # open the serial console
```

The available environments are `esp32dev`, `sparkfun_esp32_iot_redboard`, `esp32-s3-devkitm-1` and `adafruit_metro_esp32s3`. The code is formatted using `make format`.

## Setup the esPINDMD

The esPINDMD can be set up in two ways, which can also be combined:

- **Wi-Fi setup portal**: Without a configured network, the esPINDMD opens the Wi-Fi network `esPINDMD` (password `pinball12345`) for 180 seconds and shows its details on the matrix. Connect to it and open `http://192.168.4.1` to select a network.
- **Serial console**: The menu described below is available a few seconds after powering on, also while the setup portal is open. This includes the Wi-Fi credentials (`network/wifi`) as well as the pins of the matrix and the SD card.

GIFs are played from the `gif` directory of the SD card, including all sub directories. On every boot, the esPINDMD creates an index of all GIFs (`gif/.index`), which is used to play them in random order. Setting `application/gif/random` to `off` plays them in the order of the index instead. Without a SD card, the esPINDMD runs as a clock only.

## Configuration

The configuration is stored in the internal flash of the esPINDMD, so it is available without a SD card. A `config.json` on the SD card is imported on every boot and overrides the stored configuration, which allows editing it on a computer. Saving the configuration also updates the `config.json`. Examples for all supported platforms can be found in the [configurations](configurations) directory.

The configuration can also be changed using the menu on the serial console (115200 baud), e.g. using `pio device monitor`. Press Enter to open the menu, its structure follows the structure of the configuration file:

```
== esPINDMD configuration: /display/matrix ==
   1) panel/
   2) pins/
   3) brightness               90

  b) back  s) save  r) reboot  i) info  h) help  q) quit
esPINDMD:/display/matrix> 3
Brightness of the matrix
Current value: 90, allowed: 0-255
Enter the new value, an empty line keeps the current one
brightness> 120
```

Entries are selected by their number. Changes to the colour of the clock and the brightness are applied immediately, other changes take effect after saving (`s`) and rebooting (`r`). Besides the settings, the menu offers actions to reset the Wi-Fi credentials (`network/wifi/reset`) and to check for and install firmware updates (`update/check`, `update/install`).

## Firmware Updates

The esPINDMD checks for new firmware on boot and every few hours. If MQTT is enabled, the firmware shows up as an update entity in Home Assistant, which installs the update on request. The progress is shown on the matrix as well as in Home Assistant.

By default, the latest [GitHub release](https://github.com/niklasbhv/esPINDMD/releases) is used. Releases are created by pushing a version tag, e.g. `v0.1.0`, which builds the firmware for all platforms and publishes it together with a `manifest.json`.

To update from a local webserver instead, download the `manifest.json` and the `esPINDMD-*.bin` files of a release into a directory served by the webserver and set the URL of the manifest in the configuration:

```json
"update": {
  "url": "http://192.168.1.10/espindmd/manifest.json",
  "interval": 6
}
```

The `interval` sets the hours between two checks, `0` only checks on boot. A new firmware has to connect to the network (and to MQTT if enabled) within 5 minutes after the update, otherwise the previous firmware is restored.

## Hardware

The esPINDMD is designed around but not limited to the Adafruit Metro lineup as well as the Sparkfun IoT RedBoard lineup. For the respective version of the ESP32, the following boards are officially supported:

- Sparkfun IoT RedBoard ESP32
- Adafruit Metro ESP32-S3

The following pinouts are designed to be used with these boards as well as the Adafruit RGB Matrix Shield. Every other board will have the default column for its pin definitions.

### Matrix Pinout

| HUB75 | ESP32  | ESP32-S3 | Default |
|:-----:|:------:|:--------:|:-------:|
| R1    | GPIO4  | GPIO2    |         |
| G1    | GPIO13 | GPIO3    |         |
| B1    | GPIO14 | GPIO4    |         |
| R2    | GPIO16 | GPIO5    |         |
| G2    | GPIO17 | GPIO6    |         |
| B2    | GPIO25 | GPIO7    |         |
| E     | NC     | NC       | NC      |
| A     | GPIO33 | GPIO14   |         |
| B     | GPIO32 | GPIO15   |         |
| C     | Broken | GPIO16   |         |
| D     | Broken | GPIO17   |         |
| CLK   | GPIO26 | GPIO8    |         |
| LAT   | GPIO5  | GPIO10   |         |
| OE    | GPIO27 | GPIO9    |         |

### SD Reader Pinout

| SD Reader | ESP32  | ESP32-S3 | Default |
|:---------:|:------:|:--------:|:-------:|
| MISO      | GPIO23 | GPIO6    |         |
| MOSI      | GPIO19 | GPIO5    |         |
| CLK       | GPIO18 | GPIO7    |         |
| CS        | GPIO5  | GPIO4    |         |

## License

This project is licensed under the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).
