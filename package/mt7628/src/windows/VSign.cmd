@echo off
set CURRENDIR=%cd%

echo ========================================
echo WinXP driver signer check...
echo =====
if exist "%CURRENDIR%\Disk1\drivers\WinXP\x86\RT2860.sys" (
	cd %CURRENDIR%\Disk1\drivers\WinXP\x86
	call VerifySign.cmd RT2860.sys
	call VerifySign.cmd RT2860.cat)

if exist "%CURRENDIR%\Disk1\drivers\WinXP\x64\RT2860.sys" (
	cd %CURRENDIR%\Disk1\drivers\WinXP\x64
	call VerifySign.cmd RT2860.sys
	call VerifySign.cmd RT2860.cat)

echo ========================================
echo WinVista driver signer check...
echo =====
if exist "%CURRENDIR%\Disk1\drivers\WinVista\x86\netr28.sys" (
	cd %CURRENDIR%\Disk1\drivers\WinVista\x86
	call VerifySign.cmd netr28.sys
	call VerifySign.cmd netr28.cat)

if exist "%CURRENDIR%\Disk1\drivers\WinVista\x64\netr28x.sys" (
	cd %CURRENDIR%\Disk1\drivers\WinVista\x64
	call VerifySign.cmd netr28x.sys
	call VerifySign.cmd netr28x.cat)

echo ========================================
echo Win7 driver signer check...
echo =====
if exist "%CURRENDIR%\Disk1\drivers\Win7\x86\netr28.sys" (
	cd %CURRENDIR%\Disk1\drivers\Win7\x86
	call VerifySign.cmd netr28.sys
	call VerifySign.cmd netr28.cat)

if exist "%CURRENDIR%\Disk1\drivers\Win7\x64\netr28x.sys" (
	cd %CURRENDIR%\Disk1\drivers\Win7\x64
	call VerifySign.cmd netr28x.sys
	call VerifySign.cmd netr28x.cat)

echo ========================================
echo Win8 driver signer check...
echo =====
if exist "%CURRENDIR%\Disk1\drivers\Win8\x86\netr28.sys" (
	cd %CURRENDIR%\Disk1\drivers\Win8\x86
	call VerifySign.cmd netr28.sys
	call VerifySign.cmd netr28.cat)

if exist "%CURRENDIR%\Disk1\drivers\Win8\x64\netr28x.sys" (
	cd %CURRENDIR%\Disk1\drivers\Win8\x64
	call VerifySign.cmd netr28x.sys
	call VerifySign.cmd netr28x.cat)

cd %CURRENDIR%
@echo on
