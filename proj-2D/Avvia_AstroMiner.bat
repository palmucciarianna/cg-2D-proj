@echo off
cd /d "%~dp0"
if not exist "build\Release\Interactive2DWorld.exe" (
    echo =======================================================
    echo   Compilazione Astro-Miner 2D: Cosmic Coin Rush
    echo =======================================================
    cmake -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release
)
start "" "build\Release\Interactive2DWorld.exe"
