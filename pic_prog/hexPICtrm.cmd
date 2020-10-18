@echo off
setlocal enabledelayedexpansion
echo HEX to TRM convertor script by @Semper 2018
echo Command line: hexPICtrm example.hex
if "%1"=="" echo ERROR in command line&pause&exit
if not exist %1 echo ERROR: not found file %1&pause&exit
(echo s
echo e
<nul set /p T=p
set /a B=0
set /a C=0
 for /f "tokens=*" %%X in (%1) do (
 set X=%%X
  if "!X:~7,2!"=="00" if "!X:~3,1!"=="0" (
  set /a A="0x!X:~3,4!"
  set /a B="(!A!-!B!)>>1"
  if !B! neq 0 for /l %%I in (1,1,!B!) do call :print 3FFF
  set /a B="0x00!X:~1,2!"
  set /a I="(!B!<<1)+5"
  for /l %%I in (9,4,!I!) do set J=!X:~%%I,4!&call :print !J:~2,2!!J:~0,2!
  set /a B="!A!+!B!"
  )
  if "!X:~1,8!"=="02400E00" set F=f7!X:~11,2!!X:~9,2!
 )
echo.&echo.
echo !F!
echo q
)>%~n1.trm
exit /B

:print
if !C! equ 0 echo.
<nul set /p T=%1
set /a C="(C+1)&15"
exit /B
