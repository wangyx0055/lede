@echo off
echo BD  [2013/12/13  V1.0.0]

if "%DDK_TARGET_OS%"=="" (
    echo Please do setBuild.bat First.
    goto end
)

ECHO DDK_TARGET_OS = %DDK_TARGET_OS%
ECHO BUILD_DEFAULT_TARGETS = %BUILD_DEFAULT_TARGETS%
ECHO DDKBUILDENV = %DDKBUILDENV%
set SRC_ROOT=%CD%
cd ..
set SRC_ROOT_UNIFY=%CD%
cd /d %SRC_ROOT%
set WindowsSdkDir=C:\Program Files (x86)\Windows Kits\8.1
set MSBUILD_PLATFORM_TOOLSET=WindowsKernelModeDriver8.1

if exist %SRC_ROOT%\Include\BusTypeDef.h del /F %SRC_ROOT%\Include\BusTypeDef.h
copy %SRC_ROOT%\Include\BusTypeDef_USB %SRC_ROOT%\Include\BusTypeDef.h

if exist %SRC_ROOT%\Include\QAToolDef.h del /F %SRC_ROOT%\Include\QAToolDef.h
copy %SRC_ROOT%\Include\QATool_NotSupport %SRC_ROOT%\Include\QAToolDef.h

Copy Doc\History.txt %SRC_ROOT%
Call CurrentVerOnHistory.exe
del History.txt

  
ECHO "Start to copy file from UnifyBuildCfg "

copy %SRC_ROOT%\UnifyBuildCfg\*.* %SRC_ROOT%\hal\
copy %SRC_ROOT%\UnifyBuildCfg\common\*.* %SRC_ROOT%\hal\common\
copy %SRC_ROOT%\UnifyBuildCfg\mt7603\*.* %SRC_ROOT%\hal\mt7603\  
ECHO "Copy from UnifyBuildCfg end..."

if "%DDK_TARGET_OS%"=="WinXP" goto ndis51_x86
if "%DDK_TARGET_OS%"=="WinNET" goto ndis51_x64
if "%DDK_TARGET_OS%"=="WinLH" goto ndis60
if "%DDK_TARGET_OS%"=="Win7" goto BuildW7
if "%DDK_TARGET_OS%"=="WinBlue" goto BuildWinBlue
if "%DDK_TARGET_OS%"=="Win8" goto BuildW8

:BuildW7

FOR /F  %%i IN (hisver.txt) DO set Ver=%%i

if "%DDKBUILDENV%"=="chk" (
set chkorFree=objchk
) ELSE (
set chkorFree=objfre
)
if "%DDKBUILDENV%"=="fre" set MSBUILD_CONFIGURATION="Win7 Release"
if "%DDKBUILDENV%"=="chk" set MSBUILD_CONFIGURATION="Win7 Debug"
echo DDKBUILDENV= %DDKBUILDENV%
echo MSBUILD_CONFIGURATION=%MSBUILD_CONFIGURATION%
echo DDK_INC_PATH=%DDK_INC_PATH%
echo WindowsSdkDir=%WindowsSdkDir%

set PLATFORM_INC=PLATFORM\NDIS6\Ndis62.inc
set osversion=6.2
if exist %SRC_ROOT%\Include\OS_VER.h del /F %SRC_ROOT%\Include\OS_VER.h
copy %SRC_ROOT%\Include\OS_VER_Win7 %SRC_ROOT%\Include\OS_VER.h

echo PLATFORM_INC=%PLATFORM_INC%
IF "%BUILD_DEFAULT_TARGETS%"=="amd64" (
    echo "ndis62 X64........."
set CPUTYPE=amd64
set CPU_X=x64
set MSBUILD_PLATFORM=x64

goto BUILD
) ELSE (
IF "%BUILD_DEFAULT_TARGETS%"=="-amd64" (
    echo "ndis62 X64........."
set CPUTYPE=amd64
set CPU_X=x64
set MSBUILD_PLATFORM=x64

goto BUILD
) ELSE (

echo "ndis62 X86.........!!!!"
set CPUTYPE=i386
set CPU_X=x86
set MSBUILD_PLATFORM=Win32
if exist PLATFORM\Ndis6\USB\Obj%BUILD_ENV%_win7_x86\i386\*.obj       del  PLATFORM\Ndis6\USB\Obj%BUILD_ENV%_win7_x86\i386\*.obj
if exist PLATFORM\Ndis6\USB\i386 del exist PLATFORM\Ndis6\USB\i386

goto BUILD
)
)

:BuildW8
FOR /F  %%i IN (hisver.txt) DO set Ver=%%i

FOR %%i in (*.VcxProj) do SET BuildFile=%%i

set PLATFORM_INC=PLATFORM\NDIS6\Ndis63.inc
if "%DDKBUILDENV%"=="fre" set MSBUILD_CONFIGURATION="Win8 Release"
if "%DDKBUILDENV%"=="chk" set MSBUILD_CONFIGURATION="Win8 Debug"
echo DDK_INC_PATH=%DDK_INC_PATH%
echo WindowsSdkDir=%WindowsSdkDir%
echo "=================="
if "%DDKBUILDENV%"=="chk" (
set chkorFree=objchk
set DbgorRelease=Debug
) ELSE (
set chkorFree=objfre
set DbgorRelease=Release
)
if exist %SRC_ROOT%\Include\OS_VER.h del /F %SRC_ROOT%\Include\OS_VER.h
copy %SRC_ROOT%\Include\OS_VER_Win8 %SRC_ROOT%\Include\OS_VER.h
IF "%BUILD_DEFAULT_TARGETS%"=="amd64" (
    echo "Win8 X64........."
set CPUTYPE=amd64
set CPU_X=x64
set MSBUILD_PLATFORM=x64
SET sysName=%BuildFile:~0,-8%x
if exist NDIS6\Win8%DbgorRelease%\%CPU_X%\*.tlog      del  NDIS6\Win8%DbgorRelease%\%CPU_X%\*.tlog 


goto BUILD
) ELSE (
    echo "Win8 X86........."
set CPUTYPE=i386
set CPU_X=x86
set MSBUILD_PLATFORM=Win32
SET sysName=%BuildFile:~0,-8%
if exist NDIS6\Win8%DbgorRelease%\%CPU_X%\*.tlog       del  NDIS6\Win8%DbgorRelease%\%CPU_X%\*.tlog 

goto BUILD
)

:BuildWinBlue
FOR /F  %%i IN (hisver.txt) DO set Ver=%%i
cd /d Ndis64
FOR %%i in (*.VcxProj) do SET BuildFile=%%i
SET CHIP=RT2860
IF "%BuildFile:~4,2%"=="73" SET EXTRACHAR=64
IF "%BuildFile:~6,1%"=="u" SET CHIP=RT2870

set PLATFORM_INC=PLATFORM\NDIS6\Ndis64.inc
if "%DDKBUILDENV%"=="fre" set MSBUILD_CONFIGURATION="Win8.1 Release"
if "%DDKBUILDENV%"=="chk" set MSBUILD_CONFIGURATION="Win8.1 Debug"
if "%DDKBUILDENV%"=="chk" (
set chkorFree=objchk
set DbgorRelease=Debug
) ELSE (
set chkorFree=objfre
set DbgorRelease=Release
)

if exist %SRC_ROOT%\Include\OS_VER.h del /F %SRC_ROOT%\Include\OS_VER.h
copy %SRC_ROOT%\Include\OS_VER_Win81 %SRC_ROOT%\Include\OS_VER.h
IF "%BUILD_DEFAULT_TARGETS%"=="amd64" (
    echo "ndis64 X64........."
set CPUTYPE=amd64
set CPU_X=x64
set MSBUILD_PLATFORM=x64

goto BUILD
) ELSE (
    echo "ndis64 X86........."
set CPUTYPE=i386
set CPU_X=x86
set MSBUILD_PLATFORM=Win32
SET sysName=%BuildFile:~0,-8%

goto BUILD
)

:BUILD
echo 1111 %MSBUILD_CONFIGURATION%, 222 "%MSBUILD_PLATFORM%, 333 %sysName%
echo %CPUTYPE%

echo %DDK_INC_PATH%
echo %WindowsSdkDir%
IF "%DDK_TARGET_OS%"=="Win7" (
        IF "%WindowsSdkDir%"==""    (
        build -cZgw ) ELSE (
        msbuild /p:Configuration=%MSBUILD_CONFIGURATION% /p:Platform="%MSBUILD_PLATFORM%" /p:SignMode=Off /flp1:warningsonly;logfile=msbuild.wrn /flp2:errorsonly;logfile=msbuild.err MsBuild_Win_MtkUsb.sln
        )
        goto COPYFILE7
) ELSE (
    msbuild /p:Configuration=%MSBUILD_CONFIGURATION% /p:Platform="%MSBUILD_PLATFORM%" /p:SignMode=Off /flp1:warningsonly;logfile=msbuild.wrn /flp2:errorsonly;logfile=msbuild.err MsBuild_Win_MtkUsb.sln
IF "%DDK_TARGET_OS%"=="Win8" (
    goto COPYFILE8
) ELSE (    
    goto COPYFILE81

)
)




:COPYFILE7
echo ================================
echo Start To copy file...
if "%1%"=="" (
set VerA=%Ver%
) else (
set VerA=%Ver%_%1
)

set RELEASE_FOLDER=MTKUsb_%VerA%\%DDK_TARGET_OS%\%DDKBUILDENV%
set Symbol=MTKUsb_%VerA%_Symbol\%DDK_TARGET_OS%\%DDKBUILDENV%
if not exist %RELEASE_FOLDER%\%CPU_X%            md %RELEASE_FOLDER%\%CPU_X%
if not exist %Symbol%\%CPU_X%                          md %Symbol%\%CPU_X%

copy hal\FW\*.bin %RELEASE_FOLDER%\%CPU_X%  

copy PLATFORM\INF\RaCoInst.dat         %RELEASE_FOLDER%\%CPU_X%\RaCoInst.dat
echo CPU_X=%CPU_X%
echo CPUTYPE=%CPUTYPE%
if "%CPU_X%"=="x64" (
copy PLATFORM\INF\USB\mtkwlux_win7.inf %RELEASE_FOLDER%\%CPU_X%\mtkwlux.inf   
copy PLATFORM\INF\RaCoInstx.dll         %RELEASE_FOLDER%\%CPU_X%\RaCoInstx.dll
copy PLATFORM\Ndis6\USB\%CPU_X%\*.sys %RELEASE_FOLDER%\%CPU_X%  
copy PLATFORM\Ndis6\USB\%CPU_X%\*.pdb %Symbol%\%CPU_X%

@echo off
call gencat7x %RELEASE_FOLDER%\%CPU_X%  
) ELSE (
copy PLATFORM\INF\USB\mtkwlu_win7.inf %RELEASE_FOLDER%\%CPU_X%\mtkwlu.inf  
copy PLATFORM\INF\RaCoInst.dll         %RELEASE_FOLDER%\%CPU_X%\RaCoInst.dll
copy PLATFORM\Ndis6\USB\%CPU_X%\*.sys %RELEASE_FOLDER%\%CPU_X%  
copy PLATFORM\Ndis6\USB\%CPU_X%\*.pdb %Symbol%\%CPU_X%

@echo off
call gencat7 %RELEASE_FOLDER%\%CPU_X%  
)

goto SIGNDRIVER


:COPYFILE8
echo ================================
echo Start To copy file.....
if "%1%"=="" (
set VerA=%Ver%
) else (
set VerA=%Ver%_%1
)

set RELEASE_FOLDER=MTKUsb_%VerA%\%DDK_TARGET_OS%\%DDKBUILDENV%
set Symbol=MTKUsb_%VerA%_Symbol\%DDK_TARGET_OS%\%DDKBUILDENV%
if not exist %RELEASE_FOLDER%\%CPU_X%            md %RELEASE_FOLDER%\%CPU_X%
if not exist %Symbol%\%CPU_X%                          md %Symbol%\%CPU_X%

copy hal\FW\*.bin %RELEASE_FOLDER%\%CPU_X%  

copy PLATFORM\INF\RaCoInst.dat         %RELEASE_FOLDER%\%CPU_X%\RaCoInst.dat

if "%CPU_X%"=="x64" (
copy PLATFORM\INF\USB\mtkwlux.inf %RELEASE_FOLDER%\%CPU_X%\mtkwlux.inf  
copy PLATFORM\INF\RaCoInstx.dll         %RELEASE_FOLDER%\%CPU_X%\RaCoInstx.dll
copy PLATFORM\Ndis6\USB\%CPU_X%\*.sys %RELEASE_FOLDER%\%CPU_X%  
copy PLATFORM\Ndis6\USB\%CPU_X%\*.pdb %Symbol%\%CPU_X%


@echo off
call gencat7x %RELEASE_FOLDER%\%CPU_X%  
) ELSE (
copy PLATFORM\INF\USB\mtkwlu.inf %RELEASE_FOLDER%\%CPU_X%\mtkwlu.inf  
copy PLATFORM\INF\RaCoInst.dll         %RELEASE_FOLDER%\%CPU_X%\RaCoInst.dll
copy PLATFORM\Ndis6\USB\%CPU_X%\*.sys %RELEASE_FOLDER%\%CPU_X%  
copy PLATFORM\Ndis6\USB\%CPU_X%\*.pdb %Symbol%\%CPU_X%

@echo off
call gencat7 %RELEASE_FOLDER%\%CPU_X%  
)
goto SIGNDRIVER


:COPYFILE81
echo ================================
echo Start To copy file.....
if "%1%"=="" (
set VerA=%Ver%
) else (
set VerA=%Ver%_%1
)

set RELEASE_FOLDER=MTKUsb_%VerA%\%DDK_TARGET_OS%\%DDKBUILDENV%
set Symbol=MTKUsb_%VerA%_Symbol\%DDK_TARGET_OS%\%DDKBUILDENV%
if not exist %RELEASE_FOLDER%\%CPU_X%            md %RELEASE_FOLDER%\%CPU_X%
if not exist %Symbol%\%CPU_X%                          md %Symbol%\%CPU_X%

copy hal\FW\*.bin %RELEASE_FOLDER%\%CPU_X%  

copy PLATFORM\INF\RaCoInst.dat         %RELEASE_FOLDER%\%CPU_X%\RaCoInst.dat

if "%CPU_X%"=="x64" (
copy PLATFORM\INF\USB\mtkwlux.inf %RELEASE_FOLDER%\%CPU_X%\mtkwlux.inf  
copy PLATFORM\INF\RaCoInstx.dll         %RELEASE_FOLDER%\%CPU_X%\RaCoInstx.dll
copy PLATFORM\Ndis6\USB\%CPU_X%\*.sys %RELEASE_FOLDER%\%CPU_X%  
copy PLATFORM\Ndis6\USB\%CPU_X%\*.pdb %Symbol%\%CPU_X%
@echo off
call gencat7x %RELEASE_FOLDER%\%CPU_X%  
) ELSE (
copy PLATFORM\INF\USB\mtkwlu.inf %RELEASE_FOLDER%\%CPU_X%\mtkwlu.inf  
copy PLATFORM\INF\RaCoInst.dll         %RELEASE_FOLDER%\%CPU_X%\RaCoInst.dll
copy PLATFORM\Ndis6\USB\%CPU_X%\*.sys %RELEASE_FOLDER%\%CPU_X%  
copy PLATFORM\Ndis6\USB\%CPU_X%\*.pdb %Symbol%\%CPU_X%
@echo off
call gencat7 %RELEASE_FOLDER%\%CPU_X%  
)

goto SIGNDRIVER


:SIGNDRIVER
set CURRENDIR=%cd%

signtool sign /ac .\Verisign.cer -f .\MediatekInc.pfx -p www.mediatek.com -t http://timestamp.verisign.com/scripts/timstamp.dll "%CURRENDIR%\%RELEASE_FOLDER%\%CPU_X%\*.cat"
signtool sign /ac .\Verisign.cer -f .\MediatekInc.pfx -p www.mediatek.com -t http://timestamp.verisign.com/scripts/timstamp.dll "%CURRENDIR%\%RELEASE_FOLDER%\%CPU_X%\*.sys"
goto END

:END
cd /d %CURRENDIR%
if exist hisver.txt del hisver.txt
if exist %SRC_ROOT%\hal\dirs    del %SRC_ROOT%\hal\dirs
if exist %SRC_ROOT%\hal\dirs.sln    del %SRC_ROOT%\hal\dirs.sln
if exist %SRC_ROOT%\hal\common\*.vcxproj    del %SRC_ROOT%\hal\common\*.vcxproj
if exist %SRC_ROOT%\hal\common\makefile     del %SRC_ROOT%\hal\common\makefile
if exist %SRC_ROOT%\hal\common\sources  del %SRC_ROOT%\hal\common\sources
if exist %SRC_ROOT%\hal\common\sources.props    del %SRC_ROOT%\hal\common\sources.props
if exist %SRC_ROOT%\hal\mt7603\*.vcxproj    del %SRC_ROOT%\hal\mt7603\*.vcxproj
if exist %SRC_ROOT%\hal\mt7603\makefile     del %SRC_ROOT%\hal\mt7603\makefile
if exist %SRC_ROOT%\hal\mt7603\sources  del %SRC_ROOT%\hal\mt7603\sources
if exist %SRC_ROOT%\hal\mt7603\sources.props    del %SRC_ROOT%\hal\mt7603\sources.props


