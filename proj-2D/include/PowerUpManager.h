// ============================================================================
// File: include/PowerUpManager.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Gestore dei power-up e delle capsule energetiche collezionabili (v2).
//              Include Scudo Esagonale, Laser Triplo a Ventaglio e Calamita Magnetica.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Geometry.h"
#include "Shader.h"
#include "Collider2D.h"
#include "Player.h"
#include "ParticleSystem.h"

enum class PowerUpType
{
    NONE = 0,
    SHIELD,       // Scudo energetico deflettore che protegge la navicella da 1 colpo
    SPREAD_SHOT,  // Cannoni alari che sparano a ventaglio (3 laser contemporanei)
    MAGNET        // Attira gravitazionalmente le monete verso la navetta entro 380px
};

struct PowerUpInstance
{
    glm::vec2 pos;
    PowerUpType type;
    float lifetime = 16.0f; // Scompare dopo 16 secondi se non raccolto
    float phase = 0.0f;
    float rotationSpeed = 60.0f;
    float radius = 20.0f;
};

class PowerUpManager
{
public:
    PowerUpManager();
    ~PowerUpManager();

    // Inizializza le geometrie poligonali procedurali del cristallo e del nucleo
    void init();

    // Genera un power-up di un tipo specifico in una posizione assegnata
    void spawn(const glm::vec2& pos, PowerUpType type);

    // Genera proceduralmente un power-up casuale nella mappa giocabile
    void spawnRandom(float screenWidth, float screenHeight);

    // Aggiorna cinematica, rotazione ed oscillazione verticale delle capsule
    void update(float dt);

    // Renderizza le capsule con effetto di rotazione, alone glowing e codice cromatico
    void draw(const Shader& shader, float currentTime);

    // Verifica la collisione con la navicella del giocatore e attiva l'effetto
    PowerUpType checkPlayerCollision(const CircleCollider& playerCol, Player& player, ParticleSystem& particles, int& outScore);

    // Svuota tutti i power-up attivi
    void clear() { m_powerUps.clear(); }

    // Elenco power-up attivi per il radar
    const std::vector<PowerUpInstance>& instances() const { return m_powerUps; }

    static std::string getTypeName(PowerUpType type);
    static glm::vec4 getTypeColor(PowerUpType type);

private:
    std::vector<PowerUpInstance> m_powerUps;
    float m_spawnTimer = 18.0f; // Tempo per il prossimo spawn automatico

    Geometry m_gemGeo;
    Geometry m_coreGeo;
    Geometry m_glowGeo;
};
