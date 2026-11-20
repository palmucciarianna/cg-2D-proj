// ============================================================================
// File: src/GameWorld.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del GameWorld.
//              - Orchestrazione del game loop (aggiornamento fisico e rendering)
//              - Costruzione della matrice di proiezione ortografica glm::ortho
//              - Risoluzione geometrica di tutte le collisioni tra entità
//              - Interfaccia grafica ImGui con auto-compressione non invasiva
//              - Gestione transizioni di stato (PLAYING, LEVEL_CLEARED_HURRA, GAME_OVER)
// ============================================================================

#include "GameWorld.h"
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
    m_particles.init();
    m_hazards.init();
    m_player.init();
    // Colloca la navicella esattamente al centro dello schermo (640, 360).
    m_player.reset(glm::vec2(m_screenWidth * 0.5f, m_screenHeight * 0.5f));

    m_collectibles.init();
    m_asteroids.init();
    m_lasers.init();

    // Carica i modelli poligonali delle spline di Hermite (Drone Nemico e Portale Warp Gate).
    std::string alienObj = resolveAsset(assetDir, "models/alien_drone.obj");
    std::string warpObj  = resolveAsset(assetDir, "models/warp_gate.obj");
    m_hermite.init(alienObj, warpObj);

    // ------------------------------------------------------------------------
    // 3. Generazione Procedurale delle Entità per il Livello 1
    // ------------------------------------------------------------------------
    m_collectibles.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));
    m_asteroids.spawnLevel(m_currentLevel, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));

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
void GameWorld::update(float dt, GLFWwindow* window)
{
    float currentTime = static_cast<float>(glfwGetTime());

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
                m_player.takeDamage(); // Toglie 1 vita su 3
                m_particles.emitAsteroidExplosion(m_player.position(), 24);
                triggerScreenShake(0.35f, 14.0f);
            }
        }

        // Sparo Laser con Barra Spaziatrice:
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && m_player.canFire())
        {
            // Calcola il versore di direzione balistica in base all'angolo della navicella.
            float rad = glm::radians(m_player.angle());
            glm::vec2 dir(std::cos(rad), std::sin(rad));

            // Spara il raggio con offset di 36px in avanti rispetto alla prua.
            m_lasers.fire(m_player.position() + dir * 36.0f, dir);

            // Imposta il cooldown di ricarica cannone a 0.18s.
            m_player.resetFireCooldown(0.18f);
        }

        // Aggiorna i colpi laser attivi e gestisce le collisioni con asteroidi e Drone Rosso.
        int prevScore = m_player.score();
        int score = prevScore;
        m_lasers.update(simDt, m_asteroids, m_hermite, m_particles, score);
        if (score != prevScore)
        {
            m_player.addScore(score - prevScore);
            triggerScreenShake(0.16f, 5.0f);
        }

        // Aggiorna le entità del mondo con il delta-time di simulazione.
        m_asteroids.update(simDt);
        m_collectibles.update(simDt, currentTime);
        m_hermite.update(simDt, currentTime, m_player.position());
        m_particles.update(simDt);

        // Raccolta delle Monete d'Oro (+100 punti per ciascuna moneta):
        int collectedNow = m_collectibles.checkCollisions(m_player.getCollider(), m_particles);
        if (collectedNow > 0)
        {
            m_player.addScore(collectedNow * 100);
        }

        // Interazione facoltativa di prossimità con la reliquia monumentale (+50 punti bonus):
        int relicScore = 0;
        if (!m_collectibles.allCollected() && m_hermite.checkRelicInteraction(m_player.getCollider(), m_particles, relicScore))
        {
            m_player.addScore(relicScore);
        }

        // --------------------------------------------------------------------
        // Collisione con l'Alien Sentinel Drone (quello Rosso):
        // --------------------------------------------------------------------
        if (m_hermite.checkDroneCollision(m_player.getCollider()))
        {
            if (m_hermite.isWeakened())
            {
                // Drone indebolito: innocuo! Scintille di rimbalzo senza perdita di vite
                m_particles.emitDroneHitSparks(m_hermite.dronePosition(), 12);
            }
            else if (!m_player.isInvulnerable())
            {
                m_player.takeDamage();
                m_hermite.triggerEyeBlink();
                m_particles.emitAsteroidExplosion(m_player.position(), 24);
                triggerScreenShake(0.35f, 13.0f);
            }
        }

        // Collisione con gli asteroidi rocciosi in transito:
        if (m_asteroids.checkPlayerCollision(m_player.getCollider()))
        {
            m_player.takeDamage();
            m_particles.emitAsteroidExplosion(m_player.position(), 16);
            triggerScreenShake(0.30f, 11.0f);
        }

        // Se le vite sono terminate, scatta il Game Over:
        if (!m_player.isAlive())
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
            nextLevel();
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
    // 1. Disegna il background cosmico procedurale (stelle e nebulosa in NDC).
    m_bgRenderer->draw(currentTime, m_psychedelic, m_currentLevel);

    // 2. Modalità Wireframe (Requisito didattico di laboratorio):
    // Se la spunta è attiva, imposta la rasterizzazione reticolare (GL_LINE)
    // per esaminare la triangolazione di tutte le mesh 2D della scena.
    if (m_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // 3. Configura la matrice di Proiezione Ortografica per lo spazio di gioco:
    // Mappa lo spazio [0, screenWidth] su X e [0, screenHeight] su Y.
    // L'origine (0, 0) è l'angolo inferiore sinistro dello schermo.
    m_basicShader->use();
    glm::mat4 projection = glm::ortho(
        0.0f, static_cast<float>(m_screenWidth),
        0.0f, static_cast<float>(m_screenHeight),
        -1.0f, 1.0f
    );

    // Applicazione del trauma della telecamera (Screen Shake) per impatti ed esplosioni:
    if (glm::length(m_shakeOffset) > 0.001f)
    {
        projection = glm::translate(projection, glm::vec3(m_shakeOffset, 0.0f));
    }

    m_basicShader->setMat4("projection", projection);
    m_basicShader->setVec4("uColorTint", glm::vec4(1.0f));

    // A) Rendering delle istanze condivise di asteroidi:
    m_asteroids.draw(*m_basicShader, currentTime);

    // B) Rendering degli oggetti con curve di Hermite (Alien Drone e Warp Gate):
    bool gateActive = (m_state == GameState::LEVEL_CLEARED_HURRA || m_collectibles.allCollected());
    m_hermite.draw(*m_basicShader, currentTime, gateActive);

    // C) Rendering delle monete d'oro oscillanti con rotazione pseudo-3D:
    m_collectibles.draw(*m_basicShader, currentTime);

    // D) Rendering dei colpi laser al plasma:
    m_lasers.draw(*m_basicShader);

    // E) Rendering dei pericoli laterali e dei segnali stroboscopici di allerta:
    m_hazards.draw(*m_basicShader, currentTime, static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));

    // F) Rendering del sistema particellare (detriti, scintille, coriandoli):
    m_particles.draw(*m_basicShader);

    // G) Rendering gerarchico della navicella Starfighter del giocatore:
    if (m_player.isAlive())
    {
        m_player.draw(*m_basicShader, currentTime, projection);
    }

    // 4. Ripristino Modalità Piena (GL_FILL):
    // Assicura che l'interfaccia Dear ImGui e i testi vengano sempre disegnati
    // in modalità solida (senza artefatti reticolari sui caratteri).
    if (m_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

// ============================================================================
// Rendering dell'Interfaccia Utente Grafica (Dear ImGui HUD)
// ============================================================================
void GameWorld::renderUI(float dt)
{
    ImGuiIO& io = ImGui::GetIO();

    // ------------------------------------------------------------------------
    // Logica di Auto-Compressione Non Invasiva dell'HUD:
    // All'avvio la finestra è completamente espansa per mostrare le istruzioni.
    // Al primo click eseguito sull'area di gioco (ma NON sui widget ImGui come gli slider),
    // la finestra si riduce automaticamente per liberare la visuale di gioco.
    // Il controllo !io.WantCaptureMouse garantisce che l'interazione con l'HUD non chiuda la finestra!
    // ------------------------------------------------------------------------
    if (!m_hudCompact)
    {
        bool clickedOnGameArea = !io.WantCaptureMouse && (io.MouseClicked[0] || io.MouseClicked[1]);
        if (clickedOnGameArea)
        {
            m_hudCompact = true;
        }
    }

    // Finestra Principale HUD ("Mission Control")
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin("Astro-Miner 2D - Mission Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

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
    ImGui::ProgressBar(coinProgress, ImVec2(m_hudCompact ? 160 : 240, m_hudCompact ? 16 : 22));

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

    if (m_hudCompact)
    {
        // Vista Compatta: spunta rapida Wireframe e pulsante per riespandere i dettagli
        ImGui::Separator();
        ImGui::Checkbox("Wireframe", &m_wireframe);
        ImGui::SameLine();
        if (ImGui::SmallButton("Espandi Dettagli [+]"))
        {
            m_hudCompact = false;
        }
        ImGui::SliderFloat("Effetto Psichedelico", &m_psychedelic, 0.0f, 1.0f, "%.2f");

        ImGui::SliderFloat("Slow-Motion", &m_timeScale, 0.1f, 2.0f, "%.2fx");
        ImGui::Checkbox("Classifica Piloti", &m_showLeaderboard);
    }
    else
    {
        // Vista Dettagliata: spiegazione regole, comandi e controlli avanzati
        ImGui::Text("Asteroidi rimasti: %d", m_asteroids.activeCount());

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "Regole di Gioco:");
        ImGui::BulletText("Alien Drone (Rosso): Colpiscilo con il laser per INDEBOLIRLO e renderlo innocuo!");
        ImGui::BulletText("Warp Gate (Portale): Raccogli 5 monete e volaci dentro per chiudere la partita!");

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "Comandi di Gioco:");
        ImGui::BulletText("Frecce Direzionali: Spostamento della navicella (con scia al plasma)");
        ImGui::BulletText("Barra Spazio: Sparo Laser per frantumare asteroidi e indebolire il Drone Rosso");

        // Slider interattivo per la velocità di navigazione:
        float pSpeed = m_player.speed();
        if (ImGui::SliderFloat("Velocita' Navicella", &pSpeed, 120.0f, 850.0f, "%.0f px/s"))
        {
            m_player.setSpeed(pSpeed);
        }
        ImGui::TextDisabled("Scorciatoia: premi Shift + Su/Giu o +/- per regolare la velocita'");

        // Controlli Simulazione Didattica (v2):
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.3f, 0.95f, 1.0f, 1.0f), "Controlli Simulazione & Debug (v2):");
        ImGui::SliderFloat("Velocita' Simulazione (Slow-Mo)", &m_timeScale, 0.1f, 2.0f, "%.2fx");
        ImGui::SameLine();
        if (ImGui::SmallButton("0.25x (Matrix)")) m_timeScale = 0.25f;
        ImGui::SameLine();
        if (ImGui::SmallButton("1.0x (Normale)")) m_timeScale = 1.0f;
        ImGui::SameLine();
        if (ImGui::SmallButton("2.0x (Rapido)")) m_timeScale = 2.0f;

        ImGui::Checkbox("Mostra Traiettoria Hermite (Lissajous)", m_hermite.getShowOrbitPtr());
        ImGui::SameLine();
        ImGui::Checkbox("Classifica Piloti", &m_showLeaderboard);

        // Regolazione interattiva dell'effetto psichedelico dello sfondo
        ImGui::SliderFloat("Effetto Psichedelico", &m_psychedelic, 0.0f, 1.0f, "%.2f");
        ImGui::Text("FPS: %.1f (dt: %.4f s)", io.Framerate, dt);

        if (ImGui::Button("Riavvia Partita"))
        {
            restartGame();
        }

        ImGui::SameLine();
        if (ImGui::Button("Ricarica Texture Cabina"))
        {
            m_player.reloadFaceTexture("assets/textures/player_face.bmp") ||
            m_player.reloadFaceTexture("assets/textures/player_face.png") ||
            m_player.reloadFaceTexture("assets/textures/player_face.jpg");
        }

        ImGui::Separator();
        ImGui::TextDisabled("(Fai click sullo schermo per iniziare e compattare la finestra)");
        if (ImGui::SmallButton("Riduci Finestra [-]"))
        {
            m_hudCompact = true;
        }
    }
    ImGui::End();

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

    // Funzione lambda per centrare orizzontalmente le scritte all'interno dei banner ImGui.
    auto renderCenteredText = [](const char* text, const ImVec4& color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
    {
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPosX(std::max(0.0f, (windowWidth - textWidth) * 0.5f));
        ImGui::TextColored(color, "%s", text);
    };

    // ------------------------------------------------------------------------
    // Banner di Allerta: Portale Attivo (Centrato in alto)
    // ------------------------------------------------------------------------
    if (m_state == GameState::PLAYING && m_collectibles.allCollected())
    {
        float bannerWidth = 560.0f;
        float bannerHeight = 68.0f;
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, 32.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(bannerWidth, bannerHeight));
        ImGui::Begin("GateActivePrompt", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Dummy(ImVec2(0.0f, 2.0f));
        ImGui::SetWindowFontScale(1.25f);
        renderCenteredText(">> PORTALE ATTIVO! ENTRA NEL WARP GATE! <<", ImVec4(0.2f, 1.0f, 0.45f, 1.0f));
        ImGui::SetWindowFontScale(0.95f);
        renderCenteredText("Vola dentro il Warp Gate a destra per chiudere la partita con l'HURRA'!", ImVec4(0.85f, 0.95f, 1.0f, 1.0f));
        ImGui::End();
    }

    // ------------------------------------------------------------------------
    // Banner Vittoria: "HURRÀ! SEI ENTRATO NEL WARP GATE!" (Perfettamente centrato sullo schermo)
    // ------------------------------------------------------------------------
    if (m_state == GameState::LEVEL_CLEARED_HURRA)
    {
        float bannerWidth = 500.0f;
        float bannerHeight = 240.0f;
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(bannerWidth, bannerHeight));
        ImGui::Begin("CelebrationBanner", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::Dummy(ImVec2(0.0f, 14.0f));
        ImGui::SetWindowFontScale(1.75f);
        renderCenteredText("HURRA'! SEI ENTRATO NEL WARP GATE!", ImVec4(1.0f, 0.90f, 0.10f, 1.0f));
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        renderCenteredText("Salto dimensionale completato con successo!", ImVec4(0.3f, 1.0f, 0.4f, 1.0f));
        renderCenteredText("Bonus Livello: +500 Punti!", ImVec4(0.2f, 0.9f, 1.0f, 1.0f));

        char nextLvlBuf[128];
        snprintf(nextLvlBuf, sizeof(nextLvlBuf), "Generazione del Livello %d in corso (%.1f s)...", m_currentLevel + 1, m_hurraTimer);
        renderCenteredText(nextLvlBuf, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        float btnWidth = 380.0f;
        ImGui::SetCursorPosX((bannerWidth - btnWidth) * 0.5f);
        if (ImGui::Button("Passa Subito al Prossimo Livello >>", ImVec2(btnWidth, 38)))
        {
            nextLevel();
        }
        ImGui::End();
    }
    // ------------------------------------------------------------------------
    // Banner Game Over: (Perfettamente centrato sullo schermo)
    // ------------------------------------------------------------------------
    else if (m_state == GameState::GAME_OVER)
    {
        float bannerWidth = 420.0f;
        float bannerHeight = 210.0f;
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(bannerWidth, bannerHeight));
        ImGui::Begin("GameOverBanner", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::Dummy(ImVec2(0.0f, 14.0f));
        ImGui::SetWindowFontScale(1.8f);
        renderCenteredText("GAME OVER!", ImVec4(1.0f, 0.25f, 0.25f, 1.0f));
        ImGui::SetWindowFontScale(1.0f);

        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, 6.0f));

        renderCenteredText("La navicella e' andata distrutta!", ImVec4(0.95f, 0.95f, 0.95f, 1.0f));

        char scoreBuf[64];
        snprintf(scoreBuf, sizeof(scoreBuf), "Punteggio finale: %d", m_player.score());
        renderCenteredText(scoreBuf, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        float btnWidth = 280.0f;
        ImGui::SetCursorPosX((bannerWidth - btnWidth) * 0.5f);
        if (ImGui::Button("Riprova Partita", ImVec2(btnWidth, 38)))
        {
            restartGame();
        }
        ImGui::End();
    }
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
