@echo off
del %~n0.tmp 2>nul
(echo s
echo e
<nul set /p P=p
type %~n0.hex
echo q) >>%~n0.tmp 
rem lineterm COM4 115200 < %~n0.tmp
