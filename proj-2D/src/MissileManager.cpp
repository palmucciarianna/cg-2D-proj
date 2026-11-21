// ============================================================================
// File: src/MissileManager.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del gestore dei missili a ricerca termica (v2).
// ============================================================================

#include "MissileManager.h"
#include "HermiteObject.h"
#include "AudioEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

MissileManager::MissileManager()
{
}

MissileManager::~MissileManager()
{
}

void MissileManager::init()
{
    // Geometria del missile: Ogiva anteriore rossa, corpo cilindrico bianco e alette stabilizzatrici
    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;

    // Colori
    glm::vec4 tipColor(1.0f, 0.25f, 0.20f, 1.0f);   // Ogiva rossa fluorescente
    glm::vec4 bodyColor(0.92f, 0.94f, 0.98f, 1.0f); // Fusoliera bianca/grigio perla
    glm::vec4 finColor(0.35f, 0.40f, 0.50f, 1.0f);  // Alette stabilizzatrici scure

    // 1. Ogiva aerodinamica a punta (Triangolo superiore)
    // Vertici locali (centrati, orientati verso l'alto +Y):
    // Punta: (0, 18), Base sx: (-4, 8), Base dx: (4, 8)
    positions.push_back(glm::vec3(0.0f, 18.0f, 0.0f));  colors.push_back(tipColor);
    positions.push_back(glm::vec3(-4.0f, 8.0f, 0.0f));  colors.push_back(tipColor);
    positions.push_back(glm::vec3(4.0f, 8.0f, 0.0f));   colors.push_back(tipColor);

    // 2. Corpo cilindrico (Rettangolo da y=8 a y=-10, larghezza 8px)
    // Triangolo 1:
    positions.push_back(glm::vec3(-4.0f, 8.0f, 0.0f));  colors.push_back(bodyColor);
    positions.push_back(glm::vec3(-4.0f, -10.0f, 0.0f)); colors.push_back(bodyColor);
    positions.push_back(glm::vec3(4.0f, 8.0f, 0.0f));   colors.push_back(bodyColor);

    // Triangolo 2:
    positions.push_back(glm::vec3(4.0f, 8.0f, 0.0f));   colors.push_back(bodyColor);
    positions.push_back(glm::vec3(-4.0f, -10.0f, 0.0f)); colors.push_back(bodyColor);
    positions.push_back(glm::vec3(4.0f, -10.0f, 0.0f));  colors.push_back(bodyColor);

    // 3. Aletta sinistra (Triangolo da y=-4 a y=-12, sporgente fino a x=-9)
    positions.push_back(glm::vec3(-4.0f, -4.0f, 0.0f));  colors.push_back(finColor);
    positions.push_back(glm::vec3(-9.0f, -12.0f, 0.0f)); colors.push_back(finColor);
    positions.push_back(glm::vec3(-4.0f, -10.0f, 0.0f)); colors.push_back(finColor);

    // 4. Aletta destra (Triangolo da y=-4 a y=-12, sporgente fino a x=9)
    positions.push_back(glm::vec3(4.0f, -4.0f, 0.0f));   colors.push_back(finColor);
    positions.push_back(glm::vec3(4.0f, -10.0f, 0.0f));  colors.push_back(finColor);
    positions.push_back(glm::vec3(9.0f, -12.0f, 0.0f));  colors.push_back(finColor);

    m_missileGeo.setData(positions, colors, GL_TRIANGLES);
    m_missileGeo.upload();
}

void MissileManager::fire(const glm::vec2& origin, const glm::vec2& direction)
{
    HomingMissile m;
    m.pos = origin;
    m.vel = glm::normalize(direction) * m.speed;
    m.angle = glm::degrees(std::atan2(direction.y, direction.x));
    m.life = 4.5f;
    m.active = true;

    m_missiles.push_back(m);
}

void MissileManager::update(float dt, AsteroidManager& asteroids, HermiteManager& hermite, ParticleSystem& particles, int& score)
{
    if (m_cooldownTimer > 0.0f)
    {
        m_cooldownTimer -= dt;
    }

    m_trailTimer += dt;
    bool emitTrail = false;
    if (m_trailTimer >= 0.045f)
    {
        emitTrail = true;
        m_trailTimer = 0.0f;
    }

    const auto& astList = asteroids.asteroids();
    glm::vec2 dronePos = hermite.dronePosition();

    for (auto& m : m_missiles)
    {
        if (!m.active) continue;

        m.life -= dt;
        if (m.life <= 0.0f)
        {
            m.active = false;
            particles.emitAsteroidExplosion(m.pos, 10);
            continue;
        }

        // --------------------------------------------------------------------
        // Guida Autonoma: Ricerca del bersaglio piu' vicino (Pursuit Steering)
        // --------------------------------------------------------------------
        glm::vec2 closestTarget(0.0f);
        float minTargetDist = 850.0f; // Raggio massimo di aggancio radar
        bool targetFound = false;

        // 1. Valuta gli asteroidi attivi
        for (const auto& ast : astList)
        {
            if (!ast.active) continue;
            float d = glm::distance(m.pos, ast.pos);
            if (d < minTargetDist)
            {
                minTargetDist = d;
                closestTarget = ast.pos;
                targetFound = true;
            }
        }

        // 2. Valuta il Drone Sentinella Rosso (priorita' aggancio)
        float droneDist = glm::distance(m.pos, dronePos);
        if (droneDist < minTargetDist * 1.15f)
        {
            minTargetDist = droneDist;
            closestTarget = dronePos;
            targetFound = true;
        }

        // Se un bersaglio e' agganciato, corregge la traiettoria angolare
        if (targetFound)
        {
            glm::vec2 toTarget = glm::normalize(closestTarget - m.pos);
            glm::vec2 curDir = glm::normalize(m.vel);

            // Prodotto vettoriale 2D per determinare se virare in senso orario o antiorario
            float cross = curDir.x * toTarget.y - curDir.y * toTarget.x;
            float dot = glm::clamp(glm::dot(curDir, toTarget), -1.0f, 1.0f);
            float steerAngle = std::acos(dot);

            float maxSteer = m.turnRate * dt;
            float applySteer = std::min(steerAngle, maxSteer);

            if (cross < 0.0f) applySteer = -applySteer;

            // Rotazione del vettore velocita'
            float currentAngRad = std::atan2(curDir.y, curDir.x);
            float newAngRad = currentAngRad + applySteer;
            m.vel = glm::vec2(std::cos(newAngRad), std::sin(newAngRad)) * m.speed;
            m.angle = glm::degrees(newAngRad);
        }

        // Integrazione della posizione
        m.pos += m.vel * dt;

        // Scia particellare di fumo e scintille di propulsione
        if (emitTrail)
        {
            glm::vec2 exhaustPos = m.pos - glm::normalize(m.vel) * 12.0f;
            particles.emitThrusterTrail(exhaustPos, glm::normalize(m.vel), 0.75f);
        }

        // --------------------------------------------------------------------
        // Controllo Collisioni con Asteroidi
        // --------------------------------------------------------------------
        for (size_t i = 0; i < astList.size(); ++i)
        {
            const auto& ast = astList[i];
            if (!ast.active) continue;

            if (glm::distance(m.pos, ast.pos) < (ast.radius + 14.0f))
            {
                // Impatto! Distrugge l'asteroide ed esplode il missile
                asteroids.checkLaserHit(m.pos, 16.0f, particles);
                m.active = false;
                score += 80;
                particles.emitAsteroidExplosion(m.pos, 32);
                AudioEngine::instance().playExplosion();
                break;
            }
        }

        if (!m.active) continue;

        // --------------------------------------------------------------------
        // Controllo Collisione con Drone Sentinella Rosso
        // --------------------------------------------------------------------
        if (glm::distance(m.pos, dronePos) < 42.0f)
        {
            hermite.checkLaserHit(m.pos, 18.0f, particles, score);
            m.active = false;
            score += 100;
            particles.emitDroneHitSparks(m.pos, 28);
            AudioEngine::instance().playExplosion();
        }
    }

    // Rimuove i missili inattivi
    m_missiles.erase(
        std::remove_if(m_missiles.begin(), m_missiles.end(), [](const HomingMissile& m) { return !m.active; }),
        m_missiles.end()
    );
}

void MissileManager::draw(const Shader& shader)
{
    for (const auto& m : m_missiles)
    {
        if (!m.active) continue;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(m.pos, 0.0f));
        model = glm::rotate(model, glm::radians(m.angle - 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.0f));

        shader.setMat4("model", model);
        shader.setVec4("uColorTint", glm::vec4(1.0f));
        m_missileGeo.draw();
    }
}

int MissileManager::activeCount() const
{
    int count = 0;
    for (const auto& m : m_missiles) if (m.active) count++;
    return count;
}
