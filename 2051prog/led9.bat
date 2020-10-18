@echo off
setlocal enabledelayedexpansion
ASM51.exe %~n0.asm
echo Conversion from hex to trm-file ,,,
(echo se
<nul set /p T=p
set /a B=0
set /a C=0
 for /f "tokens=*" %%X in (%~n0.hex) do (
 set X=%%X
  if "!X:~7,2!"=="00" if "!X:~3,1!"=="0" (
  set /a A="0x!X:~3,4!"
  set /a B="!A!-!B!"
  if !B! neq 0 for /l %%I in (1,1,!B!) do call :print FF
  set /a B="0x00!X:~1,2!"
  set /a I="(!B!<<1)+7"
  for /l %%I in (9,2,!I!) do call :print !X:~%%I,2!
  set /a B="!A!+!B!"
 ))
echo $q
)>%~n0.trm
echo %~n0.trm successfully created.
pause&exit

:print
if !C! equ 0 echo.
<nul set /p T=%1
set /a C="(C+1)&15"
exit /b
