// ============================================================================
// File: src/CosmicDust.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del sistema di pulviscolo stellare reattivo (v2).
// ============================================================================

#include "CosmicDust.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>

CosmicDust::CosmicDust()
{
}

CosmicDust::~CosmicDust()
{
}

void CosmicDust::init(float screenWidth, float screenHeight, int count)
{
    m_width = screenWidth;
    m_height = screenHeight;
    m_motes.clear();
    m_motes.reserve(count);

    for (int i = 0; i < count; ++i)
    {
        DustMote m;
        m.pos.x = static_cast<float>(rand() % static_cast<int>(screenWidth));
        m.pos.y = static_cast<float>(rand() % static_cast<int>(screenHeight));
        m.basePos = m.pos;
        m.phase = static_cast<float>(rand() % 628) * 0.01f;
        m.driftSpeed = 6.0f + static_cast<float>(rand() % 12);
        m.baseGlow = 0.12f + static_cast<float>(rand() % 15) * 0.01f;
        m.glow = m.baseGlow;
        m.size = 2.2f + static_cast<float>(rand() % 3) * 0.6f;
        m_motes.push_back(m);
    }

    // Geometria del granello: piccolo rombo/diamante a 4 vertici
    std::vector<glm::vec3> positions = {
        glm::vec3(0.0f,  1.5f, 0.0f),
        glm::vec3(-1.5f, 0.0f, 0.0f),
        glm::vec3(1.5f,  0.0f, 0.0f),

        glm::vec3(-1.5f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.5f, 0.0f),
        glm::vec3(1.5f,  0.0f, 0.0f)
    };

    std::vector<glm::vec4> colors(6, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    m_moteGeo.setData(positions, colors, GL_TRIANGLES);
    m_moteGeo.upload();
}

void CosmicDust::update(float dt, float currentTime, const glm::vec2& playerPos, float shipAngle, bool headlightsActive, const glm::vec2& blackHolePos, bool blackHoleActive)
{
    float shipRad = glm::radians(shipAngle);
    glm::vec2 shipForward(std::cos(shipRad), std::sin(shipRad));

    for (auto& m : m_motes)
    {
        // 1. Oscillazione armonica di deriva browniana lenta
        m.pos.x = m.basePos.x + std::sin(currentTime * 0.6f + m.phase) * (m.driftSpeed * 1.5f);
        m.pos.y = m.basePos.y + std::cos(currentTime * 0.4f + m.phase * 1.3f) * (m.driftSpeed * 1.2f);

        // Clamping / wrapping dello schermo per mantenere i granelli sempre presenti
        if (m.pos.x < 0.0f) m.pos.x += m_width;
        if (m.pos.x > m_width) m.pos.x -= m_width;
        if (m.pos.y < 0.0f) m.pos.y += m_height;
        if (m.pos.y > m_height) m.pos.y -= m_height;

        // 2. Interazione di prossimita' con la navicella
        float distToPlayer = glm::distance(m.pos, playerPos);
        if (distToPlayer < 120.0f)
        {
            float targetGlow = 0.85f * (1.0f - distToPlayer / 120.0f);
            m.glow = std::max(m.glow, targetGlow);
        }

        // 3. Interazione con i fari conici volumetrici
        if (headlightsActive && distToPlayer < 430.0f && distToPlayer > 10.0f)
        {
            glm::vec2 toMote = glm::normalize(m.pos - playerPos);
            float dot = glm::dot(shipForward, toMote);
            // Angolo conico: cos(34 gradi) ~ 0.829
            if (dot > 0.82f)
            {
                float coneFactor = (dot - 0.82f) / 0.18f;
                float falloff = 1.0f - (distToPlayer / 430.0f);
                float beamGlow = 0.95f * coneFactor * falloff;
                m.glow = std::max(m.glow, beamGlow);
            }
        }

        // 4. Interazione con il buco nero
        if (blackHoleActive)
        {
            float distToHole = glm::distance(m.pos, blackHolePos);
            if (distToHole < 280.0f && distToHole > 20.0f)
            {
                float holeGlow = 0.80f * (1.0f - distToHole / 280.0f);
                m.glow = std::max(m.glow, holeGlow);
            }
        }

        // 5. Decadimento graduale della luminosita' verso il valore base di riposo
        if (m.glow > m.baseGlow)
        {
            m.glow = std::max(m.baseGlow, m.glow - dt * 1.8f);
        }
    }
}

void CosmicDust::draw(const Shader& shader)
{
    // Abilita il blending additivo per far brillare la polvere cosmica sul nero dello spazio
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (const auto& m : m_motes)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(m.pos, 0.0f));
        float currentSize = m.size * (0.8f + m.glow * 0.8f);
        model = glm::scale(model, glm::vec3(currentSize, currentSize, 1.0f));

        // Tint cromatico in base all'eccitazione energetica:
        // Riposo: ciano/azzurro delicato | Eccitato: bianco/oro scintillante
        glm::vec4 tint;
        if (m.glow > 0.45f)
        {
            tint = glm::vec4(1.0f, 0.95f, 0.70f, m.glow); // Oro caldo brillante
        }
        else
        {
            tint = glm::vec4(0.40f, 0.75f, 1.0f, m.glow * 0.8f); // Ciano stellare flebile
        }

        shader.setMat4("model", model);
        shader.setVec4("uColorTint", tint);
        m_moteGeo.draw();
    }

    // Ripristina il blending standard
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
