// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: BackgroundRenderer
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Renderizza lo sfondo cosmico procedurale animato a tutto schermo (Requisito 5).
//    - Non carica alcuna immagine pre-renderizzata: genera stelle, nebulose fluide,
//      onde a spirale polari e palette dinamiche interamente via codice GLSL nel fragment shader.
//    - Copre esattamente l'intero display a qualsiasi risoluzione o aspect ratio.
//
// 2. A CHI È COLLEGATA:
//    - Creata e chiamata da: GameWorld (istanziata in init(), invocata all'inizio di render()).
//    - Utilizza:
//      * Shader "background" (background.vert e background.frag).
//      * OpenGL Core: alloca un proprio VAO e VBO per il quad di rendering.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Coordinate NDC Pure (Normalized Device Coordinates) (Domande 9, 18):
//      I vertici sono definiti direttamente nell'intervallo canonico [-1, 1] x [-1, 1].
//      Non necessitano né di Model Matrix né di Projection Matrix: il vertex shader
//      assegna direttamente gl_Position = vec4(aPos, 0.0, 1.0).
//    - Generazione Procedurale & Hash Deterministico (Requisito 5 & Domanda 3):
//      Sfrutta l'architettura massivamente parallela SIMD della GPU per calcolare
//      in tempo reale per ogni pixel (1280x720 = 921.600 campioni) funzioni matematiche di:
//      * Hash pseudo-casuale deterministico senza texture: fract(sin(...) * dot(...));
//      * Domain Warping: coordinate UV deformate da onde sinusoidali annidate;
//      * Parallasse a 3 Strati con scintillio temporale;
//      * Palette Swapping fra livelli pari e dispari modulata dall'uniform 'uLevel'.
//    - Gestione Risorse RAII & Rule of Five (Requisito 8 & Domande 16, 17):
//      Alloca le risorse GPU nel costruttore con glGen*, le dealloca nel distruttore con glDelete*.
//      Costruttori di copia e operatori di assegnamento eliminati ('= delete') per prevenire
//      la duplicazione degli handle hardware e fughe di memoria VRAM.
// ============================================================================

#pragma once

#include <glad/glad.h>
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
