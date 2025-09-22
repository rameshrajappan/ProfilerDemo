@echo off
echo Building Minimal Profiler in all configurations...

REM Check for admin privileges
NET SESSION >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo This script requires administrator privileges.
    echo Please run as administrator to ensure proper permissions are set.
    echo.
    echo Right-click on the batch file and select "Run as administrator".
    pause
    exit /b 1
)

REM Set Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

echo.
echo Building x86 Configuration...
msbuild MiniProfiler.vcxproj /p:Configuration=Release /p:Platform=x86

echo.
echo Building x64 Configuration...
msbuild MiniProfiler.vcxproj /p:Configuration=Release /p:Platform=x64

echo.
echo All builds completed.
echo.
echo Output files:
echo - x86\Release\MiniProfiler_x86.dll
echo - x64\Release\MiniProfiler_x64.dll
echo.
echo Setting permissions on output files...
echo.
icacls "x86\Release\MiniProfiler_x86.dll" /grant "Everyone:(RX)" /T
icacls "x64\Release\MiniProfiler_x64.dll" /grant "Everyone:(RX)" /T
echo.
echo Copying the files
echo.
copy  /Y "x86\Release\MiniProfiler_x86.dll" ".\MiniProfiler_x86.dll"
copy  /Y "x64\Release\MiniProfiler_x64.dll" ".\MiniProfiler_x64.dll"
echo.
echo Setting permissions on output files...
echo.
icacls ".\MiniProfiler_x86.dll" /grant "Everyone:(RX)" /T
icacls ".\MiniProfiler_x64.dll" /grant "Everyone:(RX)" /T
echo.
echo Remember to copy the appropriate DLL to your test environment.

@REM Made with Bob
