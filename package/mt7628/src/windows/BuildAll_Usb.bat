IF "%1"=="free" goto BFREE
IF "%1"=="FREE" goto BFREE
IF "%1"=="Free" goto BFREE
IF "%1"=="fre" goto BFREE
IF "%1"=="Fre" goto BFREE
IF "%1"=="chk" goto BCHK
IF "%1"=="check" goto BCHK
IF "%1"=="CHECK" goto BCHK
IF "%1"=="Check" goto BCHK
IF "%1"=="CHK" goto BCHK
IF "%1"=="Chk" goto BCHK

:BALL
:WinBlue x86 Free
ECHO "WinBlue x86 Free"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=WinBlue x86 Free


:WinBlue x86 Chk
ECHO "WinBlue x86 Chk"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=WinBlue x86 Chk



:WinBlue x64 Free
ECHO "WinBlue x64 Free"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=WinBlue x64 Free


:WinBlue x64 Chk
ECHO "WinBlue x64 Chk"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=WinBlue x64 Chk



:Win8 x86 Free
ECHO "Win8 x86 Free"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=Win8 x86 Free



:Win8 x86 Chk
ECHO "Win8 x86 Chk"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=Win8 x86 Chk


:Win8 x64 Free
ECHO "Win8 x64 Free"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=Win8 x64 Free


:Win8 x64 Chk
ECHO "Win8 x86 Chk
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=Win8 x64 Chk


:Win7 x86 Free
ECHO "Win7 x86 Free"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=Win7 x86 Free


:Win7 x86 Chk
ECHO "Win7 x86 Chk"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=Win7 x86 Chk


:Win7 x64 Free
ECHO "Win7 x64 Free"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=Win7 x64 Free


:Win7 x64 Chk
ECHO "Win7 x64 Chk"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=Win7 x64 Chk


goto END

:BCHK
:WinBlue x86 Chk
ECHO "WinBlue x86 Chk"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=WinBlue x86 Chk


:WinBlue x64 Chk
ECHO "WinBlue x86 Chk"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=WinBlue x64 Chk


:Win8 x86 Chk
ECHO "Win8 x86 Chk"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=Win8 x86 Chk


:Win8 x64 Chk
ECHO "Win8 x86 Chk
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=Win8 x64 Chk


:Win7 x86 Chk
ECHO "Win7 x86 Chk"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=Win7 x86 Chk


:Win7 x64 Chk
ECHO "Win7 x64 Chk"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd
set BUILDFAIL=Win7 x64 Chk


goto END

:BFREE
:WinBlue x86 Free
ECHO "WinBlue x86 Free"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=WinBlue x86 Free


:WinBlue x64 Free
ECHO "WinBlue x64 Free"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=WinBlue x64 Free


:Win8 x86 Free
ECHO "Win8 x86 Free"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=Win8 x86 Free


:Win8 x64 Free
ECHO "Win8 x64 Free"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=Win8 x64 Free


:Win7 x86 Free
ECHO "Win7 x86 Free"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=Win7 x86 Free


:Win7 x64 Free
ECHO "Win7 x64 Free"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd
set BUILDFAIL=Win7 x64 Free

goto END

:ERROR
echo "========================================"


:END
for %%a in (win7, win8, WinBlue) do (
    for %%b in (chk, fre) do (
        for %%c in (x86, x64) do (
            if "%%c"=="x86" (
                if exist MTKUsb_%VerA%\%%a\%%b\%%c\mtkwlu.sys (
                    echo %%a %%c %%b mtkwlu.sys buil success
                ) ELSE (
                    set RETURNERR=1
                   powershell write-host %%a %%c %%b " Build Failed ......." -foregroundcolor Red 
                )
            ) ELSE (
                if exist MTKUsb_%VerA%\%%a\%%b\%%c\mtkwlux.sys (
                    echo %%a %%c %%b mtkwlux.sys buil success
                ) ELSE (
                    set RETURNERR=1
                    powershell write-host %%a %%c %%b " Build Failed ......." -foregroundcolor Red 
                )
            )

        )
    )
)

ECHO "Build End"
echo %BUILDFAIL%
if "%RETURNERR%"=="1" (
exit /b 1
)