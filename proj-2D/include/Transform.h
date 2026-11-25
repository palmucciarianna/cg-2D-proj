// ORIGINE: Trasformazioni affini 2D (T * R * S) e gerarchia dai laboratori della prof, incapsulate in classe C++.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: Transform (Algebra delle Trasformazioni Affini)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Incapsula la terna di trasformazione affine 2D: posizione (vec2), rotazione (float gradi), scala (vec2).
//    - Calcola la matrice di trasformazione Model 4x4: getMatrix() -> M = T * R * S.
//    - Supporta la modellazione gerarchica tramite moltiplicazione per la matrice del genitore:
//      getComposedMatrix(parentMatrix) -> parentMatrix * (T * R * S).
//
// 2. A CHI È COLLEGATA:
//    - Utilizzata da: Player (per posizionare e articolare i componenti dell'albero gerarchico)
//      e in generale dalle entità geometriche della scena.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Spazi Affini & Coordinate Omogenee (Requisito 4 & Domanda 6):
//      Un vettore colonna in coordinate omogenee 4D P = (x, y, 0, 1)^T viene trasformato da:
//      M = T(dx, dy) * R(theta) * S(sx, sy).
//    - Importanza dell'Ordine di Moltiplicazione (Non Commutativita' TR != RT) (Domanda 6):
//      Con convenzione column-major (OpenGL e GLM), l'azione si legge da destra a sinistra:
//      1) S(sx, sy): scala l'oggetto attorno alla propria origine locale;
//      2) R(theta): ruota l'oggetto attorno alla propria origine;
//      3) T(dx, dy): trasla l'oggetto nella sua posizione finale nel mondo.
//      Se invertissimo T ed R (R * T), l'oggetto ruoterebbe attorno all'origine del mondo (0,0)!
//    - Propagazione Gerarchica nei Grafi di Scena (Requisito 3.1 & Domanda 8):
//      Per un nodo figlio: M_world = M_parent * M_local.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
public:
    // Posizione 2D nel sistema di coordinate del mondo (in pixel o unità world).
    glm::vec2 position = glm::vec2(0.0f);

    // Angolo di rotazione attorno all'asse Z (espresso in gradi sessagesimali).
    float rotation = 0.0f;

    // Fattore di scala bidimensionale lungo l'asse X e l'asse Y.
    glm::vec2 scale = glm::vec2(1.0f);

    // Costruttore di default: inizializza posizione a (0,0), rotazione a 0° e scala unitaria (1,1).
    Transform() = default;

    // Costruttore parametrizzato per specificare stato iniziale personalizzato.
    Transform(const glm::vec2& pos, float rot = 0.0f, const glm::vec2& sc = glm::vec2(1.0f))
        : position(pos), rotation(rot), scale(sc) {}

    // ========================================================================
    // Calcolo della Matrice di Modellazione Locale: M = T * R * S
    // ========================================================================
    // La composizione delle trasformazioni non è commutativa:
    // Per applicare prima la scala, poi la rotazione e infine la traslazione a un vertice v:
    // v' = M * v = (T * R * S) * v = T * (R * (S * v))
    // GLM utilizza matrici in formato column-major (standard OpenGL).
    glm::mat4 localMatrix() const
    {
        // 1. Inizializza con la matrice identità 4x4:
        //    | 1  0  0  0 |
        //    | 0  1  0  0 |
        //    | 0  0  1  0 |
        //    | 0  0  0  1 |
        glm::mat4 m = glm::mat4(1.0f);

        // 2. Moltiplica a destra per la matrice di Traslazione T(position.x, position.y, 0):
        m = glm::translate(m, glm::vec3(position, 0.0f));

        // 3. Moltiplica a destra per la matrice di Rotazione R attorno all'asse Z (0, 0, 1):
        //    Convertiamo l'angolo da gradi a radianti con glm::radians.
        m = glm::rotate(m, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));

        // 4. Moltiplica a destra per la matrice di Scalatura S(scale.x, scale.y, 1):
        m = glm::scale(m, glm::vec3(scale, 1.0f));

        // Restituisce la matrice locale risultante M_local.
        return m;
    }

    // ========================================================================
    // Calcolo della Matrice Mondiale con Modellazione Gerarchica (Scena ad Albero)
    // ========================================================================
    // Per oggetti composti da parti dipendenti (es. le ali o la cabina attaccate alla fusoliera):
    // la matrice finale nel mondo è data dal prodotto tra la matrice del nodo genitore e la matrice locale:
    // M_world = M_parent * M_local
    // Se non viene passato alcun genitore, parentMatrix è l'identità e M_world = M_local.
    glm::mat4 worldMatrix(const glm::mat4& parentMatrix = glm::mat4(1.0f)) const
    {
        return parentMatrix * localMatrix();
    }
};
