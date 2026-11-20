// ============================================================================
// File: include/HazardManager.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Gestore delle minacce ambientali perimetrali (dardi di plasma ad alta velocità).
//              Implementa un pattern telegraph-and-fire:
//              1. Indicatore stroboscopico di preavviso sul bordo (0.85s)
//              2. Lancio del missile di plasma a 880 px/s attraverso lo schermo
//              3. Test di collisione con il giocatore e innesco esplosione
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"
#include "Collider2D.h"
#include "ParticleSystem.h"

// Enumerazione del bordo di provenienza del pericolo spaziale.
enum class HazardSide
{
    LEFT,   // Proviene dal bordo sinistro
    RIGHT,  // Proviene dal bordo destro
    TOP,    // Proviene dal bordo superiore
    BOTTOM  // Proviene dal bordo inferiore
};

// Struttura dati per un singolo pericolo in transito.
struct HazardShot
{
    glm::vec2 pos;      // Posizione 2D corrente
    glm::vec2 dir;      // Versore di direzione del lancio
    float speed;        // Velocità di propagazione (880 px/s)
    float radius;       // Raggio del volume di collisione (16px)
    float warningTimer; // Timer di preavviso lampeggiante prima del lancio
    bool fired;         // true se il proiettile è stato sparato ed è in volo
    bool active;        // true se il proiettile è attivo e pericoloso
    HazardSide side;    // Bordo da cui ha origine il pericolo
};

class HazardManager
{
public:
    // Costruttore predefinito.
    HazardManager();

    // Inizializza le geometrie del dardo di plasma (rombo brillante) e del mirino di allerta.
    void init();

    // Ripristina lo stato del gestore delle minacce.
    void reset();

    // Imposta il livello corrente per scalare la frequenza e le direzioni di attacco.
    void setLevel(int level) { m_level = level; }

    // Aggiorna timer di spawn casuale, animazione dei preavvisi e moto dei proiettili.
    void update(float dt, float screenWidth, float screenHeight, ParticleSystem& particles);

    // Disegna sia gli indicatori di allerta lampeggianti sui bordi sia i dardi di plasma in volo.
    void draw(const Shader& shader, float currentTime, float screenWidth, float screenHeight);

    // Verifica se un dardo di plasma attivo ha colpito il giocatore.
    bool checkPlayerHit(const CircleCollider& playerCol, ParticleSystem& particles);

    // Cancella tutte le minacce attive nella scena.
    void clear() { m_hazards.clear(); }

private:
    int m_level = 1;                    // Livello corrente: incrementa la frequenza e abilita attacchi dall'alto (>= 5)
    float m_spawnCooldown = 3.5f;       // Cooldown tra lanci consecutivi di pericoli
    std::vector<HazardShot> m_hazards;  // Vettore delle minacce attive

    Geometry m_projectileGeo;           // Mesh a rombo per il proiettile di plasma rosso
    Geometry m_warningGeo;              // Mesh rettangolare per l'indicatore stroboscopico di allerta
};
