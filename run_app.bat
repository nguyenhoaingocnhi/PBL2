@echo off
chcp 65001 >nul
title TutorFlow - PBL2
cd /d "%~dp0"
if not exist "build\PBL2App.exe" (
    echo Chua build ung dung. Chay: cmake -S . -B build ^&^& cmake --build build
    pause
    exit /b 1
)
echo Dang khoi chay ung dung PBL2...
start "" "%~dp0build\PBL2App.exe"
