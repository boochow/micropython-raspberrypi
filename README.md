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
1. edit `config.txt` on root of your microSD card and add a line `kernel=firmware.img`.

## Modules and Classes

See [wiki](https://github.com/boochow/micropython-raspberrypi/wiki).
