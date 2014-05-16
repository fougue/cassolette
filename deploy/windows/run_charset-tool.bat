@echo off

call setpaths.bat
set PATH=%APP_PATH%;%PATH%
set PATH=%APP_PATH%\3rdparty;%PATH%

start charset-tool.exe %*
exit
