// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: GameWorld.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Coordinatore centrale del motore di gioco (orchestra init, update, render e HUD).
//    - Macchina a stati FSM (PLAYING, LEVEL_CLEARED_HURRA, GAME_OVER).
//    - Risolve le collisioni (Player vs Asteroidi, Player vs Monete, Laser vs Asteroidi/Drone).
//    - Costruisce la proiezione ortografica e controlla l'interfaccia non invasiva ImGui.
// 2. A CHI È COLLEGATO:
//    - Invocato dal loop principale in main.cpp; coordina tutti i manager e renderers.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - P = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f) (Domanda 9).
//    - Disegno Back-to-Front con Alpha Blending (Domanda 20).
//    - Delta-Time Clamping a 0.05s per stabilita' fisica (Requisito 6).
// ============================================================================

#include "GameWorld.h"
#include "AudioEngine.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"
#include <iostream>
#include <fstream>

// ============================================================================
// Risoluzione Dinamica dei Percorsi degli Asset (Cross-Platform Fallback)
// ============================================================================
// Verifica l'esistenza di un file provando directory locale, cartella genitore
// e percorsi standard per supportare l'avvio da Finder, Visual Studio o Terminale.
static std::string resolveAsset(const std::string& assetDir, const std::string& subPath)
{
    // 1. Prova con la cartella indicata esplicitamente:
    std::string p1 = assetDir + "/" + subPath;
    std::ifstream f1(p1);
    if (f1.good()) return p1;

    // 2. Prova nella sottocartella locale assets/:
    std::string p2 = "assets/" + subPath;
    std::ifstream f2(p2);
    if (f2.good()) return p2;

    // 3. Prova nella cartella superiore ../assets/:
    std::string p3 = "../assets/" + subPath;
    std::ifstream f3(p3);
    if (f3.good()) return p3;

    // Fallback al percorso primario:
    return p1;
}

// ============================================================================
// Costruttore
// ============================================================================
GameWorld::GameWorld(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight)
{
}

// ============================================================================
// Inizializzazione del Mondo di Gioco e Allocazione Sottosistemi
// ============================================================================
void GameWorld::init(const std::string& assetDir)
{
    // ------------------------------------------------------------------------
    // 1. Caricamento e Compilazione degli Shader GLSL
    // ------------------------------------------------------------------------
    // Shader procedurale per il background stellare animato.
    std::string bgVert = resolveAsset(assetDir, "shaders/background.vert");
    std::string bgFrag = resolveAsset(assetDir, "shaders/background.frag");
    m_bgShader = std::make_unique<Shader>();
    if (!m_bgShader->load(bgVert.c_str(), bgFrag.c_str()))
    {
        std::cerr << "[ERRORE] Caricamento shader background fallito: " << bgVert << "\n";
    }
    // Istanzia il renderer per il fullscreen quad in coordinate NDC.
    m_bgRenderer = std::make_unique<BackgroundRenderer>(*m_bgShader);

    // Shader universale con supporto proiezione ortografica e moltiplicazione per colore.
    std::string basicVert = resolveAsset(assetDir, "shaders/basic.vert");
    std::string basicFrag = resolveAsset(assetDir, "shaders/basic.frag");
    m_basicShader = std::make_unique<Shader>();
    if (!m_basicShader->load(basicVert.c_str(), basicFrag.c_str()))
    {
        std::cerr << "[ERRORE] Caricamento basic shader fallito: " << basicVert << "\n";
    }

    // ------------------------------------------------------------------------
    // 2. Inizializzazione dei Singoli Sottosistemi
    // ------------------------------------------------------------------------
    // Inizializza la geometria condivisa del sistema particellare (scintille, detriti, coriandoli).
    m_particles.init();
    // Inizializza il gestore dei pericoli perimetrali supersonici.
    m_hazards.init();
    // Alloca le 11 componenti gerarchiche e la texture della navicella del giocatore.
    m_player.init();
    // Colloca la navicella esattamente al centro dello schermo (640, 360).
    m_player.reset(glm::vec2(m_screenWidth * 0.5f, m_screenHeight * 0.5f));

    // Prepara il VAO statico condiviso per le 5 monete d'oro collezionabili.
    m_collectibles.init();
    // Prepara il VAO statico condiviso (Buffer Sharing) per tutti gli asteroidi.
    m_asteroids.init();
    // Prepara la geometria condivisa per i proiettili laser al plasma.
    m_lasers.init();
    m_powerUps.init();
    m_blackHole.init();
    m_missiles.init();
    m_cosmicDust.init(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), 160);

    // Carica i modelli poligonali delle spline di Hermite (Drone Nemico e Portale Warp Gate).
    std::string alienObj = resolveAsset(assetDir, "models/alien_drone.obj"); // Risolve il percorso della mesh del Drone
    std::string warpObj  = resolveAsset(assetDir, "models/warp_gate.obj");   // Risolve il percorso della mesh del Warp Gate
    m_hermite.init(alienObj, warpObj); // Carica e invia in VRAM i due modelli spline di Hermite

    // ------------------------------------------------------------------------
    // 3. Generazione Procedurale delle Entità per il Livello 1
    // ------------------------------------------------------------------------
    // Posiziona le 5 monete d'oro evitando sovrapposizioni con lo spawn centrale.
    m_collectibles.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));
    // Genera il campo iniziale di asteroidi con orbite e velocità casuali.
    m_asteroids.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));

    // Inizializza la pipeline FBO di Post-Processing (Shockwave, CRT, Aberrazione Cromatica)
    m_postProcess.init(m_screenWidth, m_screenHeight, assetDir);

    // Inizializza il motore audio procedurale in tempo reale (v2)
    AudioEngine::instance().init();

    // ------------------------------------------------------------------------
    // 4. Inizializzazione della Classifica Piloti (Leaderboard)
    // ------------------------------------------------------------------------
    m_leaderboard = {
        { "Arianna (Top Ace)", 2450, 5 },
        { "Star Pioneer", 1820, 4 },
        { "Vanguard 07", 1340, 3 },
        { "Cosmic Scout", 880, 2 },
        { "Space Rookie", 450, 1 }
    };
}

// ============================================================================
// Innesco del Trauma della Telecamera (Screen Shake)
// ============================================================================
void GameWorld::triggerScreenShake(float duration, float intensity)
{
    m_shakeDuration = duration;
    m_shakeTimer = duration;
    m_shakeIntensity = intensity;
}

// ============================================================================
// Inserimento e Ordinamento Punteggio nella Leaderboard
// ============================================================================
void GameWorld::submitScore(const std::string& name, int score, int level)
{
    std::string pilotName = name.empty() ? "Arianna" : name;
    m_leaderboard.push_back(LeaderboardEntry{ pilotName, score, level });
    std::sort(m_leaderboard.begin(), m_leaderboard.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
        return a.score > b.score;
    });
    if (m_leaderboard.size() > 10)
    {
        m_leaderboard.resize(10);
    }
}

// ============================================================================
// Aggiornamento della Simulazione e della Logica di Gioco (Game Loop Update)
// ============================================================================
// [Domanda 4: Stadi della Graphics Rendering Pipeline - Fase Application su CPU]
void GameWorld::update(float dt, GLFWwindow* window)
{
    float currentTime = static_cast<float>(glfwGetTime());

    // Aggiornamento degli effetti di post-processing (onde d'urto, flash scafo)
    m_postProcess.update(dt);

    // Integrazione dello Screen Shake (decadimento sinusoidale smorzato):
    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        float progress = std::max(0.0f, m_shakeTimer / m_shakeDuration);
        float currentAmp = m_shakeIntensity * progress;
        m_shakeOffset = glm::vec2(
            std::sin(currentTime * 52.0f) * currentAmp,
            std::cos(currentTime * 44.0f) * currentAmp
        );
    }
    else
    {
        m_shakeOffset = glm::vec2(0.0f);
    }

    // Calcolo del delta-time scalato per la modalità didattica Slow-Motion (0.1..2.0x):
    float simDt = dt * m_timeScale;

    // Gestione tasto Photo Mode (P):
    static bool lastKeyP = false;
    bool currKeyP = (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);
    if (currKeyP && !lastKeyP)
    {
        m_photoMode = !m_photoMode;
    }
    lastKeyP = currKeyP;

    // Gestione tasto Fari (L):
    static bool lastKeyL = false;
    bool currKeyL = (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS);
    if (currKeyL && !lastKeyL)
    {
        m_player.setHeadlights(!m_player.headlightsActive());
    }
    lastKeyL = currKeyL;

    // In Modalita' Fotografica o Pausa Impostazioni si sospende l'avanzamento fisico del mondo
    if (m_photoMode || m_isPaused)
    {
        return;
    }

    if (m_empCooldown > 0.0f)
    {
        m_empCooldown -= dt;
    }

    // Modalita' Boss attiva a partire dal Livello 3:
    if (m_currentLevel >= 3)
    {
        m_hermite.setBossMode(true);
    }

    // ------------------------------------------------------------------------
    // STATO 1: PARTITA IN CORSO (PLAYING)
    // ------------------------------------------------------------------------
    if (m_state == GameState::PLAYING)
    {
        // Regolazione interattiva della velocità della navicella da tastiera:
        float currentSpeed = m_player.speed();
        if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
           (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS))
        {
            currentSpeed = std::min(currentSpeed + 220.0f * dt, 850.0f);
            m_player.setSpeed(currentSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
           (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS))
        {
            currentSpeed = std::max(currentSpeed - 220.0f * dt, 120.0f);
            m_player.setSpeed(currentSpeed);
        }

        // Aggiorna cinematica navicella (gestione WASD e Frecce Direzionali con simDt).
        m_player.update(simDt, window, m_screenWidth, m_screenHeight);

        // Emissione particellare della scia di plasma dai motori quando la navetta accelera:
        if (m_player.isAlive() && m_player.thrustIntensity() > 0.08f)
        {
            glm::vec2 shipDir = m_player.direction();
            glm::vec2 backPos = m_player.position() - shipDir * 30.0f;
            m_particles.emitThrusterTrail(backPos, shipDir, m_player.thrustIntensity());
        }

        // Aggiorna le minacce spaziali laterali (proiettili ad alta velocità).
        m_hazards.update(simDt, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), m_particles);

        // Controllo impatto con un raggio perimetrale di pericolo:
        if (m_hazards.checkPlayerHit(m_player.getCollider(), m_particles))
        {
            if (!m_player.isInvulnerable())
            {
                bool tookDmg = m_player.takeDamage(); // Toglie 1 vita o viene deflesso dallo scudo
                m_particles.emitAsteroidExplosion(m_player.position(), 24);
                triggerScreenShake(0.35f, 14.0f);
                m_postProcess.triggerDamageFlash(1.0f);
                m_postProcess.triggerShockwave(m_player.position(), static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), 0.045f, 0.45f);
                if (tookDmg) AudioEngine::instance().playExplosion();
                else AudioEngine::instance().playShieldDeflect();
            }
        }

        // Sparo Laser con Barra Spaziatrice:
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && m_player.canFire())
        {
            float rad = glm::radians(m_player.angle());
            glm::vec2 dir(std::cos(rad), std::sin(rad));

            if (m_player.isSpreadShotActive())
            {
                // Modalità Laser Triplo a Ventaglio: 3 raggi con divergenza angolare +/- 14 gradi
                float radLeft = glm::radians(m_player.angle() - 14.0f);
                float radRight = glm::radians(m_player.angle() + 14.0f);
                glm::vec2 dirLeft(std::cos(radLeft), std::sin(radLeft));
                glm::vec2 dirRight(std::cos(radRight), std::sin(radRight));

                m_lasers.fire(m_player.position() + dir * 36.0f, dir);
                m_lasers.fire(m_player.position() + dirLeft * 32.0f, dirLeft);
                m_lasers.fire(m_player.position() + dirRight * 32.0f, dirRight);
            }
            else
            {
                // Spara il raggio singolo con offset di 36px in avanti rispetto alla prua.
                m_lasers.fire(m_player.position() + dir * 36.0f, dir);
            }

            AudioEngine::instance().playLaser();
            m_player.resetFireCooldown(0.18f);
        }

        // --------------------------------------------------------------------
        // Lancio Missili Intelligenti a Ricerca Termica (Tasto Q):
        // --------------------------------------------------------------------
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && m_missiles.canFire())
        {
            glm::vec2 pDir = m_player.direction();
            glm::vec2 pSide(-pDir.y, pDir.x);
            m_missiles.fire(m_player.position() + pSide * 26.0f, pDir);
            m_missiles.fire(m_player.position() - pSide * 26.0f, pDir);
            m_missiles.resetCooldown(0.85f);
            AudioEngine::instance().playLaser();
        }

        // --------------------------------------------------------------------
        // Attivazione Bomba a Impulsi Elettromagnetici EMP (Tasto E o Click Destro):
        // --------------------------------------------------------------------
        ImGuiIO& io = ImGui::GetIO();
        bool ePressed = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
        bool rmbPressed = (!io.WantCaptureMouse && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        if ((ePressed || rmbPressed) && m_empCooldown <= 0.0f)
        {
            triggerEmpBomb();
        }

        // Aggiorna i colpi laser attivi e gestisce le collisioni con asteroidi e Drone Rosso.
        int prevScore = m_player.score();
        int score = prevScore;
        m_lasers.update(simDt, m_asteroids, m_hermite, m_particles, score);
        if (score != prevScore)
        {
            m_player.addScore(score - prevScore);
            triggerScreenShake(0.16f, 5.0f);
            AudioEngine::instance().playExplosion();
        }

        // Aggiorna i Missili Intelligenti a Ricerca (v2):
        int mScore = 0;
        m_missiles.update(simDt, m_asteroids, m_hermite, m_particles, mScore);
        if (mScore > 0)
        {
            m_player.addScore(mScore);
            triggerScreenShake(0.20f, 7.0f);
        }

        // Aggiorna il pulviscolo stellare reattivo (v2):
        m_cosmicDust.update(simDt, currentTime, m_player.position(), m_player.angle(), m_player.headlightsActive(), m_blackHole.position(), m_blackHole.isActive());

        // Aggiorna le entità del mondo con il delta-time di simulazione.
        m_asteroids.update(simDt);
        m_collectibles.update(simDt, currentTime);
        m_hermite.update(simDt, currentTime, m_player.position());
        m_particles.update(simDt);

        // Aggiorna l'anomalia gravitazionale del Buco Nero (v2):
        m_blackHole.update(simDt, m_player, m_asteroids);

        // Attrazione magnetica delle monete verso la navetta (se power-up Calamita e' attivo):
        if (m_player.isMagnetActive())
        {
            m_collectibles.applyMagnetAttraction(m_player.position(), 380.0f, 260.0f, simDt);
        }

        // Aggiorna capsule e collezionabili speciali Power-Up (v2):
        m_powerUps.update(simDt);
        int puScore = 0;
        PowerUpType pu = m_powerUps.checkPlayerCollision(m_player.getCollider(), m_player, m_particles, puScore);
        if (pu != PowerUpType::NONE)
        {
            m_player.addScore(puScore);
            triggerScreenShake(0.20f, 7.0f);
            m_postProcess.triggerShockwave(m_player.position(), static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), 0.045f, 0.45f);
            AudioEngine::instance().playPowerUp();
        }

        // Raccolta delle Monete d'Oro (+100 punti per ciascuna moneta):
        int collectedNow = m_collectibles.checkCollisions(m_player.getCollider(), m_particles); // Verifica impatto cerchio-cerchio
        if (collectedNow > 0) // Se almeno una moneta è stata raccolta in questo frame
        {
            m_player.addScore(collectedNow * 100);
            AudioEngine::instance().playCoin();
        }

        // Interazione facoltativa di prossimità con la reliquia monumentale (+50 punti bonus):
        int relicScore = 0; // Accumulatore per il bonus di prossimità
        if (!m_collectibles.allCollected() && m_hermite.checkRelicInteraction(m_player.getCollider(), m_particles, relicScore))
        {
            m_player.addScore(relicScore);
            AudioEngine::instance().playCoin();
        }

        // --------------------------------------------------------------------
        // Collisione con l'Alien Sentinel Drone (quello Rosso):
        // --------------------------------------------------------------------
        if (m_hermite.checkDroneCollision(m_player.getCollider())) // Test distanza al quadrato tra navicella e Drone
        {
            if (m_hermite.isWeakened()) // Caso 1: il Drone è stato temporaneamente disattivato dal laser
            {
                // Drone indebolito: innocuo! Scintille di rimbalzo senza perdita di vite
                m_particles.emitDroneHitSparks(m_hermite.dronePosition(), 12);
                AudioEngine::instance().playDroneHit();
            }
            else if (!m_player.isInvulnerable()) // Caso 2: Drone attivo e giocatore non invulnerabile
            {
                bool tookDmg = m_player.takeDamage();
                m_hermite.triggerEyeBlink();
                m_particles.emitAsteroidExplosion(m_player.position(), 24);
                triggerScreenShake(0.35f, 13.0f);
                m_postProcess.triggerDamageFlash(1.0f);
                m_postProcess.triggerShockwave(m_player.position(), static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), 0.055f, 0.55f);
                if (tookDmg) AudioEngine::instance().playExplosion();
                else AudioEngine::instance().playShieldDeflect();
            }
        }

        // Collisione con gli asteroidi rocciosi in transito:
        if (m_asteroids.checkPlayerCollision(m_player.getCollider())) // Test cerchio-cerchio con tutti gli asteroidi attivi
        {
            bool tookDmg = m_player.takeDamage();
            m_particles.emitAsteroidExplosion(m_player.position(), 16);
            triggerScreenShake(0.30f, 11.0f);
            m_postProcess.triggerDamageFlash(0.85f);
            m_postProcess.triggerShockwave(m_player.position(), static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), 0.04f, 0.45f);
            if (tookDmg) AudioEngine::instance().playExplosion();
            else AudioEngine::instance().playShieldDeflect();
        }

        // Se le vite sono terminate, scatta il Game Over:
        if (!m_player.isAlive()) // Controlla se m_lives <= 0
        {
            m_state = GameState::GAME_OVER;
            triggerScreenShake(0.45f, 16.0f);
            if (!m_scoreSubmitted && m_player.score() > 0)
            {
                submitScore("Arianna", m_player.score(), m_currentLevel);
                m_scoreSubmitted = true;
            }
        }

        // --------------------------------------------------------------------
        // Condizione di Chiusura Partita / Salto nel Cosmic Warp Gate:
        // --------------------------------------------------------------------
        bool gateActive = m_collectibles.allCollected();
        if (gateActive && m_hermite.checkPlayerGateEnter(m_player.getCollider(), gateActive))
        {
            m_state = GameState::LEVEL_CLEARED_HURRA;
            m_hurraTimer = 2.8f;    // Celebrazione di 2.8 secondi
            m_player.addScore(500); // 500 punti bonus per il salto dimensionale
            triggerScreenShake(0.40f, 9.0f);
            m_postProcess.triggerShockwave(m_hermite.gatePosition(), static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), 0.085f, 0.95f);
            AudioEngine::instance().playWarp();
            // Emette fuochi d'artificio e coriandoli a schermo.
            m_particles.emitHurraCelebration(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), 135);
            if (!m_scoreSubmitted && m_player.score() > 0)
            {
                submitScore("Arianna", m_player.score(), m_currentLevel);
                m_scoreSubmitted = true;
            }
        }
    }
    // ------------------------------------------------------------------------
    // STATO 2: VITTORIA LIVELLO E CELEBRAZIONE ("HURRÀ!")
    // ------------------------------------------------------------------------
    else if (m_state == GameState::LEVEL_CLEARED_HURRA)
    {
        m_hurraTimer -= dt;
        m_particles.update(simDt);
        m_hermite.update(simDt, currentTime, m_player.position());

        // Al termine del timer di celebrazione, avanza automaticamente al livello successivo.
        if (m_hurraTimer <= 0.0f)
        {
            nextLevel(); // Genera il livello m_currentLevel + 1
        }
    }
    // ------------------------------------------------------------------------
    // STATO 3: GAME OVER
    // ------------------------------------------------------------------------
    else if (m_state == GameState::GAME_OVER)
    {
        m_particles.update(simDt);
    }
}

// ============================================================================
// Rendering della Scena Grafica 2D
// ============================================================================
void GameWorld::render(float currentTime)
{
    // 0. Attiva l'FBO per catturare la scena nella texture di post-processing
    m_postProcess.bind();

    // [Domanda 20: Ordine di rendering Back-to-Front per trasparenza corretta]
    // 1. Disegna il background cosmico procedurale (stelle e nebulosa in NDC).
    m_bgRenderer->draw(currentTime, m_psychedelic, m_currentLevel);

    // 2. Modalità Wireframe (Requisito didattico di laboratorio):
#ifndef __EMSCRIPTEN__
    // Se la spunta è attiva, imposta la rasterizzazione reticolare (GL_LINE)
    // per esaminare la triangolazione di tutte le mesh 2D della scena.
    if (m_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
#endif

    // [Domanda 9: Sottosistema geometrico - Proiezione Ortografica 2D]
    // 3. Configura la matrice di Proiezione Ortografica per lo spazio di gioco:
    // Mappa lo spazio [0, screenWidth] su X e [0, screenHeight] su Y.
    // L'origine (0, 0) è l'angolo inferiore sinistro dello schermo.
    m_basicShader->use();
    glm::mat4 projection = glm::ortho(
        0.0f, static_cast<float>(m_screenWidth),
        0.0f, static_cast<float>(m_screenHeight),
        -1.0f, 1.0f
    );

    // Gestione zoom e pan in Modalita' Foto (v2) oppure Screen Shake:
    if (m_photoMode)
    {
        glm::vec2 camCenter = m_player.position() + m_photoPan;
        projection = glm::translate(glm::mat4(1.0f), glm::vec3(m_screenWidth * 0.5f, m_screenHeight * 0.5f, 0.0f)) *
                     glm::scale(glm::mat4(1.0f), glm::vec3(m_photoZoom, m_photoZoom, 1.0f)) *
                     glm::translate(glm::mat4(1.0f), glm::vec3(-camCenter, 0.0f)) *
                     glm::ortho(0.0f, static_cast<float>(m_screenWidth), 0.0f, static_cast<float>(m_screenHeight), -1.0f, 1.0f);
    }
    else if (glm::length(m_shakeOffset) > 0.001f)
    {
        projection = glm::translate(projection, glm::vec3(m_shakeOffset, 0.0f));
    }

    m_basicShader->setMat4("projection", projection);
    m_basicShader->setVec4("uColorTint", glm::vec4(1.0f));

    // 0) Rendering del Pulviscolo Stellare Reattivo (v2):
    m_cosmicDust.draw(*m_basicShader);

    // A) Rendering delle istanze condivise di asteroidi:
    m_asteroids.draw(*m_basicShader, currentTime);

    // B) Rendering degli oggetti con curve di Hermite (Alien Drone e Warp Gate):
    bool gateActive = (m_state == GameState::LEVEL_CLEARED_HURRA || m_collectibles.allCollected());
    m_hermite.draw(*m_basicShader, currentTime, gateActive);

    // C) Rendering delle monete d'oro oscillanti con rotazione pseudo-3D:
    m_collectibles.draw(*m_basicShader, currentTime);

    // D) Rendering dei colpi laser al plasma:
    m_lasers.draw(*m_basicShader);

    // D2) Rendering dei Missili Intelligenti a Ricerca Termica (v2):
    m_missiles.draw(*m_basicShader);

    // E) Rendering dei pericoli laterali e dei segnali stroboscopici di allerta:
    m_hazards.draw(*m_basicShader, currentTime, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));

    // F) Rendering del sistema particellare (detriti, scintille, coriandoli):
    m_particles.draw(*m_basicShader);

    // H) Rendering dell'Anomalia Gravitazionale del Buco Nero (v2):
    m_blackHole.draw(*m_basicShader, currentTime);

    // I) Rendering delle capsule speciali Power-Up (v2):
    m_powerUps.draw(*m_basicShader, currentTime);

    // G) Rendering gerarchico della navicella Starfighter del giocatore:
    if (m_player.isAlive())
    {
        m_player.draw(*m_basicShader, currentTime, projection);
    }

#ifndef __EMSCRIPTEN__
    // 4. Ripristino Modalità Piena (GL_FILL):
    // Assicura che l'interfaccia Dear ImGui e i testi vengano sempre disegnati
    // in modalità solida (senza artefatti reticolari sui caratteri).
    if (m_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
#endif

    // 5. Ripristina il Framebuffer di default a schermo
    m_postProcess.unbind();

    // 6. Esegue il rendering del quad a tutto schermo con shader di post-process (Shockwave, CRT, ecc.)
    m_postProcess.draw(currentTime);
}

// ============================================================================
// Rendering dell'Interfaccia Utente Grafica (Dear ImGui HUD)
// ============================================================================
void GameWorld::renderUI(float dt)
{
    ImGuiIO& io = ImGui::GetIO();

    // Funzione lambda per centrare orizzontalmente le scritte all'interno dei banner ImGui.
    auto renderCenteredText = [](const char* text, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPosX(std::max(0.0f, (windowWidth - textWidth) * 0.5f));
        ImGui::TextColored(color, "%s", text);
    };

    // ------------------------------------------------------------------------
    // Studio Fotografico Spaziale (Photo Mode v2):
    // ------------------------------------------------------------------------
    if (m_photoMode)
    {
        ImGui::SetNextWindowPos(ImVec2(24, 24), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(340, 430), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Studio Fotografico Spaziale (Photo Mode)", &m_photoMode))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "[MODALITA' FOTO ATTIVA - PAUSA]");
            ImGui::Separator();

            ImGui::SliderFloat("Zoom Ottico", &m_photoZoom, 0.4f, 3.5f, "%.2fx");
            ImGui::SliderFloat("Pan Orizzontale (X)", &m_photoPan.x, -500.0f, 500.0f, "%.0f px");
            ImGui::SliderFloat("Pan Verticale (Y)", &m_photoPan.y, -350.0f, 350.0f, "%.0f px");
            if (ImGui::SmallButton("Reset Centro Navicella"))
            {
                m_photoPan = glm::vec2(0.0f);
                m_photoZoom = 1.0f;
            }

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), "Fari & Illuminazione Dinamica 2D:");
            ImGui::Checkbox("Fari Volumetrici Navicella", &m_player.headlightsRef());
            ImGui::SliderFloat("Sfondo Psichedelico", &m_psychedelic, 0.0f, 1.0f, "%.2f");

            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.9f, 1.0f), "Filtri Ottici (FBO Post-Process):");
            ImGui::Checkbox("Filtro CRT Retro-Arcade", &m_postProcess.crtEnabled());
            if (m_postProcess.crtEnabled())
            {
                ImGui::SliderFloat("Scanlines", &m_postProcess.scanlineIntensity(), 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Aberrazione RGB", &m_postProcess.chromaticAberration(), 0.0f, 0.015f, "%.4f");
            }
            ImGui::SliderFloat("Vignettatura Cosmica", &m_postProcess.vignetteIntensity(), 0.0f, 1.0f, "%.2f");

            ImGui::Dummy(ImVec2(0.0f, 8.0f));
            if (ImGui::Button("Torna al Gioco [P]", ImVec2(-1, 38)))
            {
                m_photoMode = false;
            }
        }
        ImGui::End();
        return;
    }

    // ------------------------------------------------------------------------
    // Barra della Vita Boss Overlord (Livello 3+):
    // ------------------------------------------------------------------------
    if (m_hermite.isBoss() && m_state == GameState::PLAYING)
    {
        float bossW = 460.0f;
        float bossH = 46.0f;
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, 18.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(bossW, bossH));
        ImGui::Begin("BossHpBar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        float hpFrac = static_cast<float>(m_hermite.bossHp()) / static_cast<float>(m_hermite.bossMaxHp());
        char bossBuf[96];
        snprintf(bossBuf, sizeof(bossBuf), "ALIEN SENTINEL OVERLORD: %d / %d HP", m_hermite.bossHp(), m_hermite.bossMaxHp());

        float tw = ImGui::CalcTextSize(bossBuf).x;
        ImGui::SetCursorPosX(std::max(0.0f, (bossW - tw) * 0.5f));
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", bossBuf);

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.95f, 0.20f, 0.20f, 1.0f));
        ImGui::ProgressBar(hpFrac, ImVec2(bossW - 16, 16), "");
        ImGui::PopStyleColor();
        ImGui::End();
    }

    // ------------------------------------------------------------------------
    // Rilevamento Interazione Mouse per Pausa Automatica e Ripresa Gioco (v2)
    // ------------------------------------------------------------------------
    if (m_autoPauseOnSettings && m_state == GameState::PLAYING)
    {
        // Se l'utente clicca su un qualsiasi elemento o finestra delle impostazioni ImGui, attiva la pausa:
        if (!m_isPaused && io.WantCaptureMouse && (io.MouseClicked[0] || io.MouseClicked[1]))
        {
            m_isPaused = true;
        }
        // Se il gioco e' in pausa e l'utente clicca nell'area di gioco (spazio cosmico), riprendi la simulazione:
        else if (m_isPaused && !io.WantCaptureMouse && (io.MouseClicked[0] || io.MouseClicked[1]))
        {
            m_isPaused = false;
        }
    }

    // ------------------------------------------------------------------------
    // Banner Grafico Pausa Gioco Attiva (Centrato in alto)
    // ------------------------------------------------------------------------
    if (m_isPaused && m_state == GameState::PLAYING)
    {
        float pBannerW = 540.0f;
        float pBannerH = 68.0f;
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, 68.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(pBannerW, pBannerH));
        ImGui::Begin("GamePausedBanner", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Dummy(ImVec2(0.0f, 2.0f));
        ImGui::SetWindowFontScale(1.22f);
        renderCenteredText("|| GIOCO IN PAUSA (REGOLAZIONE IMPOSTAZIONI)", ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
        ImGui::SetWindowFontScale(0.95f);
        renderCenteredText("Clicca sullo sfondo cosmico o su 'Riprendi' per tornare alla navigazione!", ImVec4(0.85f, 1.0f, 0.9f, 1.0f));
        ImGui::End();
    }

    // Finestra Principale HUD ("Mission Control & Impostazioni")
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(340, 240), ImVec2(480, 680));
    ImGui::Begin("Astro-Miner 2D - Mission Control & Impostazioni", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Controlli Rapidi Pausa / Ripresa Gioco:
    if (m_isPaused)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.70f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.85f, 0.35f, 1.0f));
        if (ImGui::Button(" > RIPRENDI PARTITA (o clicca sullo schermo) ", ImVec2(-1, 32)))
        {
            m_isPaused = false;
        }
        ImGui::PopStyleColor(2);
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "[PAUSA ATTIVA: Simulazione fisica sospesa]");
    }
    else
    {
        if (ImGui::Button(" || METTI IN PAUSA ", ImVec2(-1, 24)))
        {
            m_isPaused = true;
        }
    }
    ImGui::Checkbox("Pausa automatica cliccando sulle impostazioni", &m_autoPauseOnSettings);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.2f, 0.9f, 1.0f, 1.0f), "LIVELLO ATTUALE: %d", m_currentLevel);
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "PUNTEGGIO TOTALE: %d", m_player.score());

    // Vite visualizzate graficamente come cuori <3:
    ImGui::Text("VITE:");
    ImGui::SameLine();
    for (int i = 0; i < m_player.lives(); ++i)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.3f, 1.0f), "<3 ");
        ImGui::SameLine();
    }
    ImGui::NewLine();

    // Barra di progresso monete raccolte nel livello:
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "MONETE: %d / %d", m_collectibles.collectedCoins(), m_collectibles.totalCoins());
    float coinProgress = static_cast<float>(m_collectibles.collectedCoins()) / static_cast<float>(m_collectibles.totalCoins());
    ImGui::ProgressBar(coinProgress, ImVec2(240, 20));

    if (!m_collectibles.allCollected())
    {
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Fase 1: Raccogli tutte le 5 monete d'oro!");
    }
    else
    {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), ">> PORTALE ATTIVO! ENTRA NEL WARP GATE! <<");
    }

    // Indicatore di Stato del Drone Rosso di Hermite:
    if (m_hermite.isWeakened())
    {
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), "Drone Rosso: INDEBOLITO (%.1fs) - Innocuo!", m_hermite.getWeakenedTime());
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Drone Rosso: ATTIVO E PERICOLOSO");
    }

    // Badge Indicatori Power-Up Attivi (v2):
    if (m_player.isShieldActive())
    {
        ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "[SCUDO DEFLETTORE: %.1fs]", m_player.shieldTimeRemaining());
    }
    if (m_player.isSpreadShotActive())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.15f, 1.0f), "[LASER TRIPLO: %.1fs]", m_player.spreadShotTimeRemaining());
    }
    if (m_player.isMagnetActive())
    {
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "[CALAMITA MONETE: %.1fs]", m_player.magnetTimeRemaining());
    }

    // Indicatore Afterburner & Arsenale (v2):
    ImGui::Text("Afterburner [Shift]:");
    ImGui::SameLine();
    ImGui::ProgressBar(m_player.boostPercent(), ImVec2(120, 14), m_player.isBoosting() ? "BOOST!" : "");

    if (m_empCooldown <= 0.0f)
    {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.85f, 1.0f), "[BOMBA EMP PRONTA: Premi 'E' o Click Destro]");
    }
    else
    {
        ImGui::TextDisabled("Bomba EMP in ricarica: %.1fs", m_empCooldown);
    }

    // ========================================================================
    // SEZIONE 1: IMPOSTAZIONI NAVICELLA & VOLO (ORIGINALI)
    // ========================================================================
    if (ImGui::CollapsingHeader("Impostazioni Navicella & Volo", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Slider interattivo per la velocità di navigazione (Requisito didattico di laboratorio):
        float pSpeed = m_player.speed();
        if (ImGui::SliderFloat("Velocita' Navicella", &pSpeed, 120.0f, 850.0f, "%.0f px/s"))
        {
            m_player.setSpeed(pSpeed);
        }
        ImGui::TextDisabled("Scorciatoia: premi Shift + Su/Giu o +/- per regolare la velocita'");

        ImGui::Checkbox("Fari Proiettori Volumetrici (L)", &m_player.headlightsRef());

        if (ImGui::Button("Personalizza Livrea Navetta..."))
        {
            m_showCustomizer = !m_showCustomizer;
        }
        ImGui::SameLine();
        // Hot-reload a caldo: dealloca la texture precedente (glDeleteTextures) e ricarica
        // da disco player_face.bmp/.png/.jpg senza riavviare l'applicazione.
        if (ImGui::Button("Ricarica Texture Cabina"))
        {
            m_player.reloadFaceTexture("assets/textures/player_face.bmp") ||
            m_player.reloadFaceTexture("assets/textures/player_face.png") ||
            m_player.reloadFaceTexture("assets/textures/player_face.jpg");
        }

        if (ImGui::Button("Riavvia Partita"))
        {
            restartGame();
        }
    }

    // ========================================================================
    // SEZIONE 2: GRAFICA, POST-PROCESSING & SHADERS (ORIGINALI + v2)
    // ========================================================================
    if (ImGui::CollapsingHeader("Grafica, Post-Processing & Shaders", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Spunta Wireframe (Requisito didattico di laboratorio):
        ImGui::Checkbox("Modalita' Wireframe (Mesh reticolari)", &m_wireframe);

        // Regolazione interattiva dell'effetto psichedelico dello sfondo:
        ImGui::SliderFloat("Effetto Psichedelico Sfondo", &m_psychedelic, 0.0f, 1.0f, "%.2f");

        // Spunta traiettoria Lissajous del Drone:
        ImGui::Checkbox("Mostra Traiettoria Hermite (Lissajous)", m_hermite.getShowOrbitPtr());

        // Controlli Post-Processing FBO:
        ImGui::Checkbox("Filtro CRT Retro-Arcade", &m_postProcess.crtEnabled());
        if (m_postProcess.crtEnabled())
        {
            ImGui::SliderFloat("Scanlines", &m_postProcess.scanlineIntensity(), 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Aberrazione RGB", &m_postProcess.chromaticAberration(), 0.0f, 0.012f, "%.4f");
        }
        ImGui::SliderFloat("Vignettatura Cosmica", &m_postProcess.vignetteIntensity(), 0.0f, 1.0f, "%.2f");

        if (ImGui::SmallButton("Test Shockwave al Centro"))
        {
            m_postProcess.triggerShockwave(glm::vec2(m_screenWidth * 0.5f, m_screenHeight * 0.5f), static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), 0.06f, 0.75f);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Photo Mode Freeze-Frame [P]"))
        {
            m_photoMode = true;
        }
    }

    // ========================================================================
    // SEZIONE 3: AUDIO & SIMULAZIONE DIDATTICA (v2)
    // ========================================================================
    if (ImGui::CollapsingHeader("Audio & Simulazione Didattica (v2)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        bool soundMuted = AudioEngine::instance().isMuted();
        if (ImGui::Checkbox("Muto Audio", &soundMuted))
        {
            AudioEngine::instance().setMuted(soundMuted);
        }
        ImGui::SameLine();
        ImGui::SliderFloat("Volume SFX", &AudioEngine::instance().volumeRef(), 0.0f, 1.0f, "%.2f");

        ImGui::SliderFloat("Velocita' Simulazione (Slow-Mo)", &m_timeScale, 0.1f, 2.0f, "%.2fx");
        if (ImGui::SmallButton("0.25x (Matrix)")) m_timeScale = 0.25f;
        ImGui::SameLine();
        if (ImGui::SmallButton("1.0x (Normale)")) m_timeScale = 1.0f;
        ImGui::SameLine();
        if (ImGui::SmallButton("2.0x (Rapido)")) m_timeScale = 2.0f;
    }

    // ========================================================================
    // SEZIONE 4: SUPER-ARMI, MECCANICHE SPAZIALI & SANDBOX (v2)
    // ========================================================================
    if (ImGui::CollapsingHeader("Super-Armi, Meccaniche Spaziali & Sandbox (v2)", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button(m_empCooldown <= 0.0f ? "Innesca Bomba EMP [E]" : "EMP in Ricarica..."))
        {
            if (m_empCooldown <= 0.0f) triggerEmpBomb();
        }
        ImGui::SameLine();
        if (ImGui::Button("Lancia Missili Homing [Q]"))
        {
            if (m_missiles.canFire())
            {
                glm::vec2 pDir = m_player.direction();
                glm::vec2 pSide(-pDir.y, pDir.x);
                m_missiles.fire(m_player.position() + pSide * 26.0f, pDir);
                m_missiles.fire(m_player.position() - pSide * 26.0f, pDir);
                m_missiles.resetCooldown(0.85f);
                AudioEngine::instance().playLaser();
            }
        }

        bool bhActive = m_blackHole.isActive();
        if (ImGui::Checkbox("Attiva Buco Nero (Attrazione Gravitazionale)", &bhActive))
        {
            m_blackHole.setActive(bhActive);
        }
        ImGui::SameLine();
        ImGui::Checkbox("Boss Overlord (Hermite v2)", m_hermite.getBossModePtr());

        ImGui::Text("Genera Power-Up al volo:");
        ImGui::SameLine();
        if (ImGui::SmallButton("Scudo"))
        {
            m_powerUps.spawn(m_player.position() + glm::vec2(60.0f, 0.0f), PowerUpType::SHIELD);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Laser Triplo"))
        {
            m_powerUps.spawn(m_player.position() + glm::vec2(60.0f, 0.0f), PowerUpType::SPREAD_SHOT);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Calamita"))
        {
            m_powerUps.spawn(m_player.position() + glm::vec2(60.0f, 0.0f), PowerUpType::MAGNET);
        }

        ImGui::Checkbox("Scanner Radar Olografico", &m_showRadar);
        ImGui::SameLine();
        ImGui::Checkbox("Classifica Piloti", &m_showLeaderboard);

        ImGui::Checkbox("Modalita' Sandbox (Editor Live)", &m_sandboxMode);
        if (m_sandboxMode)
        {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.7f, 1.0f), "[MODALITA' SANDBOX - EDITOR DI LIVELLO]");
            const char* spawnTypes[] = { "Asteroide", "Moneta Cosmica", "Power-Up Scudo", "Power-Up Laser Triplo", "Power-Up Calamita", "Buco Nero" };
            ImGui::Combo("Entita' da generare", &m_sandboxSpawnType, spawnTypes, IM_ARRAYSIZE(spawnTypes));
            ImGui::TextDisabled("Fai click col mouse nell'area di gioco per generare l'oggetto!");

            if (ImGui::SmallButton("Genera 5 Asteroidi"))
            {
                for (int i = 0; i < 5; ++i)
                {
                    float rx = 100.0f + static_cast<float>(rand() % (m_screenWidth - 200));
                    float ry = 100.0f + static_cast<float>(rand() % (m_screenHeight - 200));
                    m_asteroids.spawnOne(glm::vec2(rx, ry), 0.8f + static_cast<float>(rand() % 5) * 0.1f);
                }
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Pulisci Asteroidi"))
            {
                m_asteroids.clear();
            }

            if (ImGui::SmallButton("Ricarica Scudo"))
            {
                m_player.activateShield(18.0f);
                AudioEngine::instance().playPowerUp();
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Cannoni Tripli"))
            {
                m_player.activateSpreadShot(15.0f);
                AudioEngine::instance().playPowerUp();
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Calamita (15s)"))
            {
                m_player.activateMagnet(15.0f);
                AudioEngine::instance().playPowerUp();
            }
        }
    }

    // ========================================================================
    // SEZIONE 5: REGOLE, COMANDI DI GIOCO & STATISTICHE (ORIGINALI)
    // ========================================================================
    if (ImGui::CollapsingHeader("Comandi di Gioco, Regole & Info"))
    {
        ImGui::Text("Asteroidi rimasti: %d", m_asteroids.activeCount());
        ImGui::Text("FPS: %.1f (dt: %.4f s)", io.Framerate, dt);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "Regole di Gioco:");
        ImGui::BulletText("Alien Drone (Rosso): Colpiscilo con il laser per INDEBOLIRLO e renderlo innocuo!");
        ImGui::BulletText("Warp Gate (Portale): Raccogli 5 monete e volaci dentro per chiudere la partita!");

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "Comandi di Gioco:");
        ImGui::BulletText("WASD / Frecce: Movimento navetta con scia al plasma");
        ImGui::BulletText("Spazio: Sparo Laser minerario");
        ImGui::BulletText("Q: Missili intelligenti homing a ricerca termica");
        ImGui::BulletText("E / Click Destro: Bomba ad impulsi EMP");
        ImGui::BulletText("Shift: Afterburner accelerazione warp");
        ImGui::BulletText("L: Fari proiettori volumetrici conici");
        ImGui::BulletText("P: Modalita' Foto Freeze-Frame studio");
    }

    ImGui::End();

    // ------------------------------------------------------------------------
    // Cantiere Navale - Personalizzazione Livrea (v2)
    // ------------------------------------------------------------------------
    if (m_showCustomizer)
    {
        ImGui::SetNextWindowSize(ImVec2(340, 310), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Cantiere Navale - Personalizzazione Livrea (v2)", &m_showCustomizer))
        {
            ImGui::TextColored(ImVec4(0.3f, 0.95f, 1.0f, 1.0f), "PRESET CROMATICI RAPIDI:");
            if (ImGui::Button("Cobalt (Default)")) m_player.applyPresetCobalt();
            ImGui::SameLine();
            if (ImGui::Button("Phoenix (Fuoco)")) m_player.applyPresetPhoenix();
            if (ImGui::Button("Void (Spettrale)")) m_player.applyPresetVoid();
            ImGui::SameLine();
            if (ImGui::Button("Matrix (Cyber)")) m_player.applyPresetMatrix();

            ImGui::Separator();
            ImGui::Text("Colorazione Componenti RGB:");
            ImGui::ColorEdit3("Fusoliera & Muso", &m_player.hullColorRef().x);
            ImGui::ColorEdit3("Ali a Delta", &m_player.wingColorRef().x);
            ImGui::ColorEdit3("Vetro Cabina", &m_player.cockpitColorRef().x);
            ImGui::ColorEdit3("Fiamme Reattori", &m_player.thrusterColorRef().x);
        }
        ImGui::End();
    }

    // ------------------------------------------------------------------------
    // Finestra Classifica Piloti (Leaderboard)
    // ------------------------------------------------------------------------
    if (m_showLeaderboard)
    {
        ImGui::SetNextWindowSize(ImVec2(360, 270), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Classifica Piloti Cosmici (Leaderboard)", &m_showLeaderboard))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "TOP PILOTI ASTRO-MINER (v2)");
            ImGui::Separator();

            if (ImGui::BeginTable("LeaderboardTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("Nome Pilota", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Punti (Liv)", ImGuiTableColumnFlags_WidthFixed, 95.0f);
                ImGui::TableHeadersRow();

                for (size_t i = 0; i < m_leaderboard.size(); ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    if (i == 0) ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.1f, 1.0f), "#%zu", i + 1);
                    else if (i == 1) ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.9f, 1.0f), "#%zu", i + 1);
                    else if (i == 2) ImGui::TextColored(ImVec4(0.85f, 0.55f, 0.2f, 1.0f), "#%zu", i + 1);
                    else ImGui::Text("#%zu", i + 1);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", m_leaderboard[i].name.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d (L%d)", m_leaderboard[i].score, m_leaderboard[i].level);
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    // ------------------------------------------------------------------------
    // Banner di Allerta: Portale Attivo (Centrato in alto)
    // ------------------------------------------------------------------------
    if (m_state == GameState::PLAYING && m_collectibles.allCollected()) // Mostra il banner quando tutte le 5 monete sono state raccolte
    {
        float bannerWidth = 560.0f;  // Larghezza fissa del banner in pixel
        float bannerHeight = 68.0f;  // Altezza fissa del banner in pixel
        // Posiziona il banner centrato orizzontalmente (pivot X=0.5) a 32px dal bordo superiore.
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, 32.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(bannerWidth, bannerHeight)); // Applica le dimensioni fisse alla finestra
        ImGui::Begin("GateActivePrompt", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove); // Finestra senza barra titolo né trascinamento
        ImGui::Dummy(ImVec2(0.0f, 2.0f)); // Piccola spaziatura verticale superiore
        ImGui::SetWindowFontScale(1.25f); // Ingrandisce il font per il titolo del prompt
        renderCenteredText(">> PORTALE ATTIVO! ENTRA NEL WARP GATE! <<", ImVec4(0.2f, 1.0f, 0.45f, 1.0f)); // Testo verde brillante
        ImGui::SetWindowFontScale(0.95f); // Ripristina la scala del font per il sottotitolo
        renderCenteredText("Vola dentro il Warp Gate a destra per chiudere la partita con l'HURRA'!", ImVec4(0.85f, 0.95f, 1.0f, 1.0f)); // Istruzione per il giocatore
        ImGui::End(); // Chiude il contesto della finestra ImGui
    }

    // Helper condiviso per disegnare un banner modale centrato (HURRÀ o Game Over) evitando codice UI duplicato.
    auto renderCenteredModal = [&](
        const char* id, float width, float height,
        const char* title, const ImVec4& titleColor, float titleScale,
        const char* buttonLabel, float buttonWidth, auto&& drawBody)
    {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(width, height));
        ImGui::Begin(id, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::Dummy(ImVec2(0.0f, 14.0f));
        ImGui::SetWindowFontScale(titleScale);
        renderCenteredText(title, titleColor);
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        drawBody();

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::SetCursorPosX((width - buttonWidth) * 0.5f);
        bool clicked = ImGui::Button(buttonLabel, ImVec2(buttonWidth, 38));
        ImGui::End();
        return clicked;
    };

    // ------------------------------------------------------------------------
    // Banner Vittoria: "HURRÀ! SEI ENTRATO NEL WARP GATE!" (Perfettamente centrato sullo schermo)
    // ------------------------------------------------------------------------
    if (m_state == GameState::LEVEL_CLEARED_HURRA)
    {
        bool goNext = renderCenteredModal(
            "CelebrationBanner", 500.0f, 240.0f,
            "HURRA'! SEI ENTRATO NEL WARP GATE!", ImVec4(1.0f, 0.90f, 0.10f, 1.0f), 1.75f,
            "Passa Subito al Prossimo Livello >>", 380.0f,
            [&]() {
                renderCenteredText("Salto dimensionale completato con successo!", ImVec4(0.3f, 1.0f, 0.4f, 1.0f));
                renderCenteredText("Bonus Livello: +500 Punti!", ImVec4(0.2f, 0.9f, 1.0f, 1.0f));
                char nextLvlBuf[128];
                snprintf(nextLvlBuf, sizeof(nextLvlBuf), "Generazione del Livello %d in corso (%.1f s)...", m_currentLevel + 1, m_hurraTimer);
                renderCenteredText(nextLvlBuf, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
            }
        );
        if (goNext) nextLevel();
    }
    // ------------------------------------------------------------------------
    // Banner Game Over: (Perfettamente centrato sullo schermo)
    // ------------------------------------------------------------------------
    else if (m_state == GameState::GAME_OVER)
    {
        bool retry = renderCenteredModal(
            "GameOverBanner", 420.0f, 210.0f,
            "GAME OVER!", ImVec4(1.0f, 0.25f, 0.25f, 1.0f), 1.8f,
            "Riprova Partita", 280.0f,
            [&]() {
                ImGui::Dummy(ImVec2(0.0f, 2.0f));
                renderCenteredText("La navicella e' andata distrutta!", ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
                char scoreBuf[64];
                snprintf(scoreBuf, sizeof(scoreBuf), "Punteggio finale: %d", m_player.score());
                renderCenteredText(scoreBuf, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
            }
        );
        if (retry) restartGame();
    }

    // ------------------------------------------------------------------------
    // Gestione click per spawn nella modalita' Sandbox / Editor (v2):
    // ------------------------------------------------------------------------
    if (m_sandboxMode && !io.WantCaptureMouse && ImGui::IsMouseClicked(0))
    {
        float wx = io.MousePos.x;
        float wy = static_cast<float>(m_screenHeight) - io.MousePos.y;
        if (wx >= 20.0f && wx <= static_cast<float>(m_screenWidth) - 20.0f &&
            wy >= 20.0f && wy <= static_cast<float>(m_screenHeight) - 20.0f)
        {
            glm::vec2 spawnPos(wx, wy);
            switch (m_sandboxSpawnType)
            {
                case 0: m_asteroids.spawnOne(spawnPos, 1.0f); break;
                case 1: m_collectibles.spawnOne(spawnPos); break;
                case 2: m_powerUps.spawn(spawnPos, PowerUpType::SHIELD); break;
                case 3: m_powerUps.spawn(spawnPos, PowerUpType::SPREAD_SHOT); break;
                case 4: m_powerUps.spawn(spawnPos, PowerUpType::MAGNET); break;
                case 5: m_blackHole.setPosition(spawnPos); m_blackHole.setActive(true); break;
            }
            AudioEngine::instance().playPowerUp();
        }
    }

    // ------------------------------------------------------------------------
    // Scanner Radar Olografico Tattico (v2):
    // ------------------------------------------------------------------------
    if (m_showRadar)
    {
        renderRadar(static_cast<float>(glfwGetTime()));
    }
}

// ============================================================================
// Helper Condiviso: Preparazione e Avvio di un Livello
// ============================================================================
void GameWorld::startLevel(int level, bool clearParticles)
{
    m_currentLevel = level;                   // Imposta il numero del livello corrente
    m_state = GameState::PLAYING;             // Riporta la FSM in stato di gioco attivo
    m_lasers.clear();                         // Rimuove tutti i proiettili laser residui
    if (clearParticles)
    {
        m_particles.clear();                  // Svuota eventuali particelle residue al restart
    }
    m_hazards.reset();                        // Azzera i pericoli laterali attivi
    m_hazards.setLevel(m_currentLevel);       // Scala la difficoltà dei pericoli al livello target
    m_hermite.setLevel(m_currentLevel);       // Aggiorna la velocità del Drone al livello target
    m_hermite.resetWeakness();                // Ripristina lo stato attivo del Drone Rosso

    // Riposiziona il giocatore al centro dello schermo (il punteggio cumulativo non viene mai azzerato).
    m_player.reset(glm::vec2(static_cast<float>(m_screenWidth) * 0.5f, static_cast<float>(m_screenHeight) * 0.5f));
    // Rigenera le 5 monete d'oro per il livello corrente.
    m_collectibles.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));
    // Rigenera il campo di asteroidi per il livello corrente.
    m_asteroids.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));
}

// ============================================================================
// Avanzamento al Livello Successivo
// ============================================================================
void GameWorld::nextLevel()
{
    // Incrementa il livello.
    m_currentLevel++;
    m_state = GameState::PLAYING;
    m_lasers.clear();
    m_missiles.clear();
    m_hazards.reset();
    m_hazards.setLevel(m_currentLevel);
    m_hermite.setLevel(m_currentLevel);
    m_hermite.resetWeakness();
    m_scoreSubmitted = false;

    // Riposiziona il giocatore al centro dello schermo (punteggio mantenuto cumulativo).
    m_player.reset(glm::vec2(static_cast<float>(m_screenWidth) * 0.5f, static_cast<float>(m_screenHeight) * 0.5f));

    // Rigenera le 5 monete d'oro in nuove posizioni per il livello corrente.
    m_collectibles.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));

    // Rigenera un nuovo campo di asteroidi con conteggio aumentato.
    m_asteroids.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));
}

// ============================================================================
// Riavvio della Partita (Restart)
// ============================================================================
void GameWorld::restartGame()
{
    // Resetta il contatore del livello al primo.
    m_currentLevel = 1;
    m_state = GameState::PLAYING;
    m_lasers.clear();
    m_missiles.clear();
    m_particles.clear();
    m_hazards.reset();
    m_hazards.setLevel(m_currentLevel);
    m_hermite.setLevel(m_currentLevel);
    m_hermite.resetWeakness();
    m_scoreSubmitted = false;

    // Ripristina la navicella (il punteggio cumulativo di sessione non si azzera).
    m_player.reset(glm::vec2(static_cast<float>(m_screenWidth) * 0.5f, static_cast<float>(m_screenHeight) * 0.5f));
    m_collectibles.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));
    m_asteroids.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));
}

// ============================================================================
// Scanner Radar Olografico Tattico (v2)
// Visualizzatore vettoriale ImGui in tempo reale di tutte le entita' spaziali
// ============================================================================
void GameWorld::renderRadar(float currentTime)
{
    ImGui::SetNextWindowSize(ImVec2(240, 305), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_screenWidth) - 260.0f, 20.0f), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    if (ImGui::Begin("Scanner Radar Olografico (v2)", &m_showRadar, flags))
    {
        ImVec2 canvas_sz(210.0f, 210.0f);
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Sfondo circolare del radar
        ImVec2 center = ImVec2(canvas_p0.x + canvas_sz.x * 0.5f, canvas_p0.y + canvas_sz.y * 0.5f);
        float radius = canvas_sz.x * 0.5f - 4.0f;

        drawList->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(10, 15, 22, 240), 6.0f);
        drawList->AddCircleFilled(center, radius, IM_COL32(6, 26, 16, 235), 48);
        drawList->AddCircle(center, radius, IM_COL32(35, 180, 90, 220), 48, 2.0f);

        // Cerchi concentrici di portata
        drawList->AddCircle(center, radius * 0.33f, IM_COL32(35, 140, 75, 90), 32, 1.0f);
        drawList->AddCircle(center, radius * 0.66f, IM_COL32(35, 140, 75, 110), 32, 1.0f);

        // Reticolo a croce
        drawList->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), IM_COL32(35, 140, 75, 80), 1.0f);
        drawList->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), IM_COL32(35, 140, 75, 80), 1.0f);

        // Raggio rotante dello sweep del radar
        float sweepAngle = currentTime * 2.2f;
        ImVec2 sweepEnd = ImVec2(center.x + std::cos(sweepAngle) * radius, center.y + std::sin(sweepAngle) * radius);
        drawList->AddLine(center, sweepEnd, IM_COL32(90, 255, 140, 210), 1.8f);

        // Funzione di mappatura da coordinate mondo [0, W] x [0, H] a coordinate radar ImGui
        // Nel mondo Y=0 e' in basso, mentre nel canvas ImGui Y cresce verso il basso.
        auto worldToRadar = [&](const glm::vec2& wPos) -> ImVec2
        {
            float nx = (wPos.x - m_screenWidth * 0.5f) / (m_screenWidth * 0.5f);
            float ny = - (wPos.y - m_screenHeight * 0.5f) / (m_screenHeight * 0.5f);
            return ImVec2(center.x + nx * (radius * 0.90f), center.y + ny * (radius * 0.90f));
        };

        // 1. Buco Nero (Viola scuro pulsante)
        if (m_blackHole.isActive())
        {
            ImVec2 bhPos = worldToRadar(m_blackHole.position());
            drawList->AddCircleFilled(bhPos, 5.0f, IM_COL32(180, 50, 255, 230), 16);
            drawList->AddCircle(bhPos, 7.5f, IM_COL32(230, 120, 255, 180), 16, 1.0f);
        }

        // 2. Warp Gate (Verde smeraldo / Ciano splendente)
        if (m_hermite.gatePosition().x > 0.0f)
        {
            ImVec2 gatePos = worldToRadar(m_hermite.gatePosition());
            drawList->AddCircleFilled(gatePos, 4.5f, IM_COL32(0, 240, 180, 230), 16);
            drawList->AddCircle(gatePos, 6.5f, IM_COL32(140, 255, 220, 200), 16, 1.2f);
        }

        // 3. Drone Sentinella Rosso (Rosso se pericoloso, Giallo-Verde se indebolito)
        {
            ImVec2 dronePos = worldToRadar(m_hermite.dronePosition());
            ImU32 droneCol = m_hermite.isWeakened() ? IM_COL32(120, 255, 80, 255) : IM_COL32(255, 45, 45, 255);
            drawList->AddCircleFilled(dronePos, 4.0f, droneCol, 14);
            drawList->AddCircle(dronePos, 6.0f, IM_COL32(255, 255, 255, 160), 14, 1.0f);

            // Blip micro-droni di scorta del Boss (v2):
            if (m_hermite.isBoss())
            {
                for (int i = 0; i < 2; ++i)
                {
                    if (m_hermite.isEscortActive(i))
                    {
                        ImVec2 escBlip = worldToRadar(m_hermite.escortPos(i));
                        drawList->AddCircleFilled(escBlip, 2.5f, IM_COL32(255, 120, 120, 230), 8);
                    }
                }
            }
        }

        // 4. Asteroidi (Punti grigio chiaro o ciano per i cristallini)
        for (const auto& ast : m_asteroids.asteroids())
        {
            if (ast.active)
            {
                ImVec2 aPos = worldToRadar(ast.pos);
                ImU32 aCol = ast.isCrystal ? IM_COL32(100, 235, 255, 240) : IM_COL32(190, 195, 205, 210);
                drawList->AddCircleFilled(aPos, ast.isCrystal ? 2.8f : 2.2f, aCol, 8);
            }
        }

        // 5. Monete d'Oro Cosmiche (Punti dorati accesi)
        for (const auto& coin : m_collectibles.coins())
        {
            if (!coin.collected)
            {
                ImVec2 cPos = worldToRadar(coin.currentPos);
                drawList->AddCircleFilled(cPos, 2.8f, IM_COL32(255, 215, 30, 255), 10);
            }
        }

        // 6. Power-Up (Punti azzurri / ciano con anello)
        for (const auto& pu : m_powerUps.instances())
        {
            ImVec2 pPos = worldToRadar(pu.pos);
            drawList->AddCircleFilled(pPos, 3.5f, IM_COL32(50, 220, 255, 255), 10);
            drawList->AddCircle(pPos, 5.0f, IM_COL32(255, 255, 255, 200), 10, 1.0f);
        }

        // 7. Navicella del Giocatore (Triangolo verde brillante orientato o blip con scudo)
        {
            ImVec2 plPos = worldToRadar(m_player.position());
            drawList->AddCircleFilled(plPos, 3.8f, IM_COL32(40, 255, 110, 255), 12);
            if (m_player.isShieldActive())
            {
                drawList->AddCircle(plPos, 6.2f, IM_COL32(70, 200, 255, 255), 12, 1.5f);
            }
        }

        // Riserva lo spazio per il canvas in ImGui
        ImGui::Dummy(canvas_sz);

        // Legenda compatta
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "[*] Tu");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "[*] Drone");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.1f, 1.0f), "[$] Oro");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2f, 0.85f, 1.0f, 1.0f), "[+] Bonus");
    }
    ImGui::End();
}

// ============================================================================
// Innesco Bomba ad Impulsi Elettromagnetici EMP (v2)
// ============================================================================
void GameWorld::triggerEmpBomb()
{
    m_empCooldown = m_maxEmpCooldown;

    // 1. Onda d'urto FBO rifrattiva massiccia
    m_postProcess.triggerShockwave(m_player.position(), static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight), 0.085f, 0.95f);
    triggerScreenShake(0.38f, 14.0f);

    // 2. Impulso fisico circolare sugli asteroidi vicini (repulsione newtoniana inversa)
    m_asteroids.applyImpulse(m_player.position(), 520.0f, 420.0f);

    // 3. Stordisce il Drone Nemico se nel raggio d'azione
    int empBonus = 0;
    m_hermite.checkLaserHit(m_hermite.dronePosition(), 120.0f, m_particles, empBonus);
    if (empBonus > 0) m_player.addScore(empBonus);

    // 4. Annulla tutti i pericoli e raggi laser nemici attivi
    m_hazards.reset();

    // 5. Emette l'onda di archi al plasma azzurri
    m_particles.emitEmpWave(m_player.position(), 340.0f, 54);

    // 6. Effetto audio dedicato
    AudioEngine::instance().playShieldDeflect();
}

