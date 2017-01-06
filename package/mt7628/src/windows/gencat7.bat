@echo off
if "%1" == "" goto help

echo.
echo Generate Win7 X86 Cat file
echo ============================
echo inf2cat [Path]
echo where 
echo 1.)[Path] Your driver path include "sys" "inf".
echo. 

inf2cat /driver:%1 /os:7_X86 /verbose 

goto exit
:help
echo.
echo Generate Win7 X86 Cat file
echo ============================
echo inf2cat [Path]
echo where 
echo 1.)[Path] Your driver path include "sys" "inf".
echo. 

:exit