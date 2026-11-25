// ============================================================================
// File: include/Collider2D.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Sistema di rilevamento delle collisioni 2D (bounding volumes).
//              Implementa test matematici ad alte prestazioni per:
//              1. Bounding Circle vs Bounding Circle (senza radice quadrata)
//              2. Bounding Box AABB vs AABB (teorema degli assi separatori)
//              3. Bounding Circle vs AABB (clamping del punto più vicino)
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

    // Overload di comodità per confrontare direttamente due struct CircleCollider.
    inline bool checkCircleCircle(const CircleCollider& a, const CircleCollider& b)
    {
        return checkCircleCircle(a.center, a.radius, b.center, b.radius);
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
