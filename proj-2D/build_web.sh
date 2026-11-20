#!/bin/bash
# ==============================================================================
# Script: build_web.sh
# Progetto: Astro-Miner 2D: Cosmic Coin Rush
# Descrizione: Compilazione automatica del gioco in formato WebAssembly (WASM)
#              e WebGL 2.0 tramite Emscripten SDK.
# ==============================================================================

set -e

echo "=========================================================="
echo "  Astro-Miner 2D - WebAssembly / WebGL 2.0 Build Script   "
echo "=========================================================="

# 1. Verifica della presenza di Emscripten SDK nel PATH
if ! command -v emcmake &> /dev/null || ! command -v emcc &> /dev/null; then
    echo ""
    echo "[ATTENZIONE] Emscripten SDK (emcmake / emcc) non rilevato nel PATH corrente."
    echo ""
    echo "Per installare e attivare l'Emscripten SDK ufficiale:"
    echo "  1) git clone https://github.com/emscripten-core/emsdk.git"
    echo "  2) cd emsdk"
    echo "  3) ./emsdk install latest"
    echo "  4) ./emsdk activate latest"
    echo "  5) source ./emsdk_env.sh"
    echo ""
    echo "Dopo aver attivato l'ambiente con 'source ./emsdk_env.sh', riesegui questo script:"
    echo "  ./build_web.sh"
    echo ""
    exit 1
fi

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build-web"

echo "[1/3] Configurazione CMake con Emscripten..."
emcmake cmake -B "${BUILD_DIR}" -S "${PROJECT_DIR}" -DCMAKE_BUILD_TYPE=Release

echo ""
echo "[2/3] Compilazione WebAssembly in corso..."
cmake --build "${BUILD_DIR}" --config Release -j 4

echo ""
echo "[3/3] Compilazione completata con successo!"
echo "Artefatti Web generati in: ${BUILD_DIR}/"
echo "  - Interactive2DWorld.html"
echo "  - Interactive2DWorld.js"
echo "  - Interactive2DWorld.wasm"
echo "  - Interactive2DWorld.data (Assets precaricati)"
echo ""
echo "=========================================================="
echo "  Per testare il gioco nel browser via server HTTP locale:"
echo "  python3 -m http.server 8080 --directory \"${BUILD_DIR}\""
echo "  Poi apri nel browser: http://localhost:8080/Interactive2DWorld.html"
echo "=========================================================="
