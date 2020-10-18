@echo off
del %~n0.tmp 2>nul
(echo s
echo e
<nul set /p P=w
type %~n0.hex
echo q)>>%~n0.tmp
lineterm COM4 115200 < %~n0.tmp
del %~n0.tmp
