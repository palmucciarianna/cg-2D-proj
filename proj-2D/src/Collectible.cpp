// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: Collectible.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Genera e anima le 5 monete d'oro collezionabili (Requisito 7.2).
//    - Implementa la rotazione pseudo-3D e l'oscillazione armonica (Requisito 6).
// 2. A CHI È COLLEGATO:
//    - Coordinato da GameWorld; attiva l'apertura del Warp Gate al raggiungimento di 5 monete.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - Rotazione Pseudo-3D via scala modulata da coseno: s_x(t) = |cos(omega * t)| (Domanda 6, 9).
//    - Levitazione sinusoidale verticale: y(t) = y_base + A * sin(nu * t + phi) (Requisito 6).
//    - Buffer Sharing su singolo VAO per tutte le 5 monete (Domanda 17).
// ============================================================================

#include "Collectible.h"
#include "ShapeGenerator.h"
#include "MathConstants.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>

// ============================================================================
// Costruttore
// ============================================================================
CollectibleManager::CollectibleManager()
{
}

// ============================================================================
// Inizializzazione della Geometria Condivisa
// ============================================================================
void CollectibleManager::init()
{
    // Genera una moneta d'oro a 36 segmenti con gradiente radiale:
    // nucleo giallo brillante ed estremità ambrata metallica.
    MeshData coinData = ShapeGenerator::createCoin(
        22.0f, 36,
        glm::vec4(1.0f, 0.95f, 0.40f, 1.0f),
        glm::vec4(0.85f, 0.55f, 0.05f, 1.0f)
    );

    // Carica la mesh nei buffer VBO/VAO della scheda video.
    m_coinGeo.setData(coinData.positions, coinData.colors, coinData.primitiveType);
    m_coinGeo.upload();
}

// ============================================================================
// Generazione delle Monete nel Livello
// ============================================================================
void CollectibleManager::spawnLevel(int levelCount, float screenWidth, float screenHeight)
{
    (void)levelCount;

    // Svuota la collezione precedente e azzera il contatore di raccolta.
    m_coins.clear();
    m_collectedCount = 0;

    // Requisito 7.2: Genera esattamente 5 monete collezionabili per livello.
    const int count = 5;
    for (int i = 0; i < count; ++i)
    {
        CosmicCoin c;

        // Distribuzione polare distanziata dal centro per favorire l'esplorazione della mappa.
        float angle = (i * MathConstants::TWO_PI) / count + (rand() % 80) * 0.01f;
        float dist = 180.0f + (rand() % 240);

        c.basePos.x = screenWidth * 0.5f + std::cos(angle) * dist;
        c.basePos.y = screenHeight * 0.5f + std::sin(angle) * dist;

        // Limita le posizioni all'interno dello schermo giocabile.
        c.basePos.x = glm::clamp(c.basePos.x, 80.0f, screenWidth - 80.0f);
        c.basePos.y = glm::clamp(c.basePos.y, 80.0f, screenHeight - 80.0f);

        c.currentPos = c.basePos;
        // Assegna uno sfasamento temporale per rendere il moto oscillatorio asincrono tra le monete.
        c.phaseOffset = static_cast<float>(i * 1.3f);
        c.radius = 22.0f;
        c.collected = false;

        m_coins.push_back(c);
    }
}

// ============================================================================
// Aggiornamento del Moto Armonico di Galleggiamento
// ============================================================================
void CollectibleManager::update(float dt, float currentTime)
{
    (void)dt;

    for (auto& c : m_coins)
    {
        if (!c.collected)
        {
            // Moto armonico verticale: y(t) = y_base + A * sin(omega * t + phi)
            // Ampiezza A = 7 pixel, pulsazione omega = 3.6 rad/s.
            c.currentPos.y = c.basePos.y + 7.0f * std::sin(currentTime * 3.6f + c.phaseOffset);
        }
    }
}

// ============================================================================
// Rendering con Simulazione di Rotazione Pseudo-3D
// ============================================================================
void CollectibleManager::draw(const Shader& shader, float currentTime)
{
    for (const auto& c : m_coins)
    {
        if (!c.collected)
        {
            // Rotazione 3D fittizia attorno all'asse Y:
            // Scalando la coordinata orizzontale X col coseno del tempo (cos(omega * t)),
            // la moneta circolare si assottiglia fino a diventare una linea e poi si riallarga,
            // simulando perfettamente una rotazione volumetrica su se stessa.
            float spinX = std::cos(currentTime * 4.2f + c.phaseOffset);

            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(c.currentPos, 0.0f));
            m = glm::scale(m, glm::vec3(spinX, 1.0f, 1.0f));

            // Trasferisce la matrice Model alla uniform dello shader.
            shader.setMat4("model", m);

            // Emette il disegno della geometria condivisa.
            m_coinGeo.draw();
        }
    }
}

// ============================================================================
// Test di Collisione e Raccolta col Giocatore
// ============================================================================
int CollectibleManager::checkCollisions(const CircleCollider& playerCol, ParticleSystem& particles)
{
    int newlyCollected = 0;

    for (auto& c : m_coins)
    {
        if (!c.collected)
        {
            // Test di intersezione tra cerchi basato sulla distanza quadratica.
            float distSq = glm::dot(playerCol.center - c.currentPos, playerCol.center - c.currentPos);
            float radSum = playerCol.radius + c.radius;

            if (distSq <= radSum * radSum)
            {
                // La moneta viene raccolta:
                c.collected = true;
                m_collectedCount++;
                newlyCollected++;

                // Genera un'esplosione di 18 scintille dorate con fisica particellare.
                particles.emitCoinSparkles(c.currentPos, 18);
            }
        }
    }

    return newlyCollected;
}
