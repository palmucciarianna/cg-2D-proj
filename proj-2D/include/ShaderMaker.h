// ORIGINE: Base diretta dai laboratori della prof (compilazione e linking GLSL), adattata in C++.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: ShaderMaker (Compilatore e Linker GLSL)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - È la classe di utilità che compila i codici sorgente GLSL e crea i programmi OpenGL.
//    - Passaggi operativi eseguiti:
//      * Legge da disco i file di testo con estensione .vert e .frag;
//      * Crea gli shader hardware con glCreateShader(GL_VERTEX_SHADER / GL_FRAGMENT_SHADER);
//      * Invia il testo sorgente con glShaderSource e compila con glCompileShader;
//      * Controlla lo stato di compilazione (glGetShaderiv con GL_COMPILE_STATUS);
//      * Crea lo Shader Program con glCreateProgram, aggancia gli shader con glAttachShader;
//      * Esegue il linking con glLinkProgram e controlla gli errori (glGetProgramiv).
//    - In caso di errore di sintassi GLSL, estrae il log di errore (glGetShaderInfoLog / glGetProgramInfoLog)
//      e lo stampa su console per facilitare il debugging.
//
// 2. A CHI È COLLEGATA:
//    - Invocata da: Shader::load() durante l'inizializzazione dell'applicazione.
//    - Riceve i percorsi dei file: assets/shaders/basic.*, background.*, texture.*, coin.*.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Compilazione Just-In-Time sulla GPU (Domande 3, 16, 18):
//      Gli shader GLSL non vengono compilati dal compilatore C++ (Clang/GCC/MSVC),
//      ma dal compilatore interno del driver della scheda grafica a runtime.
//    - Collegamento della Pipeline Programmabile (Domanda 18):
//      Il linking assicura la coerenza delle variabili 'varying' (out vec4 vColor nel vertex shader
//      che diventa in vec4 vColor nel fragment shader) e risolve la tabella delle uniform.
// ============================================================================

#pragma once

#include "OpenGL_Headers.h"
#include <GLFW/glfw3.h>

class ShaderMaker
{
public:
    // Crea un programma shader OpenGL completo a partire dai percorsi dei file sorgente.
    // Compila il Vertex Shader e il Fragment Shader, verifica gli errori di compilazione,
    // esegue il linking nel programma finale e restituisce il Program ID (handle OpenGL).
    static GLuint createProgram(const char* vertexfilename, const char* fragmentfilename);

    // Legge il contenuto testuale di un file GLSL da disco e restituisce un buffer allocato.
    static char* readShaderSource(const char* shaderFile);

private:
    // Costruttore privato: impedisce l'istanziazione di questa classe puramente statica.
    ShaderMaker() = default;
};
