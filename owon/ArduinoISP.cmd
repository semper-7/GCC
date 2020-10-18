@echo off
set P=D:\AVR-GCC\bin\
%P%avrdude.exe -C %P%avrdude.conf -p m328p -c arduino -P COM4 -b 57600 -D -U flash:w:%~n0.ino.hex:i
pause
exit
rem 10 - Reset, 11 - MOSI, 12 - MISO, 13 - SCK.
rem attiny13 = 1,5,6,7
