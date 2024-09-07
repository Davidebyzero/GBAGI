#!/bin/bash
set -x #echo on

CFLAGS="-mthumb-interwork -mthumb -mcpu=arm7tdmi -c"
LINKFLAGS="-Wl,--gc-sections --specs=nosys.specs -mthumb-interwork -mthumb -mcpu=arm7tdmi -nostartfiles -Wl,-Tcrtls/gcc-12-custom/gba_cart.ld -lc"

arm-none-eabi-as -mthumb-interwork -o crt0.o crtls/gcc-12-custom/crt0.s

arm-none-eabi-gcc -O3 $CFLAGS main.c
arm-none-eabi-gcc -O3 $CFLAGS agimain.c
arm-none-eabi-gcc -O3 $CFLAGS gamedata.c
arm-none-eabi-gcc -O3 $CFLAGS input.c
arm-none-eabi-gcc -O3 $CFLAGS keyboard.c
arm-none-eabi-gcc -O3 $CFLAGS invobj.c
arm-none-eabi-gcc -O3 $CFLAGS logic.c
arm-none-eabi-gcc -O3 $CFLAGS picture.c
arm-none-eabi-gcc -O2 $CFLAGS screen.c
arm-none-eabi-gcc -O3 $CFLAGS status.c
arm-none-eabi-gcc -O3 $CFLAGS variables.c
arm-none-eabi-gcc -O2 $CFLAGS views.c
arm-none-eabi-gcc -O3 $CFLAGS system.c
arm-none-eabi-gcc -O2 $CFLAGS commands.c
arm-none-eabi-gcc -O2 $CFLAGS cmdagi.c
arm-none-eabi-gcc -O3 $CFLAGS cmdtest.c
arm-none-eabi-gcc -O3 $CFLAGS errmsg.c
arm-none-eabi-gcc -O2 $CFLAGS text.c
arm-none-eabi-gcc -O3 $CFLAGS menu.c
arm-none-eabi-gcc -O2 $CFLAGS wingui.c
arm-none-eabi-gcc -O3 $CFLAGS parse.c
arm-none-eabi-gcc -O2 $CFLAGS saverestore.c
arm-none-eabi-gcc -O3 $CFLAGS interrupts.c 

arm-none-eabi-gcc $LINKFLAGS crt0.o main.o agimain.o gamedata.o input.o keyboard.o invobj.o logic.o picture.o screen.o status.o variables.o views.o system.o commands.o cmdagi.o cmdtest.o errmsg.o text.o menu.o wingui.o parse.o saverestore.o interrupts.o 

arm-none-eabi-objcopy -O binary a.out gbagi.bin
