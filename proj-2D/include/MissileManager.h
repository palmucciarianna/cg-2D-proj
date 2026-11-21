// ============================================================================
// File: include/MissileManager.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Gestore balistico dei Missili Intelligenti a Ricerca Termica (v2).
//              Implementa cinematica a guida autonoma proporzionale (Pursuit Steering),
//              scia di fumo particellare e detonazione a raggio d'impatto.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"
#include "ParticleSystem.h"
#include "AsteroidManager.h"

class HermiteManager;

struct HomingMissile
{
    glm::vec2 pos;        // Posizione istantanea nel mondo 2D
    glm::vec2 vel;        // Vettore velocita' istantanea
    float angle = 0.0f;   // Orientamento angolare in gradi
    float speed = 440.0f; // Velocita' di crociera (px/s)
    float turnRate = 4.2f;// Reattivita' angolare di inseguimento (rad/s)
    float life = 4.5f;    // Durata massima del missile prima dell'auto-distruzione
    bool active = true;
};

class MissileManager
{
public:
    MissileManager();
    ~MissileManager();

    // Inizializza la geometria aerodinamica del missile (corpo appuntito con alette rosse/bianche)
    void init();

    // Lancia un missile da una posizione assegnata lungo il versore iniziale specificato
    void fire(const glm::vec2& origin, const glm::vec2& direction);

    // Aggiorna cinematica, guida autonoma pursuit verso il target piu' vicino, scia di fumo e impatti
    void update(float dt, AsteroidManager& asteroids, HermiteManager& hermite, ParticleSystem& particles, int& score);

    // Renderizza tutti i missili attivi orientandoli nella direzione della velocita'
    void draw(const Shader& shader);

    // Rimuove tutti i missili attivi
    void clear() { m_missiles.clear(); }

    // Conteggio missili attualmente in volo
    int activeCount() const;

    // Timer e cooldown per il lanciatore alare
    bool canFire() const { return m_cooldownTimer <= 0.0f; }
    void resetCooldown(float cd = 0.75f) { m_cooldownTimer = cd; }
    float cooldownRemaining() const { return m_cooldownTimer; }

private:
    std::vector<HomingMissile> m_missiles;
    Geometry m_missileGeo;
    float m_cooldownTimer = 0.0f;
    float m_trailTimer = 0.0f;
};
