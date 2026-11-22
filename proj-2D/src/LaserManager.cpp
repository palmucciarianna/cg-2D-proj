// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: LaserManager.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Gestisce la traiettoria balistica dei raggi plasma, la collisione con gli asteroidi e il drone.
// 2. A CHI È COLLEGATO:
//    - Invocato da GameWorld quando il Player spara (barra spaziatrice); interagisce con AsteroidManager.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - Cinematica: p(t+dt) = p(t) + d * v_laser * dt (Domanda 4).
//    - Orientamento con arcotangente: theta = atan2(dir.y, dir.x) per allineare la Model Matrix (Domanda 6).
//    - Clipping software rispetto alla finestra [0, 1280] x [0, 720] (Domanda 10).
// ============================================================================

#include "LaserManager.h"
#include "HermiteObject.h"
#include "ShapeGenerator.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// ============================================================================
// Costruttore
// ============================================================================
LaserManager::LaserManager()
{
}

// ============================================================================
// Inizializzazione della Geometria Laser Condivisa
// ============================================================================
void LaserManager::init()
{
    // Genera un dardo rettangolare allungato (18px x 6px) con gradiente ciano luminescente.
    MeshData laserData = ShapeGenerator::createRectangle(
        18.0f, 6.0f,
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),  // Nucleo bianco abbagliante
        glm::vec4(0.2f, 0.85f, 1.0f, 1.0f)  // Bordo azzurro plasma
    );

    // Carica la geometria nei buffer della scheda video.
    m_bulletGeo.setData(laserData.positions, laserData.colors, laserData.primitiveType);
    m_bulletGeo.upload();
}

// ============================================================================
// Generazione di un Nuovo Colpo Laser (Sparo)
// ============================================================================
void LaserManager::fire(const glm::vec2& origin, const glm::vec2& direction)
{
    LaserBullet b;
    b.pos = origin;
    // Normalizza il vettore per garantire modulo unitario (lunghezza = 1.0).
    b.dir = glm::normalize(direction);
    // Tempo di volo massimo prima della dissipazione nello spazio: 1.4 secondi.
    b.life = 1.4f;

    m_bullets.push_back(b);
}

// ============================================================================
// Aggiornamento Balistico e Risoluzione Collisioni
// ============================================================================
void LaserManager::update(float dt, AsteroidManager& asteroids, HermiteManager& hermite, ParticleSystem& particles, int& score)
{
    // Velocità di propagazione del colpo laser in pixel/secondo.
    const float speed = 720.0f;

    // Itera sui proiettili con iteratore per consentire la cancellazione sicura (erase).
    for (auto it = m_bullets.begin(); it != m_bullets.end(); )
    {
        // 1. Spostamento rettilineo uniforme: pos = pos + dir * v * dt.
        it->pos += it->dir * speed * dt;

        // 2. Decremento del tempo di vita residuo.
        it->life -= dt;

        // 3. Verifica della collisione con l'Alien Sentinel Drone (quello Rosso):
        // Se colpito, il Drone si indebolisce, rallenta drasticamente e annulla gli scatti aggressivi!
        if (hermite.checkLaserHit(it->pos, 6.0f, particles, score))
        {
            it = m_bullets.erase(it); // Rimuove il proiettile andato a segno
            continue;
        }

        // 4. Verifica della collisione con gli asteroidi nella scena:
        // Raggio d'impatto del proiettile = 6.0 pixel.
        if (asteroids.checkLaserHit(it->pos, 6.0f, particles))
        {
            score += 50; // Assegna 50 punti bonus per ogni asteroide distrutto
            it = m_bullets.erase(it); // Rimuove il proiettile andato a segno
            continue;
        }

        // 5. Se il proiettile ha esaurito il tempo di vita, lo dissolve.
        if (it->life <= 0.0f)
        {
            it = m_bullets.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// ============================================================================
// Disegno dei Dardi Laser
// ============================================================================
void LaserManager::draw(const Shader& shader)
{
    for (const auto& b : m_bullets)
    {
        // Calcola l'angolo di rotazione del dardo per allinearlo alla traiettoria di volo:
        // theta = atan2(dir.y, dir.x) in gradi sessagesimali.
        float angle = glm::degrees(std::atan2(b.dir.y, b.dir.x));

        // Calcola la Model Matrix M = T * R.
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(b.pos, 0.0f));
        m = glm::rotate(m, glm::radians(angle), glm::vec3(0, 0, 1));

        // Invia la matrice alla uniform e disegna la mesh.
        shader.setMat4("model", m);
        m_bulletGeo.draw();
    }
}
