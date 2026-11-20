// ============================================================================
// File: include/OpenGL_Headers.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Inclusione centralizzata degli header OpenGL / WebGL cross-platform.
//              Su desktop (macOS, Linux, Windows) include GLAD (OpenGL 3.3 Core).
//              Su Web (Emscripten / WASM) include GLES3/gl3.h (WebGL 2.0).
// ============================================================================

#pragma once

#ifdef __EMSCRIPTEN__
    #define GL_GLEXT_PROTOTYPES
    #include <GLES3/gl3.h>
    #include <emscripten.h>
    #include <emscripten/html5.h>
#else
    #define GLFW_INCLUDE_NONE
    #include <glad/glad.h>
#endif
