#!/bin/bash
cd "$(dirname "$0")"
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"

if [ ! -f "build/Interactive2DWorld" ]; then
    echo "======================================================="
    echo "  Compilazione Astro-Miner 2D: Cosmic Coin Rush"
    echo "======================================================="
    cmake -B build
    cmake --build build
fi

echo "Avvio dell'applicazione grafica..."
./build/Interactive2DWorld
