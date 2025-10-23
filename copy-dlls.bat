@echo off
echo Copiando DLLs de SDL3...

set SDL3_LIB=C:\SDL3\lib\x64
set SDL3_IMAGE_LIB=C:\SDL3_image\lib\x64
set OUTPUT_DIR=.\bin\Debug

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

copy "%SDL3_LIB%\SDL3.dll" "%OUTPUT_DIR%\" /Y
copy "%SDL3_IMAGE_LIB%\SDL3_image.dll" "%OUTPUT_DIR%\" /Y

echo DLLs copiadas exitosamente!
pause