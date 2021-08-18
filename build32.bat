
@echo off

REM set right vcvarsall.bat file path
set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"

for %%i in (7 8 9 10 11 12 13 14 15 16 17 18 19 20) do (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio %%i.0\VC\vcvarsall.bat" (
        set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio %%i.0\VC\vcvarsall.bat"
        goto Main
    )
)

:Main
if not exist %VCVARSALL% (
    echo Please set right vcvarsall.bat path
    exit /b 1
)

echo %VCVARSALL% x86
call %VCVARSALL% x86

if exist espack32.exe del /q espack32.exe

cl.exe /c src/fat_io_lib/release/fat_access.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo
cl.exe /c src/fat_io_lib/release/fat_cache.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo
cl.exe /c src/fat_io_lib/release/fat_filelib.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo
cl.exe /c src/fat_io_lib/release/fat_format.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo
cl.exe /c src/fat_io_lib/release/fat_misc.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo
cl.exe /c src/fat_io_lib/release/fat_string.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo
cl.exe /c src/fat_io_lib/release/fat_table.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo
cl.exe /c src/fat_io_lib/release/fat_write.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo

cl.exe /c src/crc32.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo
cl.exe /c src/espack.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo
cl.exe /c src/espack_win.c /I./src/fat_io_lib/release /I./src /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /fp:precise /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_LIB" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /Oy- /Oi /MT /EHsc /nologo

link.exe *.obj /OUT:"espack32.exe" /MANIFEST /LTCG /NXCOMPAT /DYNAMICBASE "VirtDisk.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG /MACHINE:X86 /OPT:REF /SAFESEH /INCREMENTAL:NO  /SUBSYSTEM:CONSOLE /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /OPT:ICF /ERRORREPORT:PROMPT /NOLOGO /TLBID:1 

del /q *.pdb
del /q *.manifest
del /q *.obj

if not exist espack32.exe (
    echo Failed to build espack32.exe
    exit /b 1
)
echo Build espack32.exe for x86 success ...
echo.
pause
