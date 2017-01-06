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

:WinBlue x86 Chk
ECHO "WinBlue x86 Chk"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd

:WinBlue x64 Free
ECHO "WinBlue x64 Free"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd

:WinBlue x64 Chk
ECHO "WinBlue x86 Chk"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd

:Win8 x86 Free
ECHO "Win8 x86 Free"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd

:Win8 x86 Chk
ECHO "Win8 x86 Chk"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd

:Win8 x64 Free
ECHO "Win8 x64 Free"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd

:Win8 x64 Chk
ECHO "Win8 x86 Chk
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd

:Win7 x86 Free
ECHO "Win7 x86 Free"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd

:Win7 x86 Chk
ECHO "Win7 x86 Chk"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd

:Win7 x64 Free
ECHO "Win7 x64 Free"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd

:Win7 x64 Chk
ECHO "Win7 x64 Chk"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd

goto END

:BCHK
:WinBlue x86 Chk
ECHO "WinBlue x86 Chk"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd

:WinBlue x64 Chk
ECHO "WinBlue x86 Chk"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd

:Win8 x86 Chk
ECHO "Win8 x86 Chk"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd

:Win8 x64 Chk
ECHO "Win8 x86 Chk
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd

:Win7 x86 Chk
ECHO "Win7 x86 Chk"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=chk
call BuildMtkU.cmd

:Win7 x64 Chk
ECHO "Win7 x64 Chk"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=chk
call BuildMtkU.cmd

goto END

:BFREE
:WinBlue x86 Free
ECHO "WinBlue x86 Free"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd

:WinBlue x64 Free
ECHO "WinBlue x64 Free"
set DDK_TARGET_OS=WinBlue
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd

:Win8 x86 Free
ECHO "Win8 x86 Free"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd

:Win8 x64 Free
ECHO "Win8 x64 Free"
set DDK_TARGET_OS=Win8
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd

:Win7 x86 Free
ECHO "Win7 x86 Free"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=i386
set DDKBUILDENV=fre
call BuildMtkU.cmd

:Win7 x64 Free
ECHO "Win7 x64 Free"
set DDK_TARGET_OS=Win7
set BUILD_DEFAULT_TARGETS=amd64
set DDKBUILDENV=fre
call BuildMtkU.cmd

goto END

:END
ECHO "Build All finish"