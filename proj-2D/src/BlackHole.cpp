// ============================================================================
// File: src/BlackHole.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione dell'anomalia gravitazionale del Buco Nero (v2).
// ============================================================================

#include "BlackHole.h"
#include "ShapeGenerator.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

BlackHole::BlackHole()
{
}

BlackHole::~BlackHole()
{
}

void BlackHole::init()
{
    // 1. Orizzonte degli Eventi (Singolarita' ultra-oscura)
    MeshData horizonMesh = ShapeGenerator::createCircle(
        0.0f, 0.0f, 28.0f, 28.0f, 32,
        glm::vec4(0.0f, 0.0f, 0.02f, 1.0f),
        glm::vec4(0.09f, 0.01f, 0.18f, 1.0f)
    );
    m_horizonGeo.setData(horizonMesh.positions, horizonMesh.colors, horizonMesh.primitiveType);
    m_horizonGeo.upload();

    // 2. Disco di Accrescimento ellittico caldo
    MeshData diskMesh = ShapeGenerator::createCircle(
        0.0f, 0.0f, 62.0f, 32.0f, 32,
        glm::vec4(0.95f, 0.35f, 1.0f, 0.85f),
        glm::vec4(0.35f, 0.08f, 0.65f, 0.08f)
    );
    m_diskGeo.setData(diskMesh.positions, diskMesh.colors, diskMesh.primitiveType);
    m_diskGeo.upload();

    // 3. Alone di fotoni rotante esterno
    MeshData auraMesh = ShapeGenerator::createCircle(
        0.0f, 0.0f, 85.0f, 85.0f, 24,
        glm::vec4(0.6f, 0.15f, 0.9f, 0.28f),
        glm::vec4(0.2f, 0.0f, 0.4f, 0.0f)
    );
    m_outerAuraGeo.setData(auraMesh.positions, auraMesh.colors, auraMesh.primitiveType);
    m_outerAuraGeo.upload();
}

void BlackHole::update(float dt, Player& player, AsteroidManager& asteroids)
{
    (void)asteroids;
    if (!m_active) return;

    // Attrazione newtoniana applicata alla navicella del giocatore:
    float dist = glm::distance(player.position(), m_pos);
    if (dist > 0.001f && dist <= m_pullRadius)
    {
        glm::vec2 pullDir = glm::normalize(m_pos - player.position());
        float factor = 1.0f - (dist / m_pullRadius);
        // Andamento quadratico inverso per simulare legge di gravitazione universale
        float force = factor * factor * m_maxGravityForce;
        player.addVelocity(pullDir * force * dt);

        // Contatto letale con la singolarità dell'orizzonte degli eventi
        if (dist <= m_horizonRadius + 14.0f)
        {
            player.takeDamage();
        }
    }
}

void BlackHole::draw(const Shader& shader, float currentTime)
{
    if (!m_active) return;

    // 1. Rendering Alone di fotoni e disco di accrescimento (Additive Blending)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Alone esterno pulsante
    float auraPulse = 1.0f + 0.08f * std::sin(currentTime * 3.5f);
    glm::mat4 mAura = glm::translate(glm::mat4(1.0f), glm::vec3(m_pos, 0.0f));
    mAura = glm::scale(mAura, glm::vec3(auraPulse, auraPulse, 1.0f));
    shader.setMat4("model", mAura);
    shader.setVec4("uColorTint", glm::vec4(1.0f));
    m_outerAuraGeo.draw();

    // Disco di accrescimento rotante
    float rotAngle = currentTime * 55.0f;
    glm::mat4 mDisk = glm::translate(glm::mat4(1.0f), glm::vec3(m_pos, 0.0f));
    mDisk = glm::rotate(mDisk, glm::radians(rotAngle), glm::vec3(0, 0, 1));
    shader.setMat4("model", mDisk);
    shader.setVec4("uColorTint", glm::vec4(1.0f));
    m_diskGeo.draw();

    // Secondo anello con inclinazione opposta per vortice volumetrico
    glm::mat4 mDisk2 = glm::translate(glm::mat4(1.0f), glm::vec3(m_pos, 0.0f));
    mDisk2 = glm::rotate(mDisk2, glm::radians(-rotAngle * 0.7f + 45.0f), glm::vec3(0, 0, 1));
    mDisk2 = glm::scale(mDisk2, glm::vec3(0.8f, 0.8f, 1.0f));
    shader.setMat4("model", mDisk2);
    m_diskGeo.draw();

    // 2. Ripristino blending per l'Orizzonte degli Eventi (Nero Assoluto)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glm::mat4 mHorizon = glm::translate(glm::mat4(1.0f), glm::vec3(m_pos, 0.0f));
    shader.setMat4("model", mHorizon);
    shader.setVec4("uColorTint", glm::vec4(1.0f));
    m_horizonGeo.draw();
}
