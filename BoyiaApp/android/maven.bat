@echo off

setlocal

set PROJ_DIR=%~dp0
set MVN_DIR=%PROJ_DIR%\maven
set CORE_DIR=%PROJ_DIR%\core
set BOYIA_MVN_DIR=%MVN_DIR%\BoyiaMaven
echo %MVN_DIR%

if not exist %MVN_DIR% (
:: rd /s /Q %MVN_DIR%
md %MVN_DIR%
cd %MVN_DIR%
git clone git@github.com:damonyan1985/BoyiaMaven.git
)

cd %CORE_DIR%
gradle :core:uploadArchives

echo "Upload Core Finished"
pause