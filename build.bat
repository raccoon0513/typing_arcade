@echo off
chcp 65001 > nul
echo compiling

gcc -I. main.c src/utils/utils.c src/components/tetris/tetris.c -o main.exe

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