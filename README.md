# micropython-raspberrypi

MicroPython on bare metal Raspberry Pi Zero

derived from https://github.com/micropython/micropython/pull/3522 on December 30, 2017.

## How to build / install

1. cd raspberrypi; make
1. download bootcode.bin and start.elf from https://github.com/raspberrypi/firmware/tree/master/boot
1. copy bootcode.bin and start.elf to your microSD card
1. copy build/config.txt and build/firmware.img to your microSD card
