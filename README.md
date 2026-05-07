# [GeoMCU](https://github.com/NohPei/GeoMCU) Firmware

Arduino® firmware for running [GeoMCU](https://github.com/NohPei/GeoMCU) receiver boards.

In-depth documentation is included with the [GeoMCU Documentation](https://geomcu.readthedocs.io)

## Building

This project is set up to be compiled and flashed using [PlatformIO](https://platformio.org/), whether through the IDE or the core CLI.

Building with the [Arduino IDE](https://www.arduino.cc/en/software) should be possible, but is unsupported.
Doing so would require manually installing libraries and some modifications to link with the included `lib` folder.

## Regarding time sync

- Added basic time sync functionality using [SNTP](https://en.wikipedia.org/wiki/Network_Time_Protocol#SNTP)
    - Polls an NTP specified in 'data/config/timesync/server'
    - Polling interval is specified in 'data/config/timesync/poll' in seconds

Copyright © 2024 The Regents of the University of Michigan, [PEI Lab](https://peizhang.engin.umich.edu/)
