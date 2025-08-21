# esPINDMD Extension Shield

The esPINDMD Extension Shield is an extension board for the Adafruit RGB Matrix Shield. It enables the following features for the esPINDMD:

- Support for SD as well as micro SD cards
- Logic Level Shifters for reliable operation
- Support for 64x64 matrices
- Support for Ethernet
- Support for a real time clock
- Support for adressable LEDs

This full set of features is only available when using the Adafruit Metro ESP32-S2 or the Adafruit Metro ESP32-S3 board.
The Sparkfun IoT RedBoard ESP32 is missing the support for Ethernet as well as the support for sd and micro sd cards on the extension board due to limited IO capabilities.
The board still features a onboard micro sd slot for loading GIF files.

The esPINDMD can be build without this extension board, if you do not care about the above features.
Do note that you'll be limited in your choice of board without the extension shield. The Adafruit Metro ESP32-S3 is recommended for this scenario.
The other boards are limited:

- The Sparkfun IoT RedBoard ESP32 does not work with the Adafruit RGB Matrix Shield out of the box as some pins will have to be rerouted
- The Adafruit Metro ESP32-S2 does not feature a onboard sd card slot so you'll have to add your own

The extension board is currently in a alpha development phase at best so do expect the features to be incomplete and to change over time.

## Pinout

### Matrix Pinout

| HUB75 | ESP32  | ESP32-S2 | ESP32-S3 |
|:-----:|:------:|:--------:|:--------:|
| R1    | GPIO4  | GPIO7    | GPIO2    |
| G1    | GPIO13 | GPIO8    | GPIO3    |
| B1    | GPIO14 | GPIO9    | GPIO4    |
| R2    | GPIO16 | GPIO10   | GPIO5    |
| G2    | GPIO17 | GPIO11   | GPIO6    |
| B2    | GPIO25 | GPIO12   | GPIO7    |
| E     | GPIO3  | GPIO5    | GPIO0    |
| A     | GPIO18 | GPIO42   | GPIO13   |
| B     | GPIO19 | GPIO21   | GPIO12   |
| C     | GPIO23 | GPIO16   | GPIO11   |
| D     | GPIO1  | GPIO6    | GPIO1    |
| CLK   | GPIO26 | GPIO13   | GPIO8    |
| LAT   | GPIO5  | GPIO15   | GPIO10   |
| OE    | GPIO27 | GPIO14   | GPIO9    |

### SD Reader Pinout

| SD Reader | ESP32 | ESP32-S2 | ESP32-S3 |
|:---------:|:-----:|:--------:|:--------:|
| MISO      | N/A   | GPIO37   | GPIO21   |
| MOSI      | N/A   | GPIO35   | GPIO42   |
| CLK       | N/A   | GPIO36   | GPIO39   |
| CS        | N/A   | GPIO17   | GPIO14   |

### Real Time Clock Pinout

| RTC | ESP32  | ESP32-S2 | ESP32-S3 |
|:---:|:------:|:--------:|:--------:|
| SDA | GPIO21 | GPIO33   | GPIO47   |
| SCL | GPIO22 | GPIO34   | GPIO48   |

### Ethernet Controller Pinpout

| Ethernet Controller | ESP32 | ESP32-S2 | ESP32-S3 |
|:-------------------:|:-----:|:--------:|:--------:|
| MISO                | N/A   | GPIO4    | GPIO1    |
| MOSI                | N/A   | GPIO3    | GPIO18   |
| CLK                 | N/A   | GPIO2    | GPIO17   |
| CS                  | N/A   | GPIO1    | GPIO16   |
| INT                 | N/A   | GPIO34   | GPIO48   |
| RST                 | N/A   | GPIO33   | GPIO47   |
