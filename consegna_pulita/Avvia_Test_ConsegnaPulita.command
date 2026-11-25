#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJ_DIR="$(cd "$SCRIPT_DIR/../proj-2D" && pwd)"

echo "======================================================="
echo "  Compilazione Test da: consegna_pulita/include + src"
echo "======================================================="

clang++ -std=c++17 -O2 -Wno-nontrivial-memcall \
    -I"$SCRIPT_DIR/include" \
    -I"$PROJ_DIR/external" \
    -I"$PROJ_DIR/external/glad/include" \
    -I"$PROJ_DIR/external/glfw/include" \
    -I"$PROJ_DIR/external/glm" \
    -I"$PROJ_DIR/external/imgui" \
    -I"$PROJ_DIR/external/imgui/backends" \
    "$SCRIPT_DIR"/src/*.cpp \
    "$PROJ_DIR/build/libimgui.a" \
    "$PROJ_DIR/build/libglad.a" \
    "$PROJ_DIR/external/glfw/lib/macos/libglfw3.a" \
    -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL -framework QuartzCore \
    -o "$SCRIPT_DIR/TestConsegna"

if [ $? -ne 0 ]; then
    echo ""
    echo "[ERRORE] Compilazione fallita! Controlla di aver copiato tutti i file in include/ e src/."
    read -p "Premi INVIO per chiudere..."
    exit 1
fi

echo "[OK] Compilazione riuscita! Avvio del gioco..."
cd "$PROJ_DIR"
"$SCRIPT_DIR/TestConsegna"
