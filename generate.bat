:: ----------------------
:: Paths
:: ----------------------
set SFML_DIR=%CD%\Lib\SFML
set BUILD_DIR=%CD%\build_sfmldb

:: ----------------------
:: Skip if already exists
:: ----------------------
if exist "%BUILD_DIR%" (
    echo SFML already cloned and built. Skipping setup.
    goto generate
)

if not exist "%SFML_DIR%" mkdir "%SFML_DIR%"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

call setup.bat %SFML_DIR% %BUILD_DIR%

:generate
:: -------------------------------
:: Generate the asset header files
:: -------------------------------
echo ===========================================
echo Generating header files from assets.yaml...
echo ===========================================
py -3 generate_assets_header.py

:: -------------------------------
:: Generate the solution with Sharpmake
:: -------------------------------
echo ================================
echo Generating Sharpmake Solution...
echo ================================

pushd Sharpmake
Sharpmake.Application.exe /sources('../main.sharpmake.cs') /generateDebugSolution
popd