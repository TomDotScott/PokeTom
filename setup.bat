@echo off
setlocal

:: ----------------------
:: Paths
:: ----------------------
set SFML_DIR=%CD%\Lib\SFML
set BUILD_DIR=%CD%\build_sfmldb

:: ----------------------
:: Skip if already exists
:: ----------------------
if exist "%SFML_DIR%\include\SFML" (
    echo SFML already cloned and built. Skipping setup.
    goto :EOF
)

:: ----------------------
:: Clone SFML repo
:: ----------------------
echo ===============================
echo Cloning SFML repository...
echo ===============================
if not exist "%SFML_DIR%" mkdir "%SFML_DIR%"
git clone --branch 3.0.2 --depth 1 https://github.com/SFML/SFML.git "%SFML_DIR%\src"

:: ----------------------
:: Create build directory
:: ----------------------
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%"

:: ----------------------
:: Configure CMake
:: ----------------------
echo ===============================
echo Configuring SFML with CMake...
echo ===============================

cmake "%SFML_DIR%\src" ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_INSTALL_PREFIX="%SFML_DIR%" ^
    -DSFML_BUILD_EXAMPLES=OFF ^
    -DSFML_BUILD_TESTS=OFF ^
    -DSFML_BUILD_DOC=OFF

:: ----------------------
:: Build and install SFML
:: ----------------------
echo ===============================
echo Building and installing SFML...
echo ===============================

cmake --build . --config Release --target INSTALL
cmake --build . --config Debug   --target INSTALL

:: ----------------------
:: Prepare SFML folder
:: ----------------------
if exist "%SFML_DIR%\include" rmdir /s /q "%SFML_DIR%\include"
if exist "%SFML_DIR%\lib"     rmdir /s /q "%SFML_DIR%\lib"
if exist "%SFML_DIR%\bin"     rmdir /s /q "%SFML_DIR%\bin"
