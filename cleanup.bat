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
rd /s /q obj
rd /s /q output
rd /s /q properties

rd /s /q lib\lua\build_vs2022

:: ----------------------
:: Delete any generated VS files
:: ----------------------
for %%f in (*.sln *.vcxproj *.vcxproj.filters *.csproj sharpmakeautocleanupdb_debugsolution.json) do (
    echo Deleting %%f
    del /q "%%f"
)

for %%f in (src\Engine\CodeGen) do (
    echo Deleting %%f
    del /q "%%f"
)

:: ----------------------
:: Libraries
:: ----------------------
echo Removing SFML build and libs...
rd /s /q build_sfmldb
rd /s /q SFML
rd /s /q Lib\SFML\lib
rd /s /q Lib\SFML\include
rd /s /q Lib\SFML\share
rd /s /q Lib\SFML\src

echo Removing sol2 folder
rd /s /q Lib\sol2

echo Cleanup complete.
pause
