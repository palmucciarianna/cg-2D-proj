// ============================================================================
// File: include/ShapeGenerator.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Generatore procedurale CPU di geometrie 2D complesse e primitive.
//              Costruisce strutture MeshData contenenti vertici e gradienti cromatici
//              usando formule matematiche (trigonometria, curve polari, equazioni parametriche,
//              algoritmo del caos per frattali di Sierpinski).
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "MeshData.h"

class ShapeGenerator
{
public:
    // Genera un singolo triangolo equilatero con gradienti di colore interpolati sui tre vertici.
    static MeshData createTriangle(
        const glm::vec4& color1,
        const glm::vec4& color2,
        const glm::vec4& color3
    );

    // Genera un rettangolo bidimensionale (composto da due triangoli GL_TRIANGLES) con gradiente verticale.
    static MeshData createRectangle(
        float width,
        float height,
        const glm::vec4& colorTop,
        const glm::vec4& colorBottom
    );

    // Genera una moneta circolare dorata con triangoli ad anello (pie slices) e gradiente radiale centro-bordo.
    static MeshData createCoin(
        float radius,
        int segments,
        const glm::vec4& innerColor,
        const glm::vec4& outerColor
    );

    // Genera un trapezio isoscele (utilizzato per la fusoliera aerodinamica della navicella).
    static MeshData createTrapezoid(
        float topWidth,
        float bottomWidth,
        float height,
        const glm::vec4& colorTop,
        const glm::vec4& colorBottom
    );

    // Genera un diamante/rombo a 4 triangoli con vertice centrale luminoso (motore e post-bruciatore).
    static MeshData createDiamond(
        float width,
        float height,
        const glm::vec4& centerColor,
        const glm::vec4& edgeColor
    );

    // Genera un cerchio o ellisse piena utilizzando la primitiva GL_TRIANGLE_FAN.
    static MeshData createCircle(
        float cx,
        float cy,
        float radiusX,
        float radiusY,
        int segments,
        const glm::vec4& centerColor,
        const glm::vec4& borderColor
    );

    // Genera la curva polare trascendente a farfalla (Temple H. Fay, 1989) tramite GL_TRIANGLE_FAN.
    static MeshData createButterfly(
        float cx,
        float cy,
        float radiusX,
        float radiusY,
        int segments,
        const glm::vec4& centerColor,
        const glm::vec4& borderColor
    );

    // Genera una curva cardioide a forma di cuore con equazioni parametriche trigonometriche.
    static MeshData createHeart(
        float cx,
        float cy,
        float radiusX,
        float radiusY,
        int segments,
        const glm::vec4& centerColor,
        const glm::vec4& borderColor
    );

    // Genera una curva a spicchio di luna crescente tramite combinazione armonica di seni e coseni.
    static MeshData createMoon(
        float cx,
        float cy,
        float scaleX,
        float scaleY,
        int segments,
        const glm::vec4& color
    );

    // Genera il frattale del Triangolo di Sierpinski mediante l'algoritmo stocastico Chaos Game (GL_POINTS).
    static MeshData createSierpinski(
        const glm::vec3& T0,
        const glm::vec3& T1,
        const glm::vec3& T2,
        int nPoints,
        const glm::vec4& color
    );
};
