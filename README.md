# micropython-raspberrypi

MicroPython on bare metal Raspberry Pi Zero

derived from https://github.com/micropython/micropython/pull/3522 on December 30, 2017.

## How to build

1. git clone https://github.com/boochow/micropython-raspberrypi.git
1. cd micropython-raspberrypi
1. git submodule update --init
1. cd micropython; git submodule update --init; cd ..
1. cd raspberrypi; make

## How to install

1. download bootcode.bin and start.elf from https://github.com/raspberrypi/firmware/tree/master/boot
1. copy bootcode.bin and start.elf to root of your microSD card
1. copy build/config.txt and build/firmware.img to root of your microSD card
