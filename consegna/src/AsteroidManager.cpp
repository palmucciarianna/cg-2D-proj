// ============================================================================
// File: src/AsteroidManager.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del gestore degli asteroidi a geometria condivisa.
//              Dimostra l'efficienza della pipeline OpenGL:
//              1 sola allocazione VBO/VAO per N oggetti distinti nello spazio,
//              differenziati a runtime tramite la Model Matrix Uniform.
// ============================================================================

#include "AsteroidManager.h"
#include "ShapeGenerator.h"
#include "MathConstants.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>

// ============================================================================
// Costruttore
// ============================================================================
AsteroidManager::AsteroidManager()
{
}

// ============================================================================
// Inizializzazione della Geometria Condivisa (Requisito 4)
// ============================================================================
void AsteroidManager::init()
{
    // Genera una mesh CPU circolare irregolare a 13 segmenti con gradiente roccioso
    // (nucleo grigio medio e bordo basaltico scuro).
    MeshData asteroidData = ShapeGenerator::createCircle(
        0.0f, 0.0f, 32.0f, 28.0f, 13,
        glm::vec4(0.58f, 0.54f, 0.50f, 1.0f),
        glm::vec4(0.24f, 0.22f, 0.20f, 1.0f)
    );

    // Trasferisce i vertici dell'asteroide in memoria VRAM della GPU una sola volta.
    m_asteroidGeo.uploadMesh(asteroidData);
}

// ============================================================================
// Generazione Procedurale delle Istanze del Livello
// ============================================================================
void AsteroidManager::spawnLevel(int levelCount, float screenWidth, float screenHeight)
{
    // Svuota le istanze del livello precedente.
    m_asteroids.clear();

    // Calcola il numero di asteroidi da generare in base alla difficoltà:
    // Livello 1: 4 asteroidi; cresce di 2 a livello fino a un tetto massimo di 10.
    int count = 4 + (levelCount - 1) * 2;
    if (count > 10) count = 10;

    for (int i = 0; i < count; ++i)
    {
        Asteroid a;

        // Distribuzione polare attorno al centro dello schermo (distanza 220..480px + piccolo scarto angolare):
        // evita che gli asteroidi compaiano sopra la navicella e applica clamping ai bordi per non tagliare le rocce.
        a.pos = MathConstants::sampleRingPosition(i, count, 220.0f, 480.0f, 0.50f, screenWidth, screenHeight);

        // Angolo di rotazione iniziale casuale [0°, 360°].
        a.rotation = static_cast<float>(rand() % 360);

        // Velocità angolare casuale con direzione oraria o antioraria casuale.
        a.rotSpeed = (rand() % 2 == 0 ? 1.0f : -1.0f) * (20.0f + rand() % 45);

        // Fattore di scalatura eterogeneo per dare varietà visiva e dimensionale.
        a.scale = 0.9f + (rand() % 80) * 0.01f;

        // Raggio effettivo del collider proporzionale alla scala applicata.
        a.radius = 30.0f * a.scale;

        // Contrassegna l'istanza come attiva e integra.
        a.active = true;

        // Aggiunge l'istanza al vettore.
        m_asteroids.push_back(a);
    }
}

// ============================================================================
// Aggiornamento della Rotazione delle Istanze
// ============================================================================
void AsteroidManager::update(float dt)
{
    for (auto& a : m_asteroids)
    {
        // Se l'asteroide non è stato frantumato, incrementa la rotazione col tempo.
        if (a.active)
        {
            a.rotation += a.rotSpeed * dt;
        }
    }
}

// ============================================================================
// Rendering ad Istanze Condivise (Shared Buffer Draw)
// ============================================================================
void AsteroidManager::draw(const Shader& shader, float currentTime)
{
    (void)currentTime;

    // Itera su tutte le istanze attive:
    // Ciascun asteroide calcola la propria Model Matrix M = T * R * S,
    // la invia come Uniform allo shader e invoca m_asteroidGeo.draw().
    // La GPU riutilizza lo stesso VAO e gli stessi vertici senza overhead.
    for (const auto& a : m_asteroids)
    {
        if (a.active)
        {
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(a.pos, 0.0f));
            m = glm::rotate(m, glm::radians(a.rotation), glm::vec3(0, 0, 1));
            m = glm::scale(m, glm::vec3(a.scale, a.scale, 1.0f));

            // Aggiorna la variabile uniform 'model' nello shader attivo.
            shader.setMat4("model", m);

            // Emette la draw call per l'istanza corrente.
            m_asteroidGeo.draw();
        }
    }
}

// ============================================================================
// Rilevamento Impatto Colpo Laser
// ============================================================================
bool AsteroidManager::checkLaserHit(const glm::vec2& bulletPos, float bulletRadius, ParticleSystem& particles)
{
    for (auto& a : m_asteroids)
    {
        if (a.active && Collision2D::checkCircleCircle(bulletPos, bulletRadius, a.pos, a.radius))
        {
            // Impatto confermato:
            // 1. Disattiva l'asteroide rimuovendolo dalla scena.
            a.active = false;
            // 2. Genera un'esplosione fisica particellare di 32 schegge rocciose.
            particles.emitAsteroidExplosion(a.pos, 32);
            return true;
        }
    }
    return false;
}

// ============================================================================
// Rilevamento Impatto con la Navicella
// ============================================================================
bool AsteroidManager::checkPlayerCollision(const CircleCollider& playerCol)
{
    for (auto& a : m_asteroids)
    {
        if (a.active && Collision2D::checkCircleCircle(playerCol.center, playerCol.radius, a.pos, a.radius))
        {
            return true; // Collisione rilevata
        }
    }
    return false;
}

// ============================================================================
// Conteggio Istanze Attive
// ============================================================================
int AsteroidManager::activeCount() const
{
    int c = 0;
    for (const auto& a : m_asteroids)
    {
        if (a.active) c++;
    }
    return c;
}
