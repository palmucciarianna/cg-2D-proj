// ============================================================================
// File: include/Collectible.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Gestore delle monete cosmiche collezionabili (Requisito 7.2).
//              Implementa una simulazione visiva di rotazione pseudo-3D
//              tramite modulazione non lineare della scala orizzontale (cos(t))
//              ed oscillazione armonica verticale (levitazione spaziale).
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"
#include "Collider2D.h"
#include "ParticleSystem.h"

// Struttura dati per una singola moneta d'oro nel mondo 2D.
struct CosmicCoin
{
    glm::vec2 basePos;       // Posizione centrale di riposo (ancora dell'oscillazione)
    glm::vec2 currentPos;    // Posizione istantanea calcolata con l'oscillazione armonica
    float phaseOffset = 0.0f;// Sfasamento temporale per de-sincronizzare il galleggiamento tra monete
    float radius = 22.0f;    // Raggio geometrico del collider circolare in pixel
    bool collected = false;  // Flag di stato: true quando la moneta è stata raccolta dal giocatore
};

class CollectibleManager
{
public:
    // Costruttore predefinito.
    CollectibleManager();

    // Inizializza la geometria circolare della moneta dorata a 36 segmenti e la carica su GPU.
    void init();

    // Spawna 5 monete d'oro distribuite proceduralmente nel livello.
    void spawnLevel(int levelCount, float screenWidth, float screenHeight);

    // Aggiorna l'animazione armonica verticale delle monete non ancora raccolte.
    void update(float dt, float currentTime);

    // Renderizza le monete attive applicando la scala pseudo-3D sulla coordinata X locale.
    void draw(const Shader& shader, float currentTime);

    // Verifica la collisione con il bounding circle del giocatore:
    // raccoglie la moneta, genera scintille particellari dorate e restituisce il numero di monete colte.
    int checkCollisions(const CircleCollider& playerCol, ParticleSystem& particles);

    // Metodi di ispezione dello stato della missione:
    int totalCoins() const { return static_cast<int>(m_coins.size()); }
    int collectedCoins() const { return m_collectedCount; }
    int remainingCoins() const { return totalCoins() - m_collectedCount; }
    bool allCollected() const { return totalCoins() > 0 && m_collectedCount >= totalCoins(); }

private:
    std::vector<CosmicCoin> m_coins; // Collezione delle 5 monete del livello
    Geometry m_coinGeo;              // Risorsa geometrica condivisa su GPU (VAO/VBO)
    int m_collectedCount{ 0 };       // Contatore monete raccolte nel livello corrente
};
