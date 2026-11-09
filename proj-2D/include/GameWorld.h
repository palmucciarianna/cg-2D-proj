// ============================================================================
// File: include/GameWorld.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Gestore principale del mondo di gioco (Facade Pattern).
//              Incapsula l'intera gerarchia della scena 2D, i sottosistemi di rendering,
//              la macchina a stati finiti (FSM), la logica di collisione, la gestione
//              del delta-time e l'interfaccia grafica Dear ImGui (HUD di Missione).
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include "Shader.h"
#include "BackgroundRenderer.h"
#include "Player.h"
#include "Collectible.h"
#include "AsteroidManager.h"
#include "LaserManager.h"
#include "HermiteObject.h"
#include "ParticleSystem.h"
#include "HazardManager.h"

struct GLFWwindow;

// Stati della macchina a stati finiti (FSM) del gameplay.
enum class GameState
{
    PLAYING,             // Fase attiva di navigazione, raccolta monete e combattimento
    LEVEL_CLEARED_HURRA, // Fase di vittoria e celebrazione ("HURRÀ!") dopo il salto nel Warp Gate
    GAME_OVER            // Esaurimento delle vite o distruzione da parte dei pericoli letali
};

class GameWorld
{
public:
    // Costruttore: riceve le dimensioni iniziali della finestra di gioco in pixel.
    GameWorld(int screenWidth, int screenHeight);

    // Inizializza tutti gli shader GLSL, compila le mesh geometriche e carica i modelli OBJ da cartella asset.
    void init(const std::string& assetDir);

    // Aggiorna la simulazione fisica, la cinematica e l'arbitraggio delle collisioni con il delta-time.
    void update(float dt, GLFWwindow* window);

    // Renderizza la scena grafica (sfondo procedurale, asteroidi, entità Hermite, monete, navicella, particelle).
    void render(float currentTime);

    // Costruisce e renderizza l'interfaccia utente grafica Dear ImGui (HUD e banner centrati).
    void renderUI(float dt);

    // Riavvia la partita al livello 1 mantenendo il punteggio cumulativo di sessione.
    void restartGame();

    // Avanza al livello successivo incrementando la difficoltà e rigenerando monete ed asteroidi.
    void nextLevel();



private:
    int m_screenWidth;  // Larghezza logica della finestra in pixel (1280)
    int m_screenHeight; // Altezza logica della finestra in pixel (720)

    GameState m_state = GameState::PLAYING; // Stato corrente della partita
    int m_currentLevel = 1;                 // Indice del livello corrente
    float m_hurraTimer = 0.0f;              // Timer per la durata della celebrazione "HURRÀ!" (2.8 secondi)

    // Programmi Shader e Renderer dedicati:
    std::unique_ptr<Shader> m_bgShader;                 // Shader procedurale per il background stellare
    std::unique_ptr<Shader> m_basicShader;              // Shader universale con proiezione ortografica
    std::unique_ptr<BackgroundRenderer> m_bgRenderer;   // Renderer per il quad a schermo intero

    // Sottosistemi di gioco incapsulati (Separation of Concerns):
    Player m_player;                     // Navicella mineraria Starfighter controllata dall'utente
    CollectibleManager m_collectibles;   // Gestore delle monete cosmiche oscillanti
    AsteroidManager m_asteroids;         // Gestore del campo di asteroidi a geometria condivisa
    LaserManager m_lasers;               // Gestore della balistica dei colpi al plasma
    HermiteManager m_hermite;            // Gestore del Drone alieno e del portale cosmico Warp Gate
    ParticleSystem m_particles;          // Motore particellare per esplosioni, scintille e coriandoli
    HazardManager m_hazards;             // Gestore dei raggi letali da schivare provenienti dai bordi

    bool m_showColliders = false;        // Flag di debug per visualizzare i volumi di collisione
    bool m_hudCompact = false;           // Stato di compressione dell'HUD (aperto all'avvio, compatto dopo il primo click)
};
