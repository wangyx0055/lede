@echo off
rem for /f "delims=" %%x in (%1) do @echo %%x 
if exist a.txt del a.txt
sigcheck -i -q %1 >a.txt
for /f "skip=1 tokens=1,2" %%b in (a.txt) do (
    if "%%b"=="Verified:" (if "%%c"=="Unsigned"  goto UNSIGN  )
    if "%%b"=="Catalog:"  (goto SIGNERS )
      )

:SIGNERS

for /f "skip=2 tokens=1,1" %%d in (a.txt) do (
    if "%%d"=="Signers:"  (goto OWENS )
      )

:OWENS
for /f "skip=3 tokens=1,1" %%e in (a.txt) do (
    if "%%e"=="Microsoft"  (set owen="Microsoft Signed (logo driver)". & goto SHOWOWENS )
    if "%%e"=="Winqual"    (set owen="Winqual OEM Signed (Test Sign)". & goto SHOWOWENS )
    if "%%e"=="Mediatek"   (set owen="Mediatek Signed".                & goto SHOWOWENS )
    if "%%e"=="Ralink"     (set owen="Ralink Signed".                  & goto SHOWOWENS )
    echo %1 is %%e Signed.
    goto END
      )

:UNSIGN
echo %1 is "Unsigned".
goto END

:SHOWOWENS
echo %1 is %owen%
goto END

:END
if exist a.txt del a.txt
set owen=
rem @echo on
