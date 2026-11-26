#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONSEGNA_CMD="$SCRIPT_DIR/../consegna/Avvia_Test_ConsegnaPulita.command"

if [ -f "$CONSEGNA_CMD" ]; then
    exec bash "$CONSEGNA_CMD"
else
    echo "[ERRORE] Non trovato: $CONSEGNA_CMD"
    read -p "Premi INVIO per chiudere..."
    exit 1
fi
