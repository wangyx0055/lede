
:ROOT Directory
@echo off
:: Directory Removal
for %%a in (buildchk, buildfre, objchk, objfre) do (
    for %%b in (wxp, wnet, wlh, vista, win7, win8, Win8.1, WinBlue) do (
        for %%c in (x86, x64, amd64, ia64) do (
            if exist %%a_%%b_%%c    rmdir /Q /S %%a_%%b_%%c
            if exist %%a_%%b_%%c.*  del %%a_%%b_%%c.*
        )
    )
)

:: File Removal
for %%a in (buildchk, buildfre) do (
    for %%b in (wxp, wnet, wlh, vista, win7, win8, Win8.1, WinBlue) do (
        for %%c in (x86, x64, amd64, ia64) do (
            for %%d in (log, err, prf, wrn) do (
                if exist %%a_%%b_%%c.%%d    del %%a_%%b_%%c.%%d
            )
        )
    )
)

if exist windows.msbuild.sdf    del windows.msbuild.sdf

echo ROOT Directory: DONE!
echo on

:COMMON Directory
@echo off
:: Directory Removal
set TARGET_DIR=Common
for %%a in (i386, x64, amd64, ia64, obj, objchk, objfre, Debug, Release, x86) do (
    if exist %TARGET_DIR%\%%a   rmdir /Q /S %TARGET_DIR%\%%a
)

for %%a in (objchk, objfre) do (
    for %%b in (wxp, wnet, wlh, vista, win7, win8, Win8.1, WinBlue) do (
        for %%c in (x86, x64, amd64, ia64) do (
            if exist %TARGET_DIR%\%%a_%%b_%%c   rmdir /Q /S %TARGET_DIR%\%%a_%%b_%%c
        )
    )
)

for %%a in (Vista, Win7, Win8, WinBlue, Win8.1) do (
    for %%b in (Debug, Release) do (
        if exist %TARGET_DIR%\%%a%%b    rmdir /Q /S %TARGET_DIR%\%%a%%b
    )
)

echo COMMON Directory: DONE!
echo on

:HAL Directory
@echo off
:: Directory Removal
for %%t in (HAL\common, HAL\mt7603, HAL\mt6630) do (
    for %%a in (i386, x64, amd64, ia64, obj, objchk, objfre, Debug, Release,x86) do (
        if exist %%t\%%a    rmdir /Q /S %%t\%%a
    )

    for %%a in (objchk, objfre) do (
        for %%b in (wxp, wnet, wlh, vista, win7, win8, Win8.1, WinBlue) do (
            for %%c in (x86, x64, amd64, ia64) do (
                if exist %%t\%%a_%%b_%%c    rmdir /Q /S %%t\%%a_%%b_%%c
            )
        )
    )
    
    for %%a in (Vista, Win7, Win8, WinBlue, Win8.1) do (
        for %%b in (Debug, Release) do (
            if exist %%t\%%a%%b rmdir /Q /S %%t\%%a%%b
        )
    )
)


echo HAL Directory: DONE!
echo on

:Platform Directory
@echo off
:: Directory Removal
for %%t in (PLATFORM\NDIS6, PLATFORM\NDIS5, PLATFORM\NDIS6\USB, PLATFORM\NDIS6\PCI, PLATFORM\NDIS6\SDIO, PLATFORM\NDIS5\USB, PLATFORM\NDIS5\SDIO, PLATFORM\NDIS5\SDIO) do (
    for %%a in (i386, x64, amd64, ia64, obj, objchk, objfre, Debug, Release, x86) do (
        if exist %%t\%%a    rmdir /Q /S %%t\%%a
        if exist %%t\dirs-Package rmdir /Q /S %%t\dirs-Package
    )

    for %%a in (objchk, objfre) do (
        for %%b in (wxp, wnet, wlh, vista, win7, win8, Win8.1, WinBlue) do (
            for %%c in (x86, x64, amd64, ia64) do (
                if exist %%t\%%a_%%b_%%c    rmdir /Q /S %%t\%%a_%%b_%%c
            )
        )
    )

    for %%a in (Vista, Win7, Win8, WinBlue, Win8.1) do (
        for %%b in (Debug, Release) do (
            if exist %%t\%%a%%b rmdir /Q /S %%t\%%a%%b
        )
    )
)

echo Platform Directory: DONE!
echo on

:dirs-Package Directory
@echo off
for %%t in (dirs-Package) do (
    for %%a in (Vista, Win7, Win8, WinBlue, Win8.1) do (
        for %%b in (Debug, Release) do (
            if exist %%t\%%a%%b rmdir /Q /S %%t\%%a%%b
        )
    )
            for %%c in (x86, x64, amd64, ia64) do (
                if exist %%t\%%c    rmdir /Q /S %%t\%%c
            )   
)


echo dirs-Package Directory: DONE!

Copy Doc\History.txt %SRC_ROOT%
Call CurrentVerOnHistory.exe
del History.txt

del msbuild.err
del msbuild.wrn

FOR /F  %%i IN (hisver.txt) DO set Ver=%%i
set VerA=%Ver%_%1
set VerB=%Ver%

if exist MTKUsb_%VerA% rmdir /Q /S MTKUsb_%VerA%
if exist MTKUsb_%VerA%_Symbol rmdir /Q /S MTKUsb_%VerA%_Symbol

if exist MTKUsb_%VerB% rmdir /Q /S MTKUsb_%VerB%
if exist MTKUsb_%VerB%_Symbol rmdir /Q /S MTKUsb_%VerB%_Symbol

if exist hisver.txt del hisver.txt

if exist x86 rmdir /Q /S x86
if exist x64 rmdir /Q /S x64
for %%a in (Vista, Win7, Win8, WinBlue, Win8.1) do (
    for %%b in (Debug, Release) do (
        if exist %%a%%b rmdir /Q /S %%a%%b
    )
)

cls 
powershell write-host "`n----------- Clean finished -----------" -foregroundcolor Magenta
:powershell write-host "`n----------- Clean finished -----------" -foregroundcolor Cyan
:powershell write-host "`n----------- Clean finished -----------" -foregroundcolor Red
:powershell write-host "`n----------- Clean finished -----------" -foregroundcolor DarkYellow
:powershell write-host "`n----------- Clean finished -----------" -foregroundcolor Yellow
:powershell write-host "`n----------- Clean finished -----------" -foregroundcolor Blue
:powershell write-host "`n----------- Clean finished -----------" -foregroundcolor DarkGray
:powershell write-host "`n----------- Clean finished -----------" -foregroundcolor DarkRed
echo on