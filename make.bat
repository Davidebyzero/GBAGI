@echo on
set path=C:\DevKitAdv\bin

del gbagi.elf
del gbagi.bin

as -mthumb-interwork    -o crt0.o crt0.s

gcc -c -O3 -mthumb -mthumb-interwork    -o main.o main.c
gcc -c -O3 -mthumb -mthumb-interwork    -o agimain.o agimain.c
gcc -c -O3 -mthumb -mthumb-interwork    -o gamedata.o gamedata.c
gcc -c -O3 -mthumb -mthumb-interwork    -o input.o input.c
gcc -c -O3 -mthumb -mthumb-interwork    -o keyboard.o keyboard.c
gcc -c -O3 -mthumb -mthumb-interwork    -o invobj.o invobj.c
gcc -c -O3 -mthumb -mthumb-interwork    -o logic.o logic.c
gcc -c -O3 -mthumb -mthumb-interwork    -o picture.o picture.c
gcc -c -O3 -mthumb -mthumb-interwork    -o screen.o screen.c
gcc -c -O3 -mthumb -mthumb-interwork    -o status.o status.c
gcc -c -O3 -mthumb -mthumb-interwork    -o variables.o variables.c
gcc -c -O3 -mthumb -mthumb-interwork    -o views.o views.c
gcc -c -O3 -mthumb -mthumb-interwork    -o system.o system.c
gcc -c -O3 -mthumb -mthumb-interwork    -o commands.o commands.c
gcc -c -O3 -mthumb -mthumb-interwork    -o cmdagi.o cmdagi.c
gcc -c -O3 -mthumb -mthumb-interwork    -o cmdtest.o cmdtest.c
gcc -c -O3 -mthumb -mthumb-interwork    -o errmsg.o errmsg.c
gcc -c -O2 -mthumb -mthumb-interwork    -o text.o text.c
gcc -c -O3 -mthumb -mthumb-interwork    -o menu.o menu.c
gcc -c -O3 -mthumb -mthumb-interwork    -o wingui.o wingui.c
gcc -c -O3 -mthumb -mthumb-interwork    -o parse.o parse.c
gcc -c -O3 -mthumb -mthumb-interwork    -o saverestore.o saverestore.c
gcc -c -O3 -mthumb -mthumb-interwork    -o interrupts.o interrupts.c


gcc -nostartfiles -Wl,-Tlnkscript  -mthumb -mthumb-interwork  -o test.elf crt0.o main.o interrupts.o agimain.o gamedata.o input.o keyboard.o invobj.o logic.o picture.o screen.o status.o variables.o views.o system.o commands.o cmdagi.o cmdtest.o errmsg.o text.o menu.o parse.o saverestore.o wingui.o
objcopy -O binary test.elf gbagi.bin
cd gbarom
.\aginject
cd ..
