@echo off
set P=D:\AVR-GCC\bin\
%P%avrdude.exe -C %P%avrdude.conf -p attiny13 -c stk500v1 -P COM4 -b 19200 -e -U hfuse:w:0xff:m -U lfuse:w:0x6a:m
pause
