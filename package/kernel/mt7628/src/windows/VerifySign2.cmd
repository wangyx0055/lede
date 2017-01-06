@echo off
echo VerifySign2 2013/05/24
rem for /f "delims=" %%x in (%1) do @echo %%x 
if exist a.txt del a.txt
signtool.exe verify /v /pa %1 >a.txt
for /f "tokens=1,3" %%b in (a.txt) do (
    if "%%b"=="Issued"     (
        if "%%c"=="Winqual"    (set owen="Winqual OEM Signed (Test Sign)". & goto SHOWOWENS )
        if "%%c"=="Mediatek"   (set owen="Mediatek Signed".                & goto SHOWOWENS )
        if "%%c"=="Ralink"     (set owen="Ralink Signed".                  & goto SHOWOWENS )
        if "%%c"=="Microsoft"  (set owen="Microsoft Signed (logo driver)". & goto SIGNERS   )
        if "%%c"=="MSIT"       (set owen="Winqual OEM Signed (Test Sign)"  & goto SIGNERS   )
        if "%%c"=="VeriSign"   (set owen="0" )else (
             echo %1 is %%c Signed.
             )
        )
	
    )
for /f "skip=3"             %%e in (a.txt) do ( if "%%e"=="Number"     ( goto UNSIGN    )) 
goto END

:SIGNERS

for /f "tokens=4" %%d in (a.txt) do (
    if "%%d"=="OEM"        (set owen="Winqual OEM Signed (Test Sign)". & goto SHOWOWENS )
    if "%%d"=="Windows"    (set owen="Microsoft Signed (logo driver)". & goto SHOWOWENS )
      )
    echo err2_%%d
    goto END

:UNSIGN
echo %1 is "Unsigned !!!".
echo XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
goto END

:SHOWOWENS
echo %1 is %owen%
goto END

:END
if exist a.txt del a.txt
set owen=
rem @echo on
