@echo off
if exist %~n0.obj del %~n0.obj
if exist %~n0.exe del %~n0.exe
\MASM32\bin\Ml.exe /c /coff /Fl /Sn %~n0.asm
if errorlevel 1 echo Assembly Error&pause&exit
\MASM32\bin\Link.exe /SUBSYSTEM:CONSOLE %~n0.obj
if errorlevel 1 echo Link Error&pause&exit
