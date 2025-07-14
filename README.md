# esPINDMD

## Setup

To setup this project, a git patch has to be applied before building using the following command from the root directory of the project.

```
bash scripts/apply_patches.sh
```

Afterwards, the project can be build like a normal platform.io project.

## Hardware

### Matrix Pinout

| HUB75 | ESP32 | ESP32-S2 | ESP32-S3 | 
|:-----:|:-----:|:--------:|:--------:|
|   R1  |       |          |          |
|   G1  |       |          |          |
|   B1  |       |          |          |
|   R2  |       |          |          |
|   G2  |       |          |          |
|   B2  |       |          |          |
|   E   |       |          |          |
|   A   |       |          |          |
|   B   |       |          |          |
|   C   |       |          |          |
|   D   |       |          |          |
|  CLK  |       |          |          |
|  LAT  |       |          |          |
|   OE  |       |          |          |

## License

This project is licensed under the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0).
