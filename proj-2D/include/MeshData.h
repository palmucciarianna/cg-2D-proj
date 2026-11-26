// ORIGINE: Struttura dati CPU (posizioni, colori, primitiva) basata sul modello dei laboratori della prof.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - STRUTTURA: MeshData (Container Dati Geometrici CPU)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - È il contenitore intermedio in RAM (CPU) che accumula posizioni, colori e tipo primitiva
//      prima che i dati vengano inviati alla VRAM della GPU tramite glBufferData.
//    - Separa la fase di generazione matematica della mesh dalla gestione dei buffer hardware OpenGL.
//
// 2. A CHI È COLLEGATA:
//    - Riempita da: ShapeGenerator (forme procedurali) e PlayerShapeLoader (file OBJ/MTL).
//    - Consumata da: Geometry::setData() e inviata alla GPU con Geometry::upload().
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Separazione Memoria CPU vs VRAM GPU (Domande 1, 4, 17):
//      La CPU prepara i vettori 'positions' (glm::vec3) e 'colors' (glm::vec4).
//      La GPU non può leggere direttamente std::vector della CPU: il metodo upload()
//      esegue glBufferData trasferendo i dati nel bus PCIe verso la memoria VRAM.
//    - Primitive Assembly (Domanda 4):
//      Il campo 'primitiveType' (GL_TRIANGLES, GL_TRIANGLE_FAN) istruisce la GPU su come
//      connettere la sequenza di vertici in triangoli durante la fase di rasterizzazione.
// ============================================================================

#pragma once

#include "OpenGL_Headers.h"
#include <glm/glm.hpp>
#include <vector>

// Struttura che incapsula la geometria generata lato CPU prima del trasferimento sui buffer GPU (VBO).
struct MeshData
{
    // Vettore dinamico delle coordinate (locali) spaziali 3D dei vertici (X, Y, Z).
    // Nel mondo 2D, Z è tipicamente 0.0f (o usato per ordinamento in profondità).
    std::vector<glm::vec3> positions;

    // Vettore dinamico dei canali di colore per ogni vertice in formato RGBA (Red, Green, Blue, Alpha) in [0.0, 1.0].
    // Ogni vertice in positions deve avere un colore corrispondente in colors (mapping 1:1 per attributi interleaved/separati).
    // la GPU interpreta il gradiente via coordinate baricentriche durante la rasterizzazione dei triangoli.
    std::vector<glm::vec4> colors;

    // Tipo di primitiva geometrica OpenGL utilizzato nella rasterizzazione:
    // Può assumere valori come GL_TRIANGLES, GL_TRIANGLE_FAN, GL_LINE_STRIP, GL_LINES, ecc.
    // Il valore predefinito è GL_TRIANGLES.
    // dice a glDrawArrays come collegare i vertici in triangoli o linee durante il rendering.
    GLenum primitiveType = GL_TRIANGLES;
};
