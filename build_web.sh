#!/bin/bash
# ==============================================================================
# Script: build_web.sh (Root wrapper)
# Progetto: Astro-Miner 2D: Cosmic Coin Rush
# ==============================================================================

set -e
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "${PROJECT_ROOT}/proj-2D/build_web.sh" "$@"
