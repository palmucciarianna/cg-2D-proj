// ============================================================================
// File: include/CosmicDust.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Sistema di pulviscolo stellare reattivo in tempo reale (v2).
//              Micro-particelle spaziali ambientali fluttuanti con moto browniano
//              che si illuminano all'avvicinarsi della navicella, dei fari o dei laser.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"

struct DustMote
{
    glm::vec2 pos;
    glm::vec2 basePos;
    float phase = 0.0f;
    float driftSpeed = 12.0f;
    float glow = 0.15f;      // Luminosita' corrente [0.15..1.0]
    float baseGlow = 0.15f;  // Luminosita' di riposo
    float size = 3.0f;
};

class CosmicDust
{
public:
    CosmicDust();
    ~CosmicDust();

    // Inizializza 160 granelli di polvere cosmica distribuiti casualmente e crea la mesh del micro-diamante
    void init(float screenWidth, float screenHeight, int count = 160);

    // Aggiorna l'oscillazione armonica e verifica la vicinanza a navicella, fari conici e buco nero
    void update(float dt, float currentTime, const glm::vec2& playerPos, float shipAngle, bool headlightsActive, const glm::vec2& blackHolePos, bool blackHoleActive);

    // Renderizza il pulviscolo con Additive Blending hardware GL_SRC_ALPHA, GL_ONE
    void draw(const Shader& shader);

private:
    std::vector<DustMote> m_motes;
    Geometry m_moteGeo;
    float m_width = 1280.0f;
    float m_height = 720.0f;
};
