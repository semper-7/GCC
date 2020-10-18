@echo off
set PD="C:\Program Files\PonyProg2000"
(echo SELECTDEVICE attiny13
echo LOAD-PROG %~n0.hex
echo WRITE-PROG
echo VERIFY-PROG
)>%~n0.e2s
%PD%\PonyProg2000.exe %~n0.e2s
(echo SELECTDEVICE attiny13
)>%~n0.e2s
%PD%\PonyProg2000.exe %~n0.e2s
pause
