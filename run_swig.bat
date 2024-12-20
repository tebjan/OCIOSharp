@echo off
setlocal
rem Determine the directory of the batch file
set "SWIG_DIR=%~dp0swigwin-4.3.0"
rem Temporarily add SWIG_DIR to the PATH
set "PATH=%SWIG_DIR%;%PATH%"

rem Define paths for OpenColorIO header files and SWIG interface file
set "OCIO_INCLUDE_DIR=%~dp0OpenColorIO/install/include/OpenColorIO"
set "INTERFACE_FILE=%OCIO_INCLUDE_DIR%/OpenColorIO.i"
set "OUTPUT_DIR=%~dp0generated"

rem Create output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

rem Run SWIG to generate C# bindings
swig -csharp -outdir "%OUTPUT_DIR%" "%INTERFACE_FILE%"

rem Define MSVC compiler paths (adjust if needed)
set "CL=cl.exe"
set "LINK=link.exe"

rem Compile the generated SWIG wrapper with MSVC
"%CL%" /EHsc /LD /I"%OCIO_INCLUDE_DIR%" "%OUTPUT_DIR%\OpenColorIO_wrap.cxx" /Fo"%OUTPUT_DIR%\OpenColorIO_wrap.obj" /link /DLL /OUT:"%OUTPUT_DIR%\OpenColorIO.dll"


rem End the session
endlocal
