// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: GameWorld
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - È il "direttore d'orchestra" e coordinatore centrale del gioco (Facade Pattern).
//    - Gestisce la FSM degli stati di gioco: PLAYING, LEVEL_CLEARED_HURRA, GAME_OVER.
//    - Costruisce la matrice di proiezione ortografica (glm::ortho) per lo spazio 1280x720.
//    - Risolve tutte le collisioni tra entità (Player, Asteroidi, Monete, Laser, Hermite Gate).
//    - Renderizza l'interfaccia Dear ImGui (HUD di Missione) con auto-compressione non invasiva.
//
// 2. A CHI È COLLEGATA:
//    - Creata e chiamata da: main.cpp (nel game loop tramite init(), update(dt), render()).
//    - Contiene e orchestra tutti i sottosistemi del gioco:
//      * BackgroundRenderer (sfondo procedurale in NDC)
//      * Player (navicella composita ad albero gerarchico)
//      * AsteroidManager (cintura asteroidi con Buffer Sharing su singolo VAO)
//      * CollectibleManager (5 monete d'oro con rotazione pseudo-3D ortogonale)
//      * HermiteManager (Alien Sentinel Drone su curva di Lissajous e Warp Gate)
//      * LaserManager & HazardManager (proiettili laser e pericoli perimetrali)
//      * ParticleSystem (motore particellare con attrito viscoso e alpha fade)
//    - Utilizza: Shader ("background" e "basic") e Collider2D (test geometrici di collisione).
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Proiezione Ortografica: P = glm::ortho(0, 1280, 0, 720, -1, 1). I raggi di proiezione
//      sono paralleli; le dimensioni non variano con la distanza (Requisito 5 & Domanda 9).
//    - Pipeline Graphics (Domanda 4): gestisce la fase di Application su CPU, calcola il moto
//      Euleriano e le matrici Model/Projection, inoltrandole ai Vertex Shader.
//    - Ordine di Disegno Back-to-Front (Domanda 20): Sfondo opaco -> Oggetti solidi ->
//      Navicella -> Elementi traslucidi in Alpha Blending (fiamme, LED, particelle).
//    - Separation of Concerns (SoC) e RAII (Requisito 8 & Domanda 19): architettura a moduli
//      senza variabili globali, memoria gestita tramite smart pointer (std::unique_ptr).
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

    // Accesso alla modalità di disegno wireframe (Requisito didattico di laboratorio)
    bool wireframe() const { return m_wireframe; }
    void setWireframe(bool enabled) { m_wireframe = enabled; }


private:
    // Helper condiviso per inizializzare o resettare un livello di gioco (usato da nextLevel e restartGame).
    void startLevel(int level, bool clearParticles);

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
    bool m_wireframe = false;            // Spunta Wireframe: visualizza le mesh in modalità reticolare (Laboratorio di CG)
    float m_psychedelic = 0.40f;         // Intensità dell'effetto spaziale psichedelico / warp (0.0..1.0)
};
