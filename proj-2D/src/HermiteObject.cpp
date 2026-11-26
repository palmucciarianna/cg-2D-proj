// ORIGINE: In parte basato sul modulo Hermite dei laboratori della prof (spline cubiche), esteso con logica di gioco Alien Sentinel Drone e Warp Gate.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: HermiteObject.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Realizza l'Alien Sentinel Drone e il portale monumentale Cosmic Warp Gate (Requisito 3.2).
//    - Guida il drone lungo un'orbita a 8 di Lissajous ed esegue l'ammiccamento (eye-blink).
//    - Gestisce la rotazione a doppio anello e l'attivazione del Warp Gate per l'HURRÀ.
// 2. A CHI È COLLEGATO:
//    - Carica mesh da PlayerShapeLoader (alien_drone.obj, warp_gate.obj); gestito da GameWorld.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - Spline di Hermite C^1: C(t) = h00*P0 + h01*P1 + h10*D0 + h11*D1 (Domanda 12).
//    - Traiettoria a 8 di Lissajous: x = x0 + A*cos(phi), y = y0 + B*sin(2*phi) (Domanda 11).
//    - Animazione ammiccamento occhio: scala Y modulata da campana sinusoidale (Requisito 6).
// ============================================================================

#include "HermiteObject.h"
#include "PlayerShapeLoader.h"
#include "ShapeGenerator.h"
#include "MathConstants.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>

// ============================================================================
// Costruttore
// ============================================================================
HermiteManager::HermiteManager()
{
}

// ============================================================================
// Inizializzazione Geometrie Hermite e Assemblaggio Parti del Viso
// ============================================================================
void HermiteManager::init(const std::string& alienObjPath, const std::string& warpGateObjPath)
{
    std::string err;

    // ------------------------------------------------------------------------
    // 1. Caricamento della Testa Cornuta Hermite da File OBJ
    // ------------------------------------------------------------------------
    MeshData headData;
    if (PlayerShapeLoader::load(alienObjPath, headData, err))
    {
        m_headGeo.uploadMesh(headData);
        m_hasDrone = true;
    }
    else
    {
        std::cerr << "[ERRORE] Caricamento modello Hermite Drone (" << alienObjPath << "): " << err << "\n";
    }

    // ------------------------------------------------------------------------
    // 2. Assemblaggio dei Tratti del Viso Cattivo
    // ------------------------------------------------------------------------
    // Occhio Giallo Cattivo: grande cerchio dorato brillante con bordo arancio vivo.
    MeshData eyeData = ShapeGenerator::createCircle(
        0.0f, 0.0f, 13.0f, 13.0f, 24,
        glm::vec4(1.0f, 0.98f, 0.20f, 1.0f),
        glm::vec4(1.0f, 0.65f, 0.05f, 1.0f)
    );
    m_evilEyeGeo.uploadMesh(eyeData);

    // Pupilla a fessura rettangolare verticale con sfumatura rosso sangue.
    MeshData pupilData = ShapeGenerator::createRectangle(
        3.5f, 18.0f,
        glm::vec4(0.05f, 0.05f, 0.05f, 1.0f),
        glm::vec4(0.40f, 0.02f, 0.02f, 1.0f)
    );
    m_pupilGeo.uploadMesh(pupilData);

    // Sopracciglio triangolare inclinato aggressivo.
    MeshData browData = ShapeGenerator::createTriangle(
        glm::vec4(0.1f, 0.05f, 0.05f, 1.0f),   // Vertice base scuro
        glm::vec4(0.6f, 0.05f, 0.05f, 1.0f),   // Vertice rosso intenso
        glm::vec4(0.05f, 0.02f, 0.02f, 1.0f)   // Vertice ombra esterna
    );
    m_eyebrowsGeo.uploadMesh(browData); // Carica VAO/VBO sopracciglia in VRAM

    // Bocca a ghigno malefico con zanna aguzza.
    MeshData mouthData = ShapeGenerator::createTriangle(
        glm::vec4(0.10f, 0.02f, 0.02f, 1.0f),  // Angolo sinistro scuro
        glm::vec4(0.10f, 0.02f, 0.02f, 1.0f),  // Angolo destro scuro
        glm::vec4(0.95f, 0.95f, 0.95f, 1.0f)   // Punta zanna bianca
    );
    m_evilMouthGeo.uploadMesh(mouthData); // Carica VAO/VBO bocca in VRAM

    // ------------------------------------------------------------------------
    // 3. Caricamento del Warp Gate (Portale Monumentale Hermite)
    // ------------------------------------------------------------------------
    MeshData gateData; // Struttura CPU per l'anello del Warp Gate
    if (PlayerShapeLoader::load(warpGateObjPath, gateData, err)) // Parsing del file OBJ/MTL a spline Hermite
    {
        m_gateGeo.uploadMesh(gateData); // Trasferisce i 144 vertici in VRAM
        m_hasGate = true;   // Segnala caricamento riuscito
    }
    else // Gestione errore se il file OBJ manca
    {
        std::cerr << "[ERRORE] Caricamento modello Hermite Gate (" << warpGateObjPath << "): " << err << "\n";
    }

    // Vortice di energia interno (cerchio ciano a sfumatura radiale).
    MeshData vortexData = ShapeGenerator::createCircle(
        0.0f, 0.0f, 30.0f, 30.0f, 32,
        glm::vec4(0.8f, 0.95f, 1.0f, 0.90f), // Nucleo centrale luminoso
        glm::vec4(0.1f, 0.45f, 0.85f, 0.30f) // Alone esterno sfumato
    );
    m_vortexGeo.uploadMesh(vortexData); // [Domanda 12: Curve interpolanti di Hermite C^1 per Drone e Warp Gate] Carica VAO/VBO del vortice in VRAM

    // ------------------------------------------------------------------------
    // 4. Tracciato dell'Orbita di Lissajous a 8 (Visualizzazione Didattica)
    // ------------------------------------------------------------------------
    std::vector<glm::vec3> orbitPos;
    std::vector<glm::vec4> orbitCol;
    const int segments = 120;
    for (int i = 0; i <= segments; ++i)
    {
        float t = (i * 6.2831853f) / static_cast<float>(segments);
        float x = 640.0f + 340.0f * std::cos(t);
        float y = 380.0f + 160.0f * std::sin(t * 2.0f);
        orbitPos.push_back(glm::vec3(x, y, 0.0f));
        // Ciano fluorescente con alpha morbido per eleganza visiva
        orbitCol.push_back(glm::vec4(0.25f, 0.95f, 0.85f, 0.38f));
    }
    m_orbitLineGeo.setData(orbitPos, orbitCol, GL_LINE_STRIP);
    m_orbitLineGeo.upload();
}

// ============================================================================
// Aggiornamento Cinematica: Lissajous e Animazione Sbattimento Occhio
// ============================================================================
void HermiteManager::update(float dt, float currentTime, const glm::vec2& playerPos)
{
    // Gestione del timer di lampo visivo da colpo laser
    if (m_hitFlashTimer > 0.0f)
    {
        m_hitFlashTimer -= dt;
    }

    // Calcolo della velocità base in funzione del livello:
    float speedMultiplier = 1.0f + (m_level - 1) * 0.12f;

    // Se il Drone Rosso è stato colpito dal laser, entra in stato indebolito:
    bool weakened = (m_weakenedTimer > 0.0f);
    if (weakened)
    {
        m_weakenedTimer -= dt; // Decremento del timer di indebolimento residuo
        speedMultiplier *= 0.35f; // Rallentamento drastico del 65%: moto faticoso e vulnerabile
    }

    float omega = 0.85f * speedMultiplier; // velocità angolare orbitale in rad/s (più veloce ai livelli alti)
    m_orbitPhase += omega * dt; // Aggiornamento della fase orbitale per la traiettoria a 8

    // [Domanda 11: Curve parametriche e continuità C^1 (orbita chiusa regolare di Lissajous)]
    // 1. Traiettoria orbitale armonica a figura di 8
    // x(t) = x_0 + A * cos(phi)
    // y(t) = y_0 + B * sin(2 * phi)
    // Ampiezza orizzontale A = 340px, ampiezza verticale B = 160px.
    glm::vec2 baseOrbit(
        640.0f + 340.0f * std::cos(m_orbitPhase),
        380.0f + 160.0f * std::sin(m_orbitPhase * 2.0f)
    );

    // 2. Meccanica dello scatto imprevedibile ("giri strani" casuali):
    // Quando è indebolito, il drone NON può eseguire scatti predatori!
    if (!weakened)
    {
        m_dashCooldown -= dt;
        if (m_dashCooldown <= 0.0f)
        {
            // Attiva una manovra rapida di 0.75s con direzione casuale:
            m_dashTimer = 0.75f;
            float randAngle = static_cast<float>(rand() % 360) * 0.0174533f;
            m_dashDirection = glm::vec2(std::cos(randAngle), std::sin(randAngle)); // direzione unitaria casuale
            // costruisco un versore di offset iniziale per il dash (per evitare che parta da zero)

            // Prossimo scatto tra 4.5 e 8.0 secondi (più frequente nei livelli alti):
            float minInterval = std::max(2.8f, 5.0f - (m_level - 1) * 0.4f);
            m_dashCooldown = minInterval + static_cast<float>(rand() % 30) * 0.1f; 
            // tra quanti secondi il drone potrà scattare di nuovo
        }
    }
    else
    {
        // Se colpito durante uno scatto, azzera istantaneamente la deviazione
        m_dashTimer = 0.0f;
        m_dashOffset = glm::vec2(0.0f);
    }

    if (m_dashTimer > 0.0f)
    {
        m_dashTimer -= dt; // quando il timer scende a 0, il drone torna alla traiettoria normale
        float progress = 1.0f - (m_dashTimer / 0.75f); // 0.0 all'inizio, 1.0 alla fine

        // Profilo a campana sinusoidale (starts at 0, peaks at 1, returns to 0):
        float bell = std::sin(progress * MathConstants::PI);

        // Giro strano: slancio direzionale combinato con un loop a vortice ad alta frequenza
        float loopRadius = 60.0f * bell; // raggio spirale si allarga e si restringe con la campana
        float loopAngle = progress * MathConstants::TWO_PI * 2.5f; // 2.5 rotazioni complete durante lo scatto
        m_dashOffset = m_dashDirection * (bell * 110.0f) + glm::vec2(std::cos(loopAngle) * loopRadius, std::sin(loopAngle) * loopRadius);
        // somma due movimenti: uno slancio + la spirale circolare

        // Inclina testa del drone durante la manovra
        m_droneAngle = std::sin(progress * MathConstants::TWO_PI * 2.0f) * 45.0f;
    }
    else
    {
        m_dashOffset = glm::vec2(0.0f);
        if (weakened)
        {
            // Se indebolito, la testa ondeggia in un moto stordito / barcollante:
            m_droneAngle = std::sin(currentTime * 8.0f) * 12.0f;
        }
        else
        {
            m_droneAngle = 0.0f; // testa dritta quando il drone è in orbita normale
        }
    }

    // Posizione effettiva del drone: orbita base + eventuale deviazione dello scatto
    m_dronePos = baseOrbit + m_dashOffset;

    // Aggiornamento droni di scorta e raggio di puntamento (Boss Mode v2):
    if (m_isBoss)
    {
        float escSpeed = 2.4f;
        m_escortsPos[0] = m_dronePos + glm::vec2(std::cos(currentTime * escSpeed), std::sin(currentTime * escSpeed)) * 64.0f;
        m_escortsPos[1] = m_dronePos + glm::vec2(std::cos(currentTime * escSpeed + 3.14159f), std::sin(currentTime * escSpeed + 3.14159f)) * 64.0f;

        if (!weakened && m_dashCooldown <= 0.85f)
        {
            m_targetingActive = true;
            m_targetAimPos = playerPos;
        }
        else
        {
            m_targetingActive = false;
        }
    }
    else
    {
        m_targetingActive = false;
    }

    // Fluttuazione armonica del portale cosmico in alto a destra.
    m_gatePos.x = 1080.0f + 25.0f * std::cos(currentTime * 0.5f);
    m_gatePos.y = 560.0f + 20.0f * std::sin(currentTime * 0.7f);

    // ------------------------------------------------------------------------
    // Gestione dell'Interazione Didattica: Sbattimento dell'Occhio (Eye-Blink)
    // "Quando ci passo sopra sbatte l'occhio UNA volta!"
    // ------------------------------------------------------------------------
    float dist = glm::distance(playerPos, m_dronePos); // distanza tra giocatore e drone
    bool isOver = (dist < 65.0f); // edge-trigger: attiva se il giocatore è entro 65px dal drone

    if (isOver)
    {
        // Se il giocatore entra nell'area d'influenza per la prima volta (edge-trigger):
        if (!m_wasPlayerOver && m_eyeBlinkTimer <= 0.0f)
        {
            m_eyeBlinkTimer = 0.38f; // Attiva l'animazione per 0.38 secondi
        }
        m_wasPlayerOver = true; // Ricorda che il giocatore è sopra il drone
    }
    else
    {
        // Resetta la memoria quando il giocatore si allontana, permettendo un nuovo blink al prossimo passaggio.
        m_wasPlayerOver = false;
    }

    // Decrementa i timer temporizzati con il delta-time.
    if (m_eyeBlinkTimer > 0.0f)
    {
        m_eyeBlinkTimer -= dt;
    }

    if (m_relicPulseTimer > 0.0f)
    {
        m_relicPulseTimer -= dt;
    }

    // Integrazione delle rotazioni del portale:
    // L'anello esterno ruota a 22 gradi/secondo
    // il vortice interno ruota vorticosamente a 100 gradi/secondo.
    m_gateRotation += 22.0f * dt;
    m_vortexRotation += 100.0f * dt;
}

// ============================================================================
// Rendering degli Oggetti Curvilinei di Hermite
// ============================================================================
void HermiteManager::draw(const Shader& shader, float currentTime, bool gateActive)
{
    // ------------------------------------------------------------------------
    // 0. Tracciato Didattico dell'Orbita di Lissajous a 8
    // ------------------------------------------------------------------------
    if (m_showOrbit)
    {
        shader.setMat4("model", glm::mat4(1.0f));
        shader.setVec4("uColorTint", glm::vec4(1.0f));
        m_orbitLineGeo.draw();
    }

    // ------------------------------------------------------------------------
    // 1. Disegno dell'Alien Sentinel Drone
    // ------------------------------------------------------------------------
    if (m_hasDrone)
    {
        bool weakened = (m_weakenedTimer > 0.0f);
        float currentScale = weakened ? 0.95f : 1.25f;

        // Modulazione cromatica di danno e indebolimento:
        if (m_hitFlashTimer > 0.0f)
        {
            // Flash bianco-oro rovente al momento dell'impatto laser
            shader.setVec4("uColorTint", glm::vec4(2.2f, 2.0f, 1.3f, 1.0f));
        }
        else if (weakened)
        {
            // Tinta sbiadita/pulsante viola scuro e blu: energia vitale affievolita
            float pulse = 0.70f + 0.25f * std::sin(currentTime * 9.0f);
            shader.setVec4("uColorTint", glm::vec4(0.85f * pulse, 0.45f * pulse, 0.85f * pulse, 0.85f));
        }
        else
        {
            shader.setVec4("uColorTint", glm::vec4(1.0f));
        }

        // Matrice radice posizionata sul punto della traiettoria di Lissajous.
        glm::mat4 headRoot = glm::mat4(1.0f);
        headRoot = glm::translate(headRoot, glm::vec3(m_dronePos, 0.0f));
        headRoot = glm::rotate(headRoot, glm::radians(m_droneAngle), glm::vec3(0, 0, 1));
        headRoot = glm::scale(headRoot, glm::vec3(currentScale, currentScale, 1.0f));

        // A) Disegna la testa cornuta rossa Hermite.
        shader.setMat4("model", headRoot);
        m_headGeo.draw();

        // B) Calcolo dell'ammiccamento: scala verticale Y dell'occhio da 1.0 a 0.08 e ritorno.
        // Se indebolito, l'occhio rimane socchiuso e stordito!
        float eyeScaleY = 1.0f;
        if (m_eyeBlinkTimer > 0.0f)
        {
            float progress = m_eyeBlinkTimer / 0.38f;
            eyeScaleY = std::abs(std::sin(progress * MathConstants::PI)); // Oscillazione da 1.0 a 0.0 e ritorno
            eyeScaleY = std::max(eyeScaleY, 0.08f); // Clamping per mantenere la fessura visibile
        }
        else if (weakened)
        {
            // Occhio socchiuso e stordito da impatto subito:
            eyeScaleY = 0.25f + 0.12f * std::sin(currentTime * 5.0f);
        }

        // C) Occhio Giallo Centrale posizionato gerarchicamente rispetto alla testa.
        glm::mat4 eyeMat = glm::translate(headRoot, glm::vec3(0.0f, 4.0f, 0.0f));
        eyeMat = glm::scale(eyeMat, glm::vec3(1.15f, eyeScaleY, 1.0f));
        shader.setMat4("model", eyeMat);
        m_evilEyeGeo.draw();

        // D) Pupilla verticale all'interno dell'occhio.
        glm::mat4 pupilMat = glm::scale(eyeMat, glm::vec3(1.0f, 0.9f, 1.0f)); // Scala leggermente sull'asse Y
        shader.setMat4("model", pupilMat); // Invia matrice pupilla
        m_pupilGeo.draw();                 // Disegna pupilla nera

        // E) Sopracciglio Destro inclinato di +28 gradi.
        glm::mat4 browRight = glm::translate(headRoot, glm::vec3(10.0f, 15.0f, 0.0f)); // Posizione sopra l'occhio DX
        browRight = glm::rotate(browRight, glm::radians(28.0f), glm::vec3(0, 0, 1));   // Inclinazione aggressiva +28°
        browRight = glm::scale(browRight, glm::vec3(18.0f, 5.0f, 1.0f));               // Dimensionamento rettangolare
        shader.setMat4("model", browRight); // Invia matrice sopracciglio DX
        m_eyebrowsGeo.draw();               // Disegna sopracciglio DX

        // F) Sopracciglio Sinistro inclinato di -28 gradi.
        glm::mat4 browLeft = glm::translate(headRoot, glm::vec3(-10.0f, 15.0f, 0.0f)); // Posizione sopra l'occhio SX
        browLeft = glm::rotate(browLeft, glm::radians(-28.0f), glm::vec3(0, 0, 1));   // Inclinazione simmetrica -28°
        browLeft = glm::scale(browLeft, glm::vec3(-18.0f, 5.0f, 1.0f));               // Riflessione speculare X=-18
        shader.setMat4("model", browLeft); // Invia matrice sopracciglio SX
        m_eyebrowsGeo.draw();              // Riusa la mesh del sopracciglio

        // G) Bocca a ghigno malefico posizionata nella parte inferiore del volto.
        glm::mat4 mouthMat = glm::translate(headRoot, glm::vec3(0.0f, -18.0f, 0.0f)); // Offset inferiore bocca
        mouthMat = glm::scale(mouthMat, glm::vec3(22.0f, 8.0f, 1.0f));                // Larghezza 22px, altezza 8px
        shader.setMat4("model", mouthMat); // Invia matrice bocca
        m_evilMouthGeo.draw();             // Disegna bocca con zanna

        // H) Disegno Droni di Scorta e Puntamento Laser (Boss Mode v2)
        if (m_isBoss)
        {
            for (int i = 0; i < 2; ++i)
            {
                if (m_escortsActive[i])
                {
                    glm::mat4 escMat = glm::translate(glm::mat4(1.0f), glm::vec3(m_escortsPos[i], 0.0f));
                    escMat = glm::scale(escMat, glm::vec3(0.52f, 0.52f, 1.0f));
                    shader.setMat4("model", escMat);
                    shader.setVec4("uColorTint", glm::vec4(1.3f, 0.45f, 0.45f, 1.0f));
                    m_headGeo.draw();
                }
            }

            if (m_targetingActive)
            {
                float blink = std::sin(currentTime * 24.0f);
                if (blink > 0.0f)
                {
                    std::vector<glm::vec3> linePts = {
                        glm::vec3(m_dronePos, 0.0f),
                        glm::vec3(m_targetAimPos, 0.0f)
                    };
                    std::vector<glm::vec4> lineCols = {
                        glm::vec4(1.0f, 0.1f, 0.1f, 0.90f),
                        glm::vec4(1.0f, 0.3f, 0.3f, 0.20f)
                    };
                    m_targetingLineGeo.setData(linePts, lineCols, GL_LINES);
                    m_targetingLineGeo.upload();
                    shader.setMat4("model", glm::mat4(1.0f));
                    shader.setVec4("uColorTint", glm::vec4(1.0f));
                    m_targetingLineGeo.draw();
                }
            }
        }

        // Ripristina uColorTint predefinito (1.0f) per le geometrie successive:
        shader.setVec4("uColorTint", glm::vec4(1.0f));
    }

    // ------------------------------------------------------------------------
    // 2. Disegno del Cosmic Warp Gate (Portale Monumentale)
    // ------------------------------------------------------------------------
    if (m_hasGate)
    {
        glm::mat4 gateRoot = glm::mat4(1.0f);                            // Matrice identità base portale
        gateRoot = glm::translate(gateRoot, glm::vec3(m_gatePos, 0.0f)); // Trasla alla posizione fluttuante

        bool activeVisual = (gateActive || m_relicPulseTimer > 0.0f); // Attivo visivamente se tutte le monete raccolte o toccato
        if (activeVisual)
        {
            // Portale Attivo: pulsazione luminosa radiale ad alta frequenza (10 rad/s).
            float pulse = 1.0f + 0.18f * std::sin(currentTime * 10.0f);

            // Alone al plasma "Neon Glow" attorno al portale attivo (Blending Additivo):
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glm::mat4 glowMat = glm::scale(gateRoot, glm::vec3(1.75f * pulse, 1.75f * pulse, 1.0f));
            shader.setVec4("uColorTint", glm::vec4(0.2f, 0.85f, 1.0f, 0.25f));
            shader.setMat4("model", glowMat);
            m_vortexGeo.draw();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Vortice attivo a rotazione accelerata.
            glm::mat4 vortexMat = glm::rotate(gateRoot, glm::radians(m_vortexRotation * 1.5f), glm::vec3(0, 0, 1)); // Rotazione veloce
            vortexMat = glm::scale(vortexMat, glm::vec3(1.4f * pulse, 1.4f * pulse, 1.0f)); // Scala pulsante
            shader.setVec4("uColorTint", glm::vec4(0.3f, 1.0f, 0.95f, 1.0f)); // Tinta ciano brillante
            shader.setMat4("model", vortexMat); // Invia matrice vortice
            m_vortexGeo.draw();                 // Disegna vortice interno

            // Anello monumentale Hermite.
            glm::mat4 ringMat = glm::rotate(gateRoot, glm::radians(m_gateRotation * 1.5f), glm::vec3(0, 0, 1)); // Rotazione anello
            ringMat = glm::scale(ringMat, glm::vec3(1.35f * pulse, 1.35f * pulse, 1.0f)); // Scala pulsante
            shader.setVec4("uColorTint", glm::vec4(0.4f, 0.85f, 1.0f, 1.0f)); // Tinta azzurro elettrico
            shader.setMat4("model", ringMat); // Invia matrice anello Hermite
            m_gateGeo.draw();                 // Disegna i 144 vertici Hermite
        }
        else
        {
            // Portale Dormiente (in attesa che il giocatore raccolga le 5 monete).
            glm::mat4 vortexMat = glm::rotate(gateRoot, glm::radians(m_vortexRotation * 0.4f), glm::vec3(0, 0, 1)); // Rotazione lenta
            vortexMat = glm::scale(vortexMat, glm::vec3(1.15f, 1.15f, 1.0f)); // Scala moderata
            shader.setVec4("uColorTint", glm::vec4(0.2f, 0.4f, 0.8f, 0.75f)); // Tinta blu tenue semitrasparente
            shader.setMat4("model", vortexMat); // Invia matrice vortice dormiente
            m_vortexGeo.draw();                 // Disegna vortice

            glm::mat4 ringMat = glm::rotate(gateRoot, glm::radians(m_gateRotation * 0.5f), glm::vec3(0, 0, 1)); // Rotazione lenta anello
            ringMat = glm::scale(ringMat, glm::vec3(1.15f, 1.15f, 1.0f)); // Scala fissa 1.15
            shader.setVec4("uColorTint", glm::vec4(0.35f, 0.6f, 0.95f, 0.9f)); // Tinta blu sobria
            shader.setMat4("model", ringMat); // Invia matrice anello dormiente
            m_gateGeo.draw();                 // Disegna anello Hermite
        }

        // Ripristina la tinta colore globale.
        shader.setVec4("uColorTint", glm::vec4(1.0f));
    }
}

// ============================================================================
// Rilevamento Ingresso nel Portale di Fine Livello
// ============================================================================
bool HermiteManager::checkPlayerGateEnter(const CircleCollider& playerCol, bool gateActive)
{
    if (!m_hasGate || !gateActive) return false;

    // Raggio di attivazione del trigger circolare: 58 pixel (raggio punto = 0.0f).
    return Collision2D::checkCircleCircle(playerCol.center, 0.0f, m_gatePos, 58.0f);
}

// ============================================================================
// Rilevamento Collisione con il Drone Sentinella
// ============================================================================
bool HermiteManager::checkDroneCollision(const CircleCollider& playerCol) const
{
    if (!m_hasDrone) return false;
    float droneRadius = (m_weakenedTimer > 0.0f) ? 28.0f : 36.0f;
    return Collision2D::checkCircleCircle(playerCol.center, playerCol.radius, m_dronePos, droneRadius);
}

// ============================================================================
// Rilevamento Impatto Laser sul Drone Rosso (Indebolimento)
// ============================================================================
bool HermiteManager::checkLaserHit(const glm::vec2& bulletPos, float bulletRadius, ParticleSystem& particles, int& score)
{
    if (!m_hasDrone)
    {
        return false;
    }

    // Modalita' Boss Hunter: verifica prima l'impatto con i droni di scorta
    if (m_isBoss)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_escortsActive[i] && glm::distance(bulletPos, m_escortsPos[i]) < (18.0f + bulletRadius))
            {
                m_escortsActive[i] = false;
                particles.emitDroneHitSparks(m_escortsPos[i], 24);
                score += 80;
                return true;
            }
        }
    }

    float hitRadius = (m_weakenedTimer > 0.0f) ? 30.0f : (m_isBoss ? 42.0f : 36.0f);

    if (Collision2D::checkCircleCircle(bulletPos, bulletRadius, m_dronePos, hitRadius))
    {
        m_hitFlashTimer = 0.20f;
        m_hitCount++;

        if (m_isBoss)
        {
            m_bossHp--;
            particles.emitDroneHitSparks(bulletPos, 22);

            if (m_bossHp <= 0)
            {
                // Boss sconfitto e stordito a lungo!
                m_weakenedTimer = 8.0f;
                m_bossHp = m_bossMaxHp;
                m_escortsActive[0] = true;
                m_escortsActive[1] = true;
                score += 350; // Super bonus boss
            }
            else
            {
                score += 40;
            }
        }
        else
        {
            // 1. Applica o estende lo stato di indebolimento (4.5 secondi di durata)
            m_weakenedTimer = 4.5f;

            // 2. Interrompe istantaneamente qualsiasi scatto o virata violenta in corso
            m_dashTimer = 0.0f;
            m_dashOffset = glm::vec2(0.0f);
            m_dashCooldown = 6.0f; // Rinvia il prossimo scatto aggressivo

            // 3. Occhio socchiuso e ammiccante da stordimento
            triggerEyeBlink();

            // 4. Emissione particelle di scintille elettrificate e dispersione energetica
            particles.emitDroneHitSparks(bulletPos, 18);

            // 5. Assegna punti bonus al giocatore per aver indebolito la sentinella aliena
            score += 35;
        }

        return true;
    }

    return false;
}

// ============================================================================
// Interazione di Prossimità con la Reliquia Spaziale
// ============================================================================
bool HermiteManager::checkRelicInteraction(const CircleCollider& playerCol, ParticleSystem& particles, int& score)
{
    if (!m_hasGate) return false;

    float dist = glm::distance(playerCol.center, m_gatePos);
    bool isNear = (dist < 85.0f);

    if (isNear)
    {
        // Se il giocatore si è appena avvicinato:
        if (!m_wasPlayerNearRelic)
        {
            m_wasPlayerNearRelic = true;
            m_relicPulseTimer = 1.2f;
            // Emette un'ondata di particelle cosmiche ciano.
            particles.emitRelicEnergy(m_gatePos, 22);
            score += 50; // Assegna punti bonus
            return true;
        }
    }
    else if (dist > 120.0f)
    {
        // Resetta lo stato quando il giocatore si allontana a sufficienza.
        m_wasPlayerNearRelic = false;
    }

    return false;
}

// ============================================================================
// Bounding Colliders
// ============================================================================
CircleCollider HermiteManager::getDroneCollider() const
{
    return CircleCollider{ m_dronePos, (m_weakenedTimer > 0.0f) ? 26.0f : 32.0f };
}

CircleCollider HermiteManager::getGateCollider() const
{
    return CircleCollider{ m_gatePos, 48.0f };
}
