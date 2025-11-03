@echo off
setlocal

echo ================================
echo Cleaning up Sharpmake and SFML...
echo ================================

:: ----------------------
:: Sharpmake intermediates
:: ----------------------
echo Removing Sharpmake build folders and generated projects...
rd /s /q build_vs2022
rd /s /q build

:: ----------------------
:: Delete any generated VS files
:: ----------------------
for %%f in (*.sln *.vcxproj *.vcxproj.filters *.csproj sharpmakeautocleanupdb_debugsolution.json) do (
    echo Deleting %%f
    del /q "%%f"
)

:: ----------------------
:: SFML build and libraries
:: ----------------------
echo Removing SFML build and libs...
rd /s /q build_sfmldb
rd /s /q Lib\SFML\lib
rd /s /q Lib\SFML\include
rd /s /q Lib\SFML\share
rd /s /q Lib\SFML\src

echo Cleanup complete.
pause
