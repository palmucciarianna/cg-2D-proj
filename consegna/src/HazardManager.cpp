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
    m_projectileGeo.uploadMesh(shotData);

    // Geometria dell'indicatore di allerta (mirino rettangolare rosso).
    MeshData warnData = ShapeGenerator::createRectangle(
        28.0f, 6.0f,
        glm::vec4(1.0f, 0.2f, 0.2f, 1.0f),
        glm::vec4(1.0f, 0.05f, 0.05f, 0.8f)
    );
    m_warningGeo.uploadMesh(warnData);
}

// ============================================================================
// Reset dello Stato dei Pericoli
// ============================================================================
void HazardManager::reset()
{
    m_hazards.clear();     // Svuota tutti i colpi e i segnali di allerta attivi
    m_spawnCooldown = 2.5f; // Reimposta il tempo di attesa iniziale a 2.5 secondi
}

// ============================================================================
// Aggiornamento Logica di Spawn e Volo Balistico (con scaling di difficoltà)
// ============================================================================
void HazardManager::update(float dt, float screenWidth, float screenHeight, ParticleSystem& particles)
{
    // Decrementa il timer per la prossima generazione di pericolo.
    m_spawnCooldown -= dt;
    if (m_spawnCooldown <= 0.0f) // Se il timer è scaduto, genera una nuova minaccia
    {
        HazardShot shot; // Istanzia un nuovo descrittore di colpo perimetrale
        // Scelta della direzione di provenienza:
        // - Livelli 1-4: Da sinistra o da destra (2 lati)
        // - Livelli >= 5: Anche dall'alto (3 lati, aggiunge traiettoria verticale discendente)
        int numSides = (m_level >= 5) ? 3 : 2;
        int sideChoice = rand() % numSides; // Seleziona casualmente il lato di ingresso

        if (sideChoice == 0) // Da sinistra verso destra
        {
            shot.side = HazardSide::LEFT; // Imposta lato sinistro
            float y = 70.0f + static_cast<float>(rand() % static_cast<int>(screenHeight - 140.0f)); // Coordinata Y casuale con margine
            shot.pos = glm::vec2(0.0f, y);    // Origine sul bordo sinistro
            shot.dir = glm::vec2(1.0f, 0.0f); // Versore direzione verso destra (+X)
        }
        else if (sideChoice == 1) // Da destra verso sinistra
        {
            shot.side = HazardSide::RIGHT; // Imposta lato destro
            float y = 70.0f + static_cast<float>(rand() % static_cast<int>(screenHeight - 140.0f)); // Coordinata Y casuale con margine
            shot.pos = glm::vec2(screenWidth, y); // Origine sul bordo destro
            shot.dir = glm::vec2(-1.0f, 0.0f);    // Versore direzione verso sinistra (-X)
        }
        else // Da sopra verso il basso (Sbloccato a partire dal livello 5)
        {
            shot.side = HazardSide::TOP; // Imposta lato superiore
            float x = 70.0f + static_cast<float>(rand() % static_cast<int>(screenWidth - 140.0f)); // Coordinata X casuale con margine
            shot.pos = glm::vec2(x, screenHeight); // Origine sul bordo superiore
            shot.dir = glm::vec2(0.0f, -1.0f);     // Versore direzione verso il basso (-Y)
        }

        shot.speed = 880.0f + std::min((m_level - 1) * 35.0f, 220.0f); // Leggero boost di velocità per livello
        shot.radius = 16.0f;       // Raggio del collider circolare del proiettile
        shot.warningTimer = std::max(0.60f, 0.85f - (m_level - 1) * 0.04f); // Finestra di preavviso più serrata nei livelli alti
        shot.fired = false;  // Parte in fase di preavviso (non ancora lanciato)
        shot.active = true;  // Segnala che l'oggetto è attivo

        m_hazards.push_back(shot); // Inserisce il colpo nel vettore attivo

        // Frequenza di ricarica che si accorcia con i livelli:
        float minWait = std::max(1.4f, 3.2f - (m_level - 1) * 0.35f); // Attesa minima progressivamente ridotta
        float randAdd = std::max(0.8f, 2.5f - (m_level - 1) * 0.25f); // Intervallo casuale aggiuntivo
        m_spawnCooldown = minWait + static_cast<float>(rand() % 20) * (randAdd / 20.0f); // Calcola il prossimo cooldown
    }

    // Aggiornamento dello stato dei colpi esistenti:
    for (auto it = m_hazards.begin(); it != m_hazards.end(); )
    {
        if (!it->fired) // Se è ancora in fase di preavviso lampeggiante
        {
            // Fase 1: Conteggio del tempo di preavviso sul bordo.
            it->warningTimer -= dt;
            if (it->warningTimer <= 0.0f) // Allo scadere del preavviso
            {
                it->fired = true; // Scatta il lancio effettivo del proiettile
            }
            ++it; // Avanza al prossimo elemento
        }
        else
        {
            // Fase 2: Integrazione cinematica del volo ad alta velocità.
            it->pos += it->dir * it->speed * dt; // p(t+dt) = p(t) + v*dt

            // Emissione stocastica di scintille rosse come scia del proiettile.
            if (rand() % 3 == 0)
            {
                particles.emitCoinSparkles(it->pos, 2); // Genera 2 particelle luminose in scia
            }

            // Rimuove il proiettile se ha superato completamente i margini dello schermo.
            if (it->pos.x < -80.0f || it->pos.x > screenWidth + 80.0f ||
                it->pos.y < -80.0f || it->pos.y > screenHeight + 80.0f)
            {
                it = m_hazards.erase(it); // Rimuove e restituisce l'iteratore successivo
            }
            else
            {
                ++it; // Avanza al prossimo elemento
            }
        }
    }
}

// ============================================================================
// Rendering dei Segnali di Allerta e dei Proiettili in Volo
// ============================================================================
void HazardManager::draw(const Shader& shader, float currentTime, float screenWidth, float screenHeight)
{
    for (const auto& h : m_hazards) // Itera tutte le minacce attive
    {
        if (!h.fired) // Caso 1: Fase di allerta sul bordo
        {
            // Segnale di preavviso stroboscopico sul bordo:
            // Funzione sinusoidale a 30 rad/s per un effetto visivo d'emergenza intermittente.
            float blink = std::sin(currentTime * 30.0f);
            if (blink > -0.2f) // Mostra il triangolo solo nei picchi positivi dell'onda
            {
                glm::mat4 m = glm::mat4(1.0f); // Matrice identità di partenza
                if (h.side == HazardSide::TOP) // Se proviene dal bordo superiore
                {
                    m = glm::translate(m, glm::vec3(h.pos.x, screenHeight - 20.0f, 0.0f)); // Posiziona vicino al bordo superiore
                    m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1));          // Ruota di 90° per puntare verso il basso
                }
                else // Se proviene da sinistra o destra
                {
                    float posX = (h.side == HazardSide::LEFT) ? 20.0f : screenWidth - 20.0f; // Margine X di 20px dal bordo
                    m = glm::translate(m, glm::vec3(posX, h.pos.y, 0.0f));                   // Trasla alla coordinata verticale
                }
                m = glm::scale(m, glm::vec3(1.6f, 1.8f, 1.0f)); // Scala il segnale triangolare

                // Tinta rosso vivo
                shader.setVec4("uColorTint", glm::vec4(1.0f, 0.1f, 0.1f, 1.0f));
                shader.setMat4("model", m); // Invia la Model Matrix allo shader
                m_warningGeo.draw();        // Disegna il triangolo di allerta
            }
        }
        else // Caso 2: Proiettile in volo
        {
            // Disegno del missile di plasma a rombo:
            float angle = glm::degrees(std::atan2(h.dir.y, h.dir.x)); // Orientamento lungo la direzione di volo
            glm::mat4 m = glm::mat4(1.0f);                            // Matrice identità
            m = glm::translate(m, glm::vec3(h.pos, 0.0f));            // Traslazione alla posizione corrente
            m = glm::rotate(m, glm::radians(angle), glm::vec3(0, 0, 1)); // Rotazione verso la direzione balistica
            m = glm::scale(m, glm::vec3(1.6f, 1.6f, 1.0f));           // Fattore di scala uniforme

            // Bagliore pulsante al plasma
            shader.setVec4("uColorTint", glm::vec4(1.0f, 0.3f, 0.2f, 1.0f));
            shader.setMat4("model", m); // Invia la matrice Model allo shader
            m_projectileGeo.draw();     // Disegna il rombo del proiettile
        }
    }

    // Ripristino tinta shader a bianco neutro
    shader.setVec4("uColorTint", glm::vec4(1.0f));
}

// ============================================================================
// Verifica Impatto Colpo Fatale con il Giocatore
// ============================================================================
bool HazardManager::checkPlayerHit(const CircleCollider& playerCol, ParticleSystem& particles)
{
    for (auto& h : m_hazards) // Scorre tutti i proiettili in scena
    {
        if (h.fired && h.active && Collision2D::checkCircleCircle(playerCol.center, playerCol.radius, h.pos, h.radius))
        {
            h.active = false; // Disattiva il proiettile per evitare danni multipli
            // Innesca un'esplosione particellare massiccia (45 frammenti).
            particles.emitAsteroidExplosion(h.pos, 45);
            return true; // Segnala impatto avvenuto
        }
    }
    return false; // Nessuna collisione rilevata
}
