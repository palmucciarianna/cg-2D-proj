// ============================================================================
// GUIDA RAPIDA PER L'ORALE - MODULO: Collider2D (Bounding Volumes & Geometria Computazionale)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Fornisce le strutture dati dei volumi di delimitazione (Bounding Volumes):
//      * CircleCollider (centro, raggio);
//      * AABBCollider (Axis-Aligned Bounding Box: minPoint, maxPoint).
//    - Implementa i tre algoritmi matematici fondamentali di intersezione geometrica:
//      * checkCircleCircle: test Cerchio vs Cerchio (ottimizzato senza radice quadrata);
//      * checkAABB_AABB: test AABB vs AABB (Teorema degli Assi Separatori SAT);
//      * checkCircleAABB: test Cerchio vs AABB (clamping del punto più vicino).
//
// 2. A CHI È COLLEGATO:
//    - Utilizzato in tutte le entità di gioco: Player, AsteroidManager, CollectibleManager, HermiteManager, HazardManager.
//    - Invocato ad ogni frame dal coordinatore GameWorld::checkCollisions() per risolvere le interazioni fisiche.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Cerchio vs Cerchio & Prodotto Scalare (Requisito 7 & Domande 5, 7):
//      Condizione: dist(c1, c2) <= r1 + r2.
//      Elevando al quadrato per evitare la costosa funzione sqrt():
//      diff = c1 - c2;  distSq = dot(diff, diff);
//      Collisione se: distSq <= (r1 + r2)^2.
//    - Teorema degli Assi Separatori (SAT) per AABB (Domanda 10):
//      Due rettangoli si intersecano se e solo se i loro intervalli proiettati si sovrappongono
//      contemporaneamente sia sull'asse X che sull'asse Y:
//      (a.min.x <= b.max.x && a.max.x >= b.min.x) && (a.min.y <= b.max.y && a.max.y >= b.min.y).
//    - Cerchio vs AABB tramite Punto Clamped (Domanda 5):
//      Trova il punto p sul bordo/interno dell'AABB più vicino al centro c del cerchio:
//      p = clamp(c, box.min, box.max);  testa se dot(c - p, c - p) <= r^2.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <algorithm>

// Enumerazione tipizzata delle forme di collisione geometrica supportate nel mondo di gioco.
enum class ColliderType
{
    CIRCLE, // Collider circolare, ideale per entità isotrope e rotanti (monete, asteroidi, navicella)
    AABB    // Axis-Aligned Bounding Box, allineato agli assi del mondo (ideale per trigger e ostacoli rettangolari)
};

// Struttura dati per un volume di collisione circolare.
struct CircleCollider
{
    glm::vec2 center{ 0.0f }; // Posizione del centro del cerchio nello spazio del mondo
    float radius{ 10.0f };    // Raggio geometrico della sfera di collisione in pixel
};

// Struttura dati per un volume di collisione rettangolare allineato agli assi (AABB).
struct AABBCollider
{
    glm::vec2 minPoint{ 0.0f }; // Coordinate dell'angolo inferiore sinistro (min_x, min_y)
    glm::vec2 maxPoint{ 0.0f }; // Coordinate dell'angolo superiore destro (max_x, max_y)

    // Restituisce il baricentro dell'AABB sommando gli estremi e dimezzando il vettore.
    glm::vec2 center() const { return (minPoint + maxPoint) * 0.5f; }

    // Restituisce le semi-dimensioni (semi-ampiezza orizzontale e semi-altezza verticale).
    glm::vec2 halfSize() const { return (maxPoint - minPoint) * 0.5f; }
};

namespace Collision2D
{
    // ========================================================================
    // 1. Test di Collisione Cerchio - Cerchio (Circle-Circle Intersection)
    // ========================================================================
    // Due cerchi si intersecano se e solo se la distanza euclidea tra i loro centri
    // è minore o uguale alla somma dei loro raggi:
    // dist(c1, c2) <= r1 + r2
    // Per massimizzare le prestazioni computazionali e azzerare l'impatto sul game-loop,
    // eleviamo entrambi i membri al quadrato per evitare la costosa funzione sqrt():
    // dist^2 <= (r1 + r2)^2, dove dist^2 = dot(c1 - c2, c1 - c2)
    inline bool checkCircleCircle(const glm::vec2& c1, float r1, const glm::vec2& c2, float r2)
    {
        // Vettore differenza tra i due centri.
        glm::vec2 diff = c1 - c2;
        // Prodotto scalare del vettore per se stesso, pari alla distanza quadratica.
        float distSq = glm::dot(diff, diff);
        // Somma dei raggi algebrici.
        float radSum = r1 + r2;
        // Test di intersezione scalare.
        return distSq <= (radSum * radSum);
    }

    // ========================================================================
    // 2. Test di Collisione AABB - AABB (Axis-Aligned Bounding Box)
    // ========================================================================
    // Basato sul Teorema degli Assi Separatori (SAT) proiettato sugli assi cartesiani X e Y:
    // Se su almeno un asse i due intervalli di proiezione sono completamente disgiunti,
    // i due rettangoli non possono collidere.
    inline bool checkAABBAABB(const glm::vec2& min1, const glm::vec2& max1, const glm::vec2& min2, const glm::vec2& max2)
    {
        // Se il bordo destro del primo è a sinistra del sinistro del secondo, o viceversa: separati su X.
        if (max1.x < min2.x || min1.x > max2.x) return false;
        // Se il bordo superiore del primo è sotto l'inferiore del secondo, o viceversa: separati su Y.
        if (max1.y < min2.y || min1.y > max2.y) return false;
        // Nessun asse separatore trovato: i volumi si sovrappongono.
        return true;
    }

    // ========================================================================
    // 3. Test di Collisione Misto Cerchio - AABB (Circle-AABB Intersection)
    // ========================================================================
    // Si determina il punto geometrico dell'AABB più vicino al centro del cerchio
    // limitando ("clamping") le coordinate del centro dell'intervallo [min, max].
    // Si valuta poi se la distanza tra il centro del cerchio e tale punto è <= raggio.
    inline bool checkCircleAABB(const glm::vec2& c, float r, const glm::vec2& boxMin, const glm::vec2& boxMax)
    {
        // Calcolo del punto proiettato più vicino all'interno della bounding box.
        glm::vec2 closest(
            glm::clamp(c.x, boxMin.x, boxMax.x),
            glm::clamp(c.y, boxMin.y, boxMax.y)
        );

        // Vettore dal punto proiettato al centro del cerchio.
        glm::vec2 diff = c - closest;
        // Distanza quadratica.
        float distSq = glm::dot(diff, diff);
        // Verifica se la distanza quadratica è minore o uguale al quadrato del raggio.
        return distSq <= (r * r);
    }
}
