// ============================================================================
// File: include/BackgroundRenderer.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Renderer specializzato per lo sfondo cosmico animato procedurale.
//              Disegna un quad a tutto schermo direttamente nello spazio NDC
//              (Normalized Device Coordinates) senza richiedere matrici di proiezione.
// ============================================================================

#pragma once

#include "OpenGL_Headers.h"
#include "Shader.h"

class BackgroundRenderer
{
public:
    // Costruttore: riceve il riferimento allo shader procedurale dello sfondo e alloca il quad NDC.
    explicit BackgroundRenderer(const Shader& shader);

    // Distruttore RAII: dealloca VAO e VBO associati al fullscreen quad.
    ~BackgroundRenderer();

    // Disabilita la copia (Rule of Five) per evitare gestione duplicata dei buffer GPU.
    BackgroundRenderer(const BackgroundRenderer&) = delete;
    BackgroundRenderer& operator=(const BackgroundRenderer&) = delete;

    // Disegna il quad a schermo intero passando tempo globale, intensità psichedelica e livello di gioco.
    void draw(float timeSeconds, float psychedelic = 0.35f, int level = 1) const;

private:
    const Shader& m_shader; // Riferimento immutabile allo shader procedurale del background
    GLuint m_vao = 0;       // Vertex Array Object per il quad
    GLuint m_vbo = 0;       // Vertex Buffer Object contenente i 6 vertici in coordinate NDC
};
