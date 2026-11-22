// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: AsteroidManager.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Genera e anima la cintura di asteroidi rotanti (Requisito 4).
//    - Implementa la tecnica del Buffer Sharing: 1 solo VBO/VAO per tutti gli asteroidi.
// 2. A CHI È COLLEGATO:
//    - Coordinato da GameWorld; interagisce con LaserManager e ParticleSystem.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - Buffer Sharing su VRAM con GL_STATIC_DRAW (Domanda 17).
//    - Invio uniforme Model Matrix CPU -> GPU: M_i = T * R * S per ogni istanza (Domande 6, 18).
//    - Distribuzione in coordinate polari: x = cx + r*cos(theta), y = cy + r*sin(theta) (Domanda 5).
// ============================================================================

#include "AsteroidManager.h"
#include "ShapeGenerator.h"
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
    m_asteroidGeo.setData(asteroidData.positions, asteroidData.colors, asteroidData.primitiveType);
    m_asteroidGeo.upload();
}

// ============================================================================
// Generazione Procedurale delle Istanze del Livello
// ============================================================================
void AsteroidManager::spawnLevel(int levelCount, float screenWidth, float screenHeight)
{
    // Svuota le istanze del livello precedente.
    m_asteroids.clear();

    // Calcola il numero di asteroidi da generare in base alla difficoltà:
    // Livello 1: 4 asteroidi; cresce di 2 a livello fino a un tetto massimo di 9.
    int count = 4 + (levelCount - 1) * 2;
    if (count > 9) count = 9;

    for (int i = 0; i < count; ++i)
    {
        Asteroid a;

        // Distribuzione polare attorno al centro dello schermo:
        // Evita che gli asteroidi compaiano direttamente sopra la navicella del giocatore.
        float angle = (i * 6.28318f) / count + (rand() % 100) * 0.005f;
        float dist = 220.0f + (rand() % 260);

        // Calcola le coordinate cartesiane (x, y) a partire da raggio e angolo polare.
        a.pos = glm::vec2(
            screenWidth * 0.5f + std::cos(angle) * dist,
            screenHeight * 0.5f + std::sin(angle) * dist
        );

        // Clamping di sicurezza per mantenere le istanze all'interno dei bordi del display.
        a.pos.x = glm::clamp(a.pos.x, 80.0f, screenWidth - 80.0f);
        a.pos.y = glm::clamp(a.pos.y, 80.0f, screenHeight - 80.0f);

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
        if (a.active)
        {
            // Test di intersezione cerchio-cerchio privo di sqrt():
            float distSq = glm::dot(bulletPos - a.pos, bulletPos - a.pos);
            float radSum = bulletRadius + a.radius;

            if (distSq <= radSum * radSum)
            {
                // Impatto confermato:
                // 1. Disattiva l'asteroide rimuovendolo dalla scena.
                a.active = false;
                // 2. Genera un'esplosione fisica particellare di 32 schegge rocciose.
                particles.emitAsteroidExplosion(a.pos, 32);
                return true;
            }
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
        if (a.active)
        {
            // Verifica la distanza quadratica rispetto al collider del giocatore.
            float distSq = glm::dot(playerCol.center - a.pos, playerCol.center - a.pos);
            float radSum = playerCol.radius + a.radius;

            if (distSq <= radSum * radSum)
            {
                return true; // Collisione rilevata
            }
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
