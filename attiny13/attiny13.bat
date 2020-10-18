@echo off
setlocal enabledelayedexpansion
echo Compile %~n0.cpp ...
set P=D:\AVR-GCC\bin\
set CP=%P%avr-gcc
set AR=%P%avr-ar
set OP=-Wall -mmcu=attiny13 -Os -DF_CPU=128000
%CP% %OP% -c %~n0.c
if %errorlevel% NEQ 0 pause&exit
echo Linking ...
%CP% %OP% -o %~n0.elf %~n0.o
if %errorlevel% NEQ 0 pause&exit
%P%avr-objdump -h -S %~n0.elf > %~n0.lst
%P%avr-objcopy -O ihex %~n0.elf %~n0.hex
del *.o *.elf >nul
pause
