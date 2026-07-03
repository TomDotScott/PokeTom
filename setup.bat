@echo off
setlocal

:: ----------------------
:: Paths
:: ----------------------
if not "%~1"=="" (
    set SFML_DIR=%~1
) else (
    set SFML_DIR=%CD%\Lib\SFML
)

if not "%~2"=="" (
    set BUILD_DIR=%~2
) else (
    set BUILD_DIR=%CD%\build_sfmldb
)

set SOL2_DIR=%CD%\Lib\sol2
set SOL2_SINGLE_HEADER=%CD%\Lib\sol2\single

:: ----------------------
:: Skip cloning sol2 if already exists
:: ----------------------
if exist "%SOL2_DIR%" (
    echo sol2 already cloned. Skipping git clone step.
    goto :GENERATE_SOL2_HEADERS_STEP
)

echo ===============================
echo Cloning sol2 repository...
echo ===============================
mkdir "%SOL2_DIR%"
git clone --depth 1 https://github.com/ThePhD/sol2.git "%SOL2_DIR%"

:: ----------------------
:: Skip generating sol2 if already exists
:: ----------------------
:GENERATE_SOL2_HEADERS_STEP
if exist "%SOL2_DIR%\single\single\include\sol" (
    echo sol2 already cloned and built. Skipping python single.py
    goto :SFML_STEP
)

echo ===============================
echo Generating sol2 includes
echo ===============================
pushd %SOL2_SINGLE_HEADER%
py -3 single.py
popd

:SFML_STEP

:: ----------------------
:: Skip Cloning and Building SFML if already exists
:: ----------------------
if exist "%SFML_DIR%\include\SFML" (
    echo SFML already cloned and built. Skipping setup.
    goto :PYTHON_SETUP
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

:PYTHON_SETUP

:: ----------------------
:: Configure Python
:: ----------------------
@echo ==========================
@echo Configuring Python venv...
@echo ==========================
py -3 -m venv %~dp0.venv

%~dp0.venv\Scripts\pip.exe install -r %~dp0requirements.txt