<p align="center">
    <img src="/images/esPINDMD_logo.png">
</p>

esPINDMD is a clock and GIF player combination for playing Pinball animations.

## Features

As of now, not all of these features are implemented.

esPINDMD supports the following features:

- Support for ESP32 and ESP32-S3
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

## Configuration

The configuration is stored as `config.json` on the SD card, examples for all supported platforms can be found in the [configurations](configurations) directory.

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
