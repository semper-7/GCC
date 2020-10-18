@echo off
setlocal enabledelayedexpansion
echo Compile %~n0.cpp ...
set P=D:\AVR-GCC\bin\
set CP=%P%avr-g++
set AR=%P%avr-ar
set OP=-Wall -mmcu=atmega328p -Os -DF_CPU=16000000
%CP% %OP% -c %~n0.cpp
if %errorlevel% NEQ 0 pause&exit
echo Compile LIBS ...
cd ..\lib
for %%I in (*.cpp) do set C=!C! %%I
%CP% %OP% -c !C!
if %errorlevel% NEQ 0 pause&exit
for %%I in (*.o) do set O=!O! %%I
%AR% rcs lib.a !O! 
if %errorlevel% NEQ 0 pause&exit
del *.o >nul
cd %~dp0
echo Linking ...
%CP% %OP% -o %~n0.elf %~n0.o ..\lib\lib.a
if %errorlevel% NEQ 0 pause&exit
%P%avr-objdump -h -S %~n0.elf > %~n0.lst
%P%avr-objcopy -O ihex %~n0.elf %~n0.hex
del *.o *.elf >nul
pause
