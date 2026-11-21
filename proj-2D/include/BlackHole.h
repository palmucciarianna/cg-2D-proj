// ============================================================================
// File: include/BlackHole.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Anomalia Gravitazionale (Buco Nero / Singolarità Cosmica v2).
//              Esercita attrazione gravitazionale newtoniana sulla navetta e sugli
//              asteroidi vicini. Modellato con disco di accrescimento rotante ad
//              effetto Doppler cromatico ed orizzonte degli eventi ultra-nero.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include "Geometry.h"
#include "Shader.h"
#include "Player.h"
#include "AsteroidManager.h"

class BlackHole
{
public:
    BlackHole();
    ~BlackHole();

    // Inizializza le geometrie del disco di accrescimento e dell'orizzonte degli eventi
    void init();

    // Aggiorna l'attrazione gravitazionale applicata alla navicella e agli asteroidi
    void update(float dt, Player& player, AsteroidManager& asteroids);

    // Renderizza il buco nero con disco di accrescimento luminoso in rotazione continua
    void draw(const Shader& shader, float currentTime);

    // Getter e Setter
    const glm::vec2& position() const { return m_pos; }
    void setPosition(const glm::vec2& p) { m_pos = p; }

    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }

    float pullRadius() const { return m_pullRadius; }
    float eventHorizonRadius() const { return m_horizonRadius; }

private:
    glm::vec2 m_pos{ 960.0f, 240.0f };
    bool m_active = true;

    float m_horizonRadius = 30.0f; // Raggio dell'orizzonte degli eventi in pixel
    float m_pullRadius = 320.0f;   // Raggio d'influenza del campo gravitazionale
    float m_maxGravityForce = 210.0f; // Forza di attrazione al bordo dell'orizzonte

    Geometry m_horizonGeo; // Orizzonte degli eventi nero opaco
    Geometry m_diskGeo;    // Disco di accrescimento interno
    Geometry m_outerAuraGeo; // Alone di fotoni esterno
};
