// ============================================================================
// File: include/AsteroidManager.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Gestore delle istanze condivise degli asteroidi (Requisito 4).
//              Tutti gli asteroidi condividono la STESSA risorsa geometrica su VRAM
//              (un unico VAO/VBO), riducendo a zero la duplicazione di memoria GPU
//              e variando per ogni istanza unicamente la matrice di modellazione (Model Matrix).
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"
#include "Collider2D.h"
#include "ParticleSystem.h"

// Struttura dati CPU per una singola istanza di asteroide nel mondo di gioco.
struct Asteroid
{
    glm::vec2 pos;      // Posizione 2D nel sistema di coordinate del mondo
    float rotation;     // Angolo di rotazione corrente attorno al proprio baricentro (in gradi)
    float rotSpeed;     // Velocità angolare di rotazione in gradi/secondo (oraria o antioraria)
    float scale;        // Fattore di scala dimensionale (variazione di grandezza)
    float radius;       // Raggio effettivo del collider circolare (30px * scale)
    bool active = true; // Stato di vita: true = intatto e visibile, false = distrutto dal laser
    bool isCrystal = false;    // Asteroide cristallino raro con riflessi prismatici (v2)
    glm::vec2 velocity{ 0.0f }; // Velocita' cinematica per impulsi fisici EMP (v2)
};

class AsteroidManager
{
public:
    // Costruttore: inizializza il gestore delle istanze.
    AsteroidManager();

    // Alloca e carica sulla GPU l'unica geometria condivisa dell'asteroide (13 segmenti conici).
    void init();

    // Genera proceduralmente gli asteroidi distribuiti a raggiera attorno al punto di spawn del giocatore.
    void spawnLevel(int levelCount, float screenWidth, float screenHeight);

    // Aggiorna l'angolo di rotazione di ciascun asteroide attivo integrando la velocità angolare col delta-time.
    void update(float dt);

    // Renderizza tutti gli asteroidi attivi iterando sull'unica mesh condivisa e impostando le matrici Uniform.
    void draw(const Shader& shader, float currentTime);

    // Verifica la collisione tra un colpo laser del giocatore e gli asteroidi:
    // in caso di impatto, disattiva l'asteroide ed emette un'esplosione di 32 frammenti rocciosi.
    bool checkLaserHit(const glm::vec2& bulletPos, float bulletRadius, ParticleSystem& particles);

    // Verifica se il giocatore è entrato in collisione con uno qualsiasi degli asteroidi attivi.
    bool checkPlayerCollision(const CircleCollider& playerCol);

    // Restituisce il conteggio degli asteroidi ancora intatti nella scena.
    int activeCount() const;

    // Accesso alla collezione per radar scanner ed editor (v2):
    const std::vector<Asteroid>& asteroids() const { return m_asteroids; }
    void spawnOne(const glm::vec2& pos, float scale = 1.0f, bool crystal = false);
    void clear() { m_asteroids.clear(); }

    // Applica un impulso fisico di repulsione circolare da detonazione (es. Onda EMP v2):
    void applyImpulse(const glm::vec2& center, float maxForce, float radius);

private:
    std::vector<Asteroid> m_asteroids; // Collezione delle istanze di asteroidi residenti nel livello
    Geometry m_asteroidGeo;            // Singola geometria condivisa in memoria GPU (Buffer Sharing)
};
