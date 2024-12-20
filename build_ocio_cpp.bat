@echo off
setlocal

:: Configuration
set BUILD_TYPE=Release
set OCIO_SOURCE_DIR=%~dp0OpenColorIO
set BUILD_DIR=%OCIO_SOURCE_DIR%\build
set INSTALL_DIR=%~dp0OcioSharpCLI\ThirdParty\OCIO

:: Ensure required directories exist
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

:: Navigate to the build directory
cd /d "%BUILD_DIR%"

:: Configure the build with CMake
cmake -G "Visual Studio 16 2019" ^
      -A x64 ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
      "%OCIO_SOURCE_DIR%"

:: Build and install OpenColorIO
cmake --build . --config %BUILD_TYPE% --target INSTALL

endlocal
