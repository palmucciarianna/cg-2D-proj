// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: ShapeGenerator (Factory Mesh Procedurali)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - È la factory matematica statica che genera su memoria CPU le strutture MeshData.
//    - Costruisce le primitive geometriche bidimensionali e le forme parametriche:
//      * createCircle: cerchi/dischi con suddivisione polare in Triangle Fan;
//      * createCoin: monete d'oro con gradiente radiale metallico;
//      * createRectangle: quad/rettangoli triangolati in 2 triangoli (6 vertici);
//      * createTriangle: triangoli con colori di vertice differenziati (Gouraud);
//      * createStar: stelle a N punte con raggi polari alternati (r_min, r_max);
//      * createSierpinski: frattale a triangoli annidati tramite l'Algoritmo del Caos.
//
// 2. A CHI È COLLEGATA:
//    - Invocata durante la fase di init() da: Player, AsteroidManager, CollectibleManager, HermiteManager.
//    - Restituisce: strutture MeshData (array di glm::vec3 posizioni, glm::vec4 colori, GLenum primitiva).
//    - I suoi dati vengono poi passati a Geometry::setData() e caricati su VRAM con upload().
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Coordinate Polari per Dischi e Cerchi (Domande 2, 5, 13):
//      Dato un angolo theta_i = 2*pi * i / N, le coordinate sono:
//      x_i = cx + rx * cos(theta_i);  y_i = cy + ry * sin(theta_i).
//      Il centro è (cx, cy); la decomposizione in triangoli usa la topologia GL_TRIANGLE_FAN.
//    - Triangolazione di Poligoni Convessi e Quad (Domanda 14):
//      Un rettangolo è decomposto nei due triangoli: (v0, v1, v2) e (v0, v2, v3).
//    - Assegnazione dei Colori di Vertice (Vertex Color) (Domande 7, 20):
//      Ad ogni vertice viene associata una quaterna RGBA. Durante la rasterizzazione la GPU calcola
//      le coordinate baricentriche, generando sfumature continue morbide (Gouraud Shading).
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
