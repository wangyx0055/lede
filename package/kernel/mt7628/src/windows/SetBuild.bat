
@echo off
echo Please set the environment variables.

@ECHO OFF  
powershell write-host "`n------ Select Target Operating System ----------------------------" -foregroundcolor Green
echo (1)WinBlue	(2)Win8	 (3)Win7 
SET /P INPUT=
IF "%INPUT%"=="" GOTO Error

if "%INPUT%"=="1" (
	set DDK_TARGET_OS=WinBlue
) ELSE (
	if "%INPUT%"=="2" (
	set DDK_TARGET_OS=Win8
) ELSE (
	set DDK_TARGET_OS=Win7
)	
	
	
)
powershell write-host "`n------ Select Target Architecture ----------------------------" -foregroundcolor Green
echo (1)i386	(2)amd64
SET /P INPUT=
IF "%INPUT%"=="" GOTO Error

if "%INPUT%"=="1" (
	set BUILD_DEFAULT_TARGETS=i386
) ELSE (
	set BUILD_DEFAULT_TARGETS=amd64
)
powershell write-host "`n------ Select Release Type  ----------------------------" -foregroundcolor Green
echo (1)chk	(2)fre
SET /P INPUT=
IF "%INPUT%"=="" GOTO Error

if "%INPUT%"=="1" (
	set DDKBUILDENV=chk
) ELSE (
	set DDKBUILDENV=fre
)
powershell write-host "`n+ Select as below ---" -foregroundcolor Yellow
ECHO DDK_TARGET_OS = %DDK_TARGET_OS%
ECHO BUILD_DEFAULT_TARGETS = %BUILD_DEFAULT_TARGETS%
ECHO DDKBUILDENV = %DDKBUILDENV%
ECHO DDK_TARGET = %DDK_TARGET%


if "%DDK_TARGET_OS%"=="Win7" (

	if "%DDKBUILDENV%"=="chk" (
		set DDKENV=chk   
	) ELSE (
		set DDKENV=fre
	)
)

if "%DDK_TARGET_OS%"=="Win7" (
	if "%BUILD_DEFAULT_TARGETS%"=="i386" (
		set WIN7TAR=x86
	) ELSE (
		set WIN7TAR=x64
	)
)

set CURRENDIRA=%CD%
IF "%WindowsSdkDir%"=="" (
	set BUILD_DEFAULT_TARGETS=
	set DDKBUILDENV=
if "%DDK_TARGET_OS%"=="Win7" (
	C:\WinDDK\7600.16385.0\bin\setenv.bat C:\WinDDK\7600.16385.0\ %DDKENV% %WIN7TAR% WIN7
cd /d %CURRENDIRA%	
)
)


GOTO End
:Error
echo error!!!!
:End
