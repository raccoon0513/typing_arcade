@echo off
chcp 65001 > nul
echo compiling...

if exist main.exe (
    del /f /q main.exe
)

set SOURCES=main.c
for /r src %%f in (*.c) do (
    call set SOURCES=%%SOURCES%% "%%f"
)

gcc -I. %SOURCES% -o main.exe

if %errorlevel% equ 0 (
    cls
    echo success. loading game.
    timeout /t 1 /nobreak > nul
    cls
    echo success. loading game..
    timeout /t 1 /nobreak > nul
    cls
    echo success. loading game...
    timeout /t 2 /nobreak > nul
    main.exe
) else (
    echo Error. pls check your location
)