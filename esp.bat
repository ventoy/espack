
@echo off

set espack=.\tool\espack64.exe
if "%PROCESSOR_ARCHITECTURE%"=="AMD64" goto Main
set espack=.\tool\espack32.exe

:Main

%espack% -r ESP

pause
