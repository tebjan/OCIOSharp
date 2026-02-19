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

:: Clean old CMake cache to avoid picking up system/conda libraries
if exist "%BUILD_DIR%\CMakeCache.txt" del "%BUILD_DIR%\CMakeCache.txt"

:: Navigate to the build directory
cd /d "%BUILD_DIR%"

:: Configure the build with CMake
:: OCIO_INSTALL_EXT_PACKAGES=ALL forces building all dependencies from source
:: Disable find of system expat/yaml-cpp to ensure static linking into OCIO
cmake -G "Visual Studio 17 2022" ^
      -A x64 ^
      -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
      -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
      -DOCIO_INSTALL_EXT_PACKAGES=ALL ^
      -DBUILD_SHARED_LIBS=ON ^
      -Dexpat_FORCE_INTERNAL_BUILD=ON ^
      -Dyaml-cpp_FORCE_INTERNAL_BUILD=ON ^
      -DCMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH=OFF ^
      -DCMAKE_FIND_USE_CMAKE_SYSTEM_PATH=OFF ^
      "%OCIO_SOURCE_DIR%"

:: Build and install OpenColorIO
cmake --build . --config %BUILD_TYPE% --target INSTALL

:: Copy the built DLL to the deployment folders
echo Copying OpenColorIO_2_5.dll to deployment folders...
copy /Y "%INSTALL_DIR%\bin\OpenColorIO_2_5.dll" "%~dp0..\..\runtimes\win-x64\native\"
copy /Y "%INSTALL_DIR%\bin\OpenColorIO_2_5.dll" "%~dp0..\..\lib\net8.0-windows7.0\"

endlocal
