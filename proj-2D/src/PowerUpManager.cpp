// ============================================================================
// File: src/PowerUpManager.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del gestore dei power-up e delle capsule spaziali.
// ============================================================================

#include "PowerUpManager.h"
#include "ShapeGenerator.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>

PowerUpManager::PowerUpManager()
{
}

PowerUpManager::~PowerUpManager()
{
}

void PowerUpManager::init()
{
    // 1. Geometria del cristallo a diamante esterno
    MeshData gemMesh = ShapeGenerator::createDiamond(
        28.0f, 36.0f,
        glm::vec4(1.0f, 1.0f, 1.0f, 0.95f),
        glm::vec4(0.7f, 0.7f, 0.7f, 0.75f)
    );
    m_gemGeo.setData(gemMesh.positions, gemMesh.colors, gemMesh.primitiveType);
    m_gemGeo.upload();

    // 2. Nucleo interno luminoso
    MeshData coreMesh = ShapeGenerator::createDiamond(
        14.0f, 18.0f,
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        glm::vec4(0.9f, 0.9f, 0.9f, 0.85f)
    );
    m_coreGeo.setData(coreMesh.positions, coreMesh.colors, coreMesh.primitiveType);
    m_coreGeo.upload();

    // 3. Alone di luce radiale per blending additivo
    MeshData glowMesh = ShapeGenerator::createCircle(
        0.0f, 0.0f, 30.0f, 30.0f, 16,
        glm::vec4(1.0f, 1.0f, 1.0f, 0.40f),
        glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)
    );
    m_glowGeo.setData(glowMesh.positions, glowMesh.colors, glowMesh.primitiveType);
    m_glowGeo.upload();
}

std::string PowerUpManager::getTypeName(PowerUpType type)
{
    switch (type)
    {
    case PowerUpType::SHIELD:      return "SCUDO DEFLETTORE";
    case PowerUpType::SPREAD_SHOT: return "LASER TRIPLO";
    case PowerUpType::MAGNET:      return "CALAMITA MONETE";
    default:                       return "SCONOSCIUTO";
    }
}

glm::vec4 PowerUpManager::getTypeColor(PowerUpType type)
{
    switch (type)
    {
    case PowerUpType::SHIELD:      return glm::vec4(0.20f, 0.85f, 1.0f, 1.0f); // Ciano elettrico
    case PowerUpType::SPREAD_SHOT: return glm::vec4(1.0f, 0.45f, 0.15f, 1.0f); // Arancio plasma
    case PowerUpType::MAGNET:      return glm::vec4(0.25f, 1.0f, 0.50f, 1.0f); // Smeraldo magnetico
    default:                       return glm::vec4(1.0f);
    }
}

void PowerUpManager::spawn(const glm::vec2& pos, PowerUpType type)
{
    PowerUpInstance p;
    p.pos = pos;
    p.type = type;
    p.lifetime = 18.0f;
    p.phase = static_cast<float>(rand() % 100) * 0.1f;
    p.rotationSpeed = 45.0f + static_cast<float>(rand() % 40);
    p.radius = 20.0f;

    m_powerUps.push_back(p);
}

void PowerUpManager::spawnRandom(float screenWidth, float screenHeight)
{
    int choice = rand() % 3;
    PowerUpType type = PowerUpType::SHIELD;
    if (choice == 1) type = PowerUpType::SPREAD_SHOT;
    else if (choice == 2) type = PowerUpType::MAGNET;

    float margin = 120.0f;
    float x = margin + static_cast<float>(rand() % static_cast<int>(screenWidth - 2.0f * margin));
    float y = margin + static_cast<float>(rand() % static_cast<int>(screenHeight - 2.0f * margin));

    spawn(glm::vec2(x, y), type);
}

void PowerUpManager::update(float dt)
{
    // Timer per lo spawn automatico occasionale
    m_spawnTimer -= dt;
    if (m_spawnTimer <= 0.0f)
    {
        m_spawnTimer = 22.0f + static_cast<float>(rand() % 10);
        if (m_powerUps.size() < 2)
        {
            spawnRandom(1280.0f, 720.0f);
        }
    }

    // Aggiornamento durata e rimozione di quelli scaduti
    for (auto it = m_powerUps.begin(); it != m_powerUps.end(); )
    {
        it->lifetime -= dt;
        if (it->lifetime <= 0.0f)
        {
            it = m_powerUps.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void PowerUpManager::draw(const Shader& shader, float currentTime)
{
    for (const auto& p : m_powerUps)
    {
        glm::vec4 color = getTypeColor(p.type);
        float bob = std::sin(currentTime * 3.5f + p.phase) * 6.0f;
        glm::vec2 drawPos = p.pos + glm::vec2(0.0f, bob);

        // Effetto lampeggio negli ultimi 3 secondi di vita
        float alphaMult = 1.0f;
        if (p.lifetime < 3.0f)
        {
            alphaMult = (std::sin(currentTime * 14.0f) > 0.0f) ? 1.0f : 0.25f;
        }

        // 1. Alone Glowing Additivo
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float pulse = 1.25f + 0.15f * std::sin(currentTime * 5.0f + p.phase);
        glm::mat4 mGlow = glm::translate(glm::mat4(1.0f), glm::vec3(drawPos, 0.0f));
        mGlow = glm::scale(mGlow, glm::vec3(pulse, pulse, 1.0f));
        shader.setMat4("model", mGlow);
        shader.setVec4("uColorTint", glm::vec4(color.r, color.g, color.b, 0.35f * alphaMult));
        m_glowGeo.draw();

        // 2. Cristallo Esterno Rotante
        float angle = currentTime * p.rotationSpeed;
        glm::mat4 mGem = glm::translate(glm::mat4(1.0f), glm::vec3(drawPos, 0.0f));
        mGem = glm::rotate(mGem, glm::radians(angle), glm::vec3(0, 0, 1));
        shader.setMat4("model", mGem);
        shader.setVec4("uColorTint", glm::vec4(color.r, color.g, color.b, 0.85f * alphaMult));
        m_gemGeo.draw();

        // 3. Nucleo Interno a Rotazione Inversa
        glm::mat4 mCore = glm::translate(glm::mat4(1.0f), glm::vec3(drawPos, 0.0f));
        mCore = glm::rotate(mCore, glm::radians(-angle * 1.5f), glm::vec3(0, 0, 1));
        shader.setMat4("model", mCore);
        shader.setVec4("uColorTint", glm::vec4(1.0f, 1.0f, 1.0f, 0.95f * alphaMult));
        m_coreGeo.draw();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    shader.setVec4("uColorTint", glm::vec4(1.0f));
}

PowerUpType PowerUpManager::checkPlayerCollision(const CircleCollider& playerCol, Player& player, ParticleSystem& particles, int& outScore)
{
    outScore = 0;

    for (auto it = m_powerUps.begin(); it != m_powerUps.end(); ++it)
    {
        float distSq = glm::dot(playerCol.center - it->pos, playerCol.center - it->pos);
        float radSum = playerCol.radius + it->radius;

        if (distSq <= radSum * radSum)
        {
            PowerUpType collected = it->type;

            if (collected == PowerUpType::SHIELD)
            {
                player.activateShield(22.0f);
            }
            else if (collected == PowerUpType::SPREAD_SHOT)
            {
                player.activateSpreadShot(15.0f);
            }
            else if (collected == PowerUpType::MAGNET)
            {
                player.activateMagnet(16.0f);
            }

            particles.emitCoinSparkles(it->pos, 24);
            outScore = 75; // 75 punti bonus

            m_powerUps.erase(it);
            return collected;
        }
    }

    return PowerUpType::NONE;
}
