// ============================================================================
// File: include/MeshData.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Struttura dati CPU leggera (POD-like) per il trasferimento e l'assemblaggio
//              di vertici, attributi di colore RGBA e tipo di primitiva di rendering OpenGL.
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// Struttura che incapsula la geometria generata lato CPU prima del trasferimento sui buffer GPU (VBO).
struct MeshData
{
    // Vettore dinamico delle coordinate spaziali 3D dei vertici (X, Y, Z).
    // Nel nostro mondo 2D, Z è tipicamente 0.0f (o usato per ordinamento in profondità).
    std::vector<glm::vec3> positions;

    // Vettore dinamico dei canali di colore per ogni vertice in formato RGBA (Red, Green, Blue, Alpha) in [0.0, 1.0].
    // Ogni vertice in positions deve avere un colore corrispondente in colors (mapping 1:1 per attributi interleaved/separati).
    std::vector<glm::vec4> colors;

    // Tipo di primitiva geometrica OpenGL utilizzato nella rasterizzazione:
    // Può assumere valori come GL_TRIANGLES, GL_TRIANGLE_FAN, GL_LINE_STRIP, GL_LINES, ecc.
    // Il valore predefinito è GL_TRIANGLES.
    GLenum primitiveType = GL_TRIANGLES;
};
