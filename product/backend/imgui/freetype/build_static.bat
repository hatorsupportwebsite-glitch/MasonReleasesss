@echo off
REM Build FreeType as STATIC library (no freetype.dll).
REM Run this once from the product folder or from freetype folder.
REM Result: out_static\Release\freetype.lib and out_static\Debug\freetype.lib

set FT_DIR=%~dp0
cd /d "%FT_DIR%"

if not exist "include\ft2build.h" (
    echo Error: Run this from freetype folder. include\ft2build.h not found.
    pause
    exit /b 1
)

echo Configuring FreeType as static lib (no ZLIB/PNG/BZIP2/BROTLI/HARFBUZZ)...
cmake -B out_static -G "Visual Studio 17 2022" -A x64 ^
  -DBUILD_SHARED_LIBS=OFF ^
  -DFT_DISABLE_ZLIB=ON ^
  -DFT_DISABLE_PNG=ON ^
  -DFT_DISABLE_BZIP2=ON ^
  -DFT_DISABLE_BROTLI=ON ^
  -DFT_DISABLE_HARFBUZZ=ON

if errorlevel 1 (
    echo Trying Visual Studio 16 2019...
    cmake -B out_static -G "Visual Studio 16 2019" -A x64 ^
      -DBUILD_SHARED_LIBS=OFF ^
      -DFT_DISABLE_ZLIB=ON ^
      -DFT_DISABLE_PNG=ON ^
      -DFT_DISABLE_BZIP2=ON ^
      -DFT_DISABLE_BROTLI=ON ^
      -DFT_DISABLE_HARFBUZZ=ON
)
if errorlevel 1 (
    echo CMake configure failed. Install CMake and Visual Studio (x64).
    pause
    exit /b 1
)

echo Building Release...
cmake --build out_static --config Release
if errorlevel 1 ( echo Release build failed. & pause & exit /b 1 )

echo Building Debug...
cmake --build out_static --config Debug
if errorlevel 1 ( echo Debug build failed. & pause & exit /b 1 )

echo Done. freetype.lib is in out_static\Release and out_static\Debug
echo Product project will link this static lib - no freetype.dll needed for injection.
pause
