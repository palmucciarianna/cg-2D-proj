// ============================================================================
// File: include/ShaderMaker.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Classe statica di utilità didattica per la lettura, compilazione
//              e linking degli shader GLSL (Vertex e Fragment Shader).
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
