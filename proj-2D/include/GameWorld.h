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
#include "PostProcessing.h"
#include "PowerUpManager.h"
#include "BlackHole.h"
#include "MissileManager.h"
#include "CosmicDust.h"

struct GLFWwindow;

// Stati della macchina a stati finiti (FSM) del gameplay.
enum class GameState
{
    PLAYING,             // Fase attiva di navigazione, raccolta monete e combattimento
    LEVEL_CLEARED_HURRA, // Fase di vittoria e celebrazione ("HURRÀ!") dopo il salto nel Warp Gate
    GAME_OVER            // Esaurimento delle vite o distruzione da parte dei pericoli letali
};

// Struttura dati per le voci della classifica globale / piloti d'élite.
struct LeaderboardEntry
{
    std::string name;
    int score;
    int level;
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

    // Accesso alla modalità di disegno wireframe (Requisito didattico di laboratorio)
    bool wireframe() const { return m_wireframe; }
    void setWireframe(bool enabled) { m_wireframe = enabled; }

    // Controllo del fattore di scala temporale (Slow-Motion didattico per esame):
    float timeScale() const { return m_timeScale; }
    void setTimeScale(float ts) { m_timeScale = ts; }

    // Innesca il trauma della telecamera 2D (Screen Shake) per impatti ed esplosioni:
    void triggerScreenShake(float duration = 0.25f, float intensity = 8.0f);

    // Gestione della classifica piloti:
    void submitScore(const std::string& name, int score, int level);
    const std::vector<LeaderboardEntry>& leaderboard() const { return m_leaderboard; }

    // Accesso al gestore di Post-Processing (FBO, CRT, Shockwaves, Vignette)
    PostProcessing& postProcess() { return m_postProcess; }

    // Accesso ai power-up e al buco nero (v2):
    PowerUpManager& powerUps() { return m_powerUps; }
    BlackHole& blackHole() { return m_blackHole; }

    // Bomba a Impulsi Elettromagnetici EMP (v2):
    void triggerEmpBomb();
    float empCooldown() const { return m_empCooldown; }
    bool isEmpReady() const { return m_empCooldown <= 0.0f; }

    // Modalita' Fotografica / Freeze Frame (v2):
    bool isPhotoMode() const { return m_photoMode; }
    void setPhotoMode(bool p) { m_photoMode = p; }


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
    PowerUpManager m_powerUps;           // Gestore delle capsule e dei power-up (Scudo, Laser Triplo, Magnete)
    BlackHole m_blackHole;               // Anomalia gravitazionale del Buco Nero (v2)
    MissileManager m_missiles;           // Gestore missili intelligenti a ricerca termica (v2)
    CosmicDust m_cosmicDust;             // Pulviscolo stellare reattivo (v2)

    bool m_showColliders = false;        // Flag di debug per visualizzare i volumi di collisione
    bool m_hudCompact = false;           // Stato di compressione dell'HUD (aperto all'avvio, compatto dopo il primo click)
    bool m_wireframe = false;            // Spunta Wireframe: visualizza le mesh in modalità reticolare (Laboratorio di CG)
    float m_psychedelic = 0.40f;         // Intensità dell'effetto spaziale psichedelico / warp (0.0..1.0)

    // Meccaniche avanzate post-consegna (v2):
    PostProcessing m_postProcess;        // Pipeline di Post-Processing basata su FBO
    float m_timeScale = 1.0f;            // Fattore di scala temporale per slow-motion (0.1..2.0x)
    float m_shakeTimer = 0.0f;           // Timer residuo dello scuotimento telecamera
    float m_shakeDuration = 0.0f;        // Durata iniziale dello shake
    float m_shakeIntensity = 0.0f;       // Ampiezza dell'oscillazione
    glm::vec2 m_shakeOffset{ 0.0f };     // Vettore di spostamento camera 2D

    // Bomba a Impulsi Elettromagnetici EMP (v2):
    float m_empCooldown = 0.0f;
    float m_maxEmpCooldown = 10.0f;

    // Modalita' Fotografica & Studio Screenshot (v2):
    bool m_photoMode = false;
    float m_photoZoom = 1.0f;
    glm::vec2 m_photoPan{ 0.0f, 0.0f };
    bool m_showCustomizer = false;

    std::vector<LeaderboardEntry> m_leaderboard; // Classifica piloti
    bool m_showLeaderboard = false;              // Finestra classifica aperta/chiusa
    bool m_scoreSubmitted = false;               // Flag di invio punteggio partita

    // Sottosistemi audio, scanner radar e sandbox (v2):
    bool m_showRadar = true;                     // Finestra Scanner Radar Olografico
    bool m_sandboxMode = false;                  // Modalita' Sandbox / Editor attiva
    int m_sandboxSpawnType = 0;                  // Tipo di entita' selezionata per lo spawn

    void renderRadar(float currentTime);         // Rendering del radar vettoriale ImGui
};
