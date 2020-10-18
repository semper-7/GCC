@echo off
"C:\Program Files\Microchip\MPASM Suite\MPASMWIN.exe" %~n0.asm -p12F635
if errorlevel 1 echo Assembly Error&exit
hexPICtrm %~n0.hex
