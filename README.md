# micropython-raspberrypi

MicroPython on bare metal Raspberry Pi Zero / Zero W

derived from https://github.com/micropython/micropython/pull/3522 on December 30, 2017.

## How to build
```
git clone https://github.com/boochow/micropython-raspberrypi.git
cd micropython-raspberrypi
git submodule update --init
cd micropython; git submodule update --init; cd ..
cd raspberrypi; make
```
## How to install

1. download `bootcode.bin` and `start.elf` from https://github.com/raspberrypi/firmware/tree/master/boot
1. copy `bootcode.bin` and `start.elf` to root of your microSD card
1. copy `build/firmware.img` to root of your microSD card
1. edit `config.txt` on root of your microSD card and add a line `kernel=firmware.img`.

## Modules and Classes

See [wiki](https://github.com/boochow/micropython-raspberrypi/wiki).

## Credits

csud USB host driver by Alex Chadwick. (I modified the original [Chadderz121/csud: Chadderz's Simple USB Driver for Raspberry Pi](https://github.com/Chadderz121/csud) to support RPi zero/zero W. The modified version is [here](https://github.com/boochow/csud)).

sd.c SD card driver by Zoltan Baldaszti. ([raspi3\-tutorial/0B\_readsector at master · bztsrc/raspi3\-tutorial](https://github.com/bztsrc/raspi3-tutorial/tree/master/0B_readsector))

A lot of bare metal examples by David Welch. ([dwelch67/raspberrypi: Raspberry Pi ARM based bare metal examples](https://github.com/dwelch67/raspberrypi))