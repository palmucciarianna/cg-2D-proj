// ============================================================================
// File: src/HazardManager.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del gestore delle minacce ambientali.
//              - Spawn casuale ogni 3.2 - 5.7 secondi
//              - Fase di telegrafia con lampeggio visivo sinusoidale ad alta frequenza (30 rad/s)
//              - Lancio balistico ad altissima velocità (880 px/s)
//              - Scia particellare dinamica durante il volo e cancellazione off-screen
// ============================================================================

#include "HazardManager.h"
#include "ShapeGenerator.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>

// ============================================================================
// Costruttore
// ============================================================================
HazardManager::HazardManager()
{
}

// ============================================================================
// Inizializzazione Geometrie dei Proiettili e dei Mirini
// ============================================================================
void HazardManager::init()
{
    // Geometria del dardo a rombo/diamante con nucleo bianco termico e bordi cremisi.
    MeshData shotData = ShapeGenerator::createDiamond(
        34.0f, 14.0f,
        glm::vec4(1.0f, 0.95f, 0.8f, 1.0f),  // Centro bianco rovente
        glm::vec4(1.0f, 0.15f, 0.1f, 1.0f)   // Bordi rosso cremisi
    );
    m_projectileGeo.setData(shotData.positions, shotData.colors, shotData.primitiveType);
    m_projectileGeo.upload();

    // Geometria dell'indicatore di allerta (mirino rettangolare rosso).
    MeshData warnData = ShapeGenerator::createRectangle(
        28.0f, 6.0f,
        glm::vec4(1.0f, 0.2f, 0.2f, 1.0f),
        glm::vec4(1.0f, 0.05f, 0.05f, 0.8f)
    );
    m_warningGeo.setData(warnData.positions, warnData.colors, warnData.primitiveType);
    m_warningGeo.upload();
}

// ============================================================================
// Reset dello Stato dei Pericoli
// ============================================================================
void HazardManager::reset()
{
    m_hazards.clear();
    m_spawnCooldown = 3.0f;
}

// ============================================================================
// Aggiornamento Logica di Spawn e Volo Balistico
// ============================================================================
void HazardManager::update(float dt, float screenWidth, float screenHeight, ParticleSystem& particles)
{
    // Decrementa il timer per la prossima generazione di pericolo.
    m_spawnCooldown -= dt;
    if (m_spawnCooldown <= 0.0f)
    {
        HazardShot shot;
        // Scelta equiprobabile del lato di partenza: 0 = sinistra verso destra, 1 = destra verso sinistra.
        int sideChoice = rand() % 2;

        if (sideChoice == 0) // Da sinistra
        {
            shot.side = HazardSide::LEFT;
            float y = 70.0f + static_cast<float>(rand() % static_cast<int>(screenHeight - 140.0f));
            shot.pos = glm::vec2(0.0f, y);
            shot.dir = glm::vec2(1.0f, 0.0f);
        }
        else // Da destra
        {
            shot.side = HazardSide::RIGHT;
            float y = 70.0f + static_cast<float>(rand() % static_cast<int>(screenHeight - 140.0f));
            shot.pos = glm::vec2(screenWidth, y);
            shot.dir = glm::vec2(-1.0f, 0.0f);
        }

        shot.speed = 880.0f;       // Velocità supersonica per testare i riflessi del giocatore
        shot.radius = 16.0f;       // Bounding radius
        shot.warningTimer = 0.85f; // Finestra di telegrafia (preavviso di 0.85s)
        shot.fired = false;
        shot.active = true;

        m_hazards.push_back(shot);

        // Intervallo di ricarica per il prossimo colpo: tra 3.2 e 5.7 secondi.
        m_spawnCooldown = 3.2f + static_cast<float>(rand() % 25) * 0.1f;
    }

    // Aggiornamento dello stato dei colpi esistenti:
    for (auto it = m_hazards.begin(); it != m_hazards.end(); )
    {
        if (!it->fired)
        {
            // Fase 1: Conteggio del tempo di preavviso sul bordo.
            it->warningTimer -= dt;
            if (it->warningTimer <= 0.0f)
            {
                it->fired = true; // Scatta il lancio del proiettile
            }
            ++it;
        }
        else
        {
            // Fase 2: Integrazione cinematica del volo ad alta velocità.
            it->pos += it->dir * it->speed * dt;

            // Emissione stocastica di scintille rosse come scia del proiettile.
            if (rand() % 3 == 0)
            {
                particles.emitCoinSparkles(it->pos, 2);
            }

            // Rimuove il proiettile se ha superato completamente i margini dello schermo.
            if (it->pos.x < -80.0f || it->pos.x > screenWidth + 80.0f)
            {
                it = m_hazards.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

// ============================================================================
// Rendering dei Segnali di Allerta e dei Proiettili in Volo
// ============================================================================
void HazardManager::draw(const Shader& shader, float currentTime, float screenWidth, float screenHeight)
{
    (void)screenHeight;

    for (const auto& h : m_hazards)
    {
        if (!h.fired)
        {
            // Segnale di preavviso stroboscopico sul bordo:
            // Funzione sinusoidale a 30 rad/s per un effetto visivo d'emergenza intermittente.
            float blink = std::sin(currentTime * 30.0f);
            if (blink > -0.2f)
            {
                glm::mat4 m = glm::mat4(1.0f);
                float posX = (h.side == HazardSide::LEFT) ? 20.0f : screenWidth - 20.0f;
                m = glm::translate(m, glm::vec3(posX, h.pos.y, 0.0f));
                m = glm::scale(m, glm::vec3(1.6f, 1.8f, 1.0f));

                // Tinta rosso vivo
                shader.setVec4("uColorTint", glm::vec4(1.0f, 0.1f, 0.1f, 1.0f));
                shader.setMat4("model", m);
                m_warningGeo.draw();
            }
        }
        else
        {
            // Disegno del missile di plasma a rombo:
            float angle = glm::degrees(std::atan2(h.dir.y, h.dir.x));
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(h.pos, 0.0f));
            m = glm::rotate(m, glm::radians(angle), glm::vec3(0, 0, 1));
            m = glm::scale(m, glm::vec3(1.6f, 1.6f, 1.0f));

            // Bagliore pulsante al plasma
            shader.setVec4("uColorTint", glm::vec4(1.0f, 0.3f, 0.2f, 1.0f));
            shader.setMat4("model", m);
            m_projectileGeo.draw();
        }
    }

    // Ripristino tinta shader
    shader.setVec4("uColorTint", glm::vec4(1.0f));
}

// ============================================================================
// Verifica Impatto Colpo Fatale con il Giocatore
// ============================================================================
bool HazardManager::checkPlayerHit(const CircleCollider& playerCol, ParticleSystem& particles)
{
    for (auto& h : m_hazards)
    {
        if (h.fired && h.active)
        {
            // Test di intersezione tra cerchi senza sqrt():
            float distSq = glm::dot(playerCol.center - h.pos, playerCol.center - h.pos);
            float radSum = playerCol.radius + h.radius;

            if (distSq <= radSum * radSum)
            {
                h.active = false;
                // Innesca un'esplosione particellare massiccia (45 frammenti).
                particles.emitAsteroidExplosion(h.pos, 45);
                return true; // Colpito!
            }
        }
    }
    return false;
}
