@echo off
set C=COM4
set P=E:\AVR-GCC\bin\
"%P%\avrdude.exe" -C "%P%avrdude.conf" -p m328p -c arduino -P %C% -b 57600 -D -U flash:w:%~n0.hex:i
pause
