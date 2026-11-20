// ============================================================================
// File: src/HermiteObject.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del gestore degli oggetti basati su curve di Hermite.
//              - Alien Sentinel Drone: segue una traiettoria a forma di 8 (curva di Lissajous),
//                modula la scala Y dell'occhio per l'ammiccamento (blink) quando sorvolato dal player.
//              - Cosmic Warp Gate: portale a vortice rotante bifase (dormiente/attivo),
//                costruito da anello cavo Hermite e disco di interpolazione ciano.
// ============================================================================

#include "HermiteObject.h"
#include "PlayerShapeLoader.h"
#include "ShapeGenerator.h"
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
        m_headGeo.setData(headData.positions, headData.colors, headData.primitiveType);
        m_headGeo.upload();
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
    m_evilEyeGeo.setData(eyeData.positions, eyeData.colors, eyeData.primitiveType);
    m_evilEyeGeo.upload();

    // Pupilla a fessura rettangolare verticale con sfumatura rosso sangue.
    MeshData pupilData = ShapeGenerator::createRectangle(
        3.5f, 18.0f,
        glm::vec4(0.05f, 0.05f, 0.05f, 1.0f),
        glm::vec4(0.40f, 0.02f, 0.02f, 1.0f)
    );
    m_pupilGeo.setData(pupilData.positions, pupilData.colors, pupilData.primitiveType);
    m_pupilGeo.upload();

    // Sopracciglio triangolare inclinato aggressivo.
    MeshData browData = ShapeGenerator::createTriangle(
        glm::vec4(0.1f, 0.05f, 0.05f, 1.0f),
        glm::vec4(0.6f, 0.05f, 0.05f, 1.0f),
        glm::vec4(0.05f, 0.02f, 0.02f, 1.0f)
    );
    m_eyebrowsGeo.setData(browData.positions, browData.colors, browData.primitiveType);
    m_eyebrowsGeo.upload();

    // Bocca a ghigno malefico con zanna aguzza.
    MeshData mouthData = ShapeGenerator::createTriangle(
        glm::vec4(0.10f, 0.02f, 0.02f, 1.0f),
        glm::vec4(0.10f, 0.02f, 0.02f, 1.0f),
        glm::vec4(0.95f, 0.95f, 0.95f, 1.0f)
    );
    m_evilMouthGeo.setData(mouthData.positions, mouthData.colors, mouthData.primitiveType);
    m_evilMouthGeo.upload();

    // ------------------------------------------------------------------------
    // 3. Caricamento del Warp Gate (Portale Monumentale Hermite)
    // ------------------------------------------------------------------------
    MeshData gateData;
    if (PlayerShapeLoader::load(warpGateObjPath, gateData, err))
    {
        m_gateGeo.setData(gateData.positions, gateData.colors, gateData.primitiveType);
        m_gateGeo.upload();
        m_hasGate = true;
    }
    else
    {
        std::cerr << "[ERRORE] Caricamento modello Hermite Gate (" << warpGateObjPath << "): " << err << "\n";
    }

    // Vortice di energia interno (cerchio ciano a sfumatura radiale).
    MeshData vortexData = ShapeGenerator::createCircle(
        0.0f, 0.0f, 30.0f, 30.0f, 32,
        glm::vec4(0.8f, 0.95f, 1.0f, 0.90f),
        glm::vec4(0.1f, 0.45f, 0.85f, 0.30f)
    );
    m_vortexGeo.setData(vortexData.positions, vortexData.colors, vortexData.primitiveType);
    m_vortexGeo.upload();

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
        m_weakenedTimer -= dt;
        speedMultiplier *= 0.35f; // Rallentamento drastico del 65%: moto faticoso e vulnerabile
    }

    float omega = 0.85f * speedMultiplier;
    m_orbitPhase += omega * dt;

    // 1. Traiettoria orbitale armonica a figura di 8 (Curva di Lissajous con rapporto 1:2):
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
            m_dashDirection = glm::vec2(std::cos(randAngle), std::sin(randAngle));

            // Prossimo scatto tra 4.5 e 8.0 secondi (più frequente nei livelli alti):
            float minInterval = std::max(2.8f, 5.0f - (m_level - 1) * 0.4f);
            m_dashCooldown = minInterval + static_cast<float>(rand() % 30) * 0.1f;
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
        m_dashTimer -= dt;
        float progress = 1.0f - (m_dashTimer / 0.75f); // 0.0 all'inizio, 1.0 alla fine

        // Profilo a campana sinusoidale (starts at 0, peaks at 1, returns to 0):
        float bell = std::sin(progress * 3.14159f);

        // Giro strano: slancio direzionale combinato con un loop a vortice ad alta frequenza
        float loopRadius = 60.0f * bell;
        float loopAngle = progress * 6.28318f * 2.5f; // 2.5 rotazioni complete durante lo scatto
        m_dashOffset = m_dashDirection * (bell * 110.0f) + glm::vec2(std::cos(loopAngle) * loopRadius, std::sin(loopAngle) * loopRadius);

        // Rollio della testa del drone durante la manovra
        m_droneAngle = std::sin(progress * 6.28318f * 2.0f) * 45.0f;
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
            m_droneAngle = 0.0f;
        }
    }

    // Posizione effettiva del drone: orbita base + eventuale deviazione dello scatto
    m_dronePos = baseOrbit + m_dashOffset;

    // Fluttuazione armonica del portale cosmico in alto a destra.
    m_gatePos.x = 1080.0f + 25.0f * std::cos(currentTime * 0.5f);
    m_gatePos.y = 560.0f + 20.0f * std::sin(currentTime * 0.7f);

    // ------------------------------------------------------------------------
    // Gestione dell'Interazione Didattica: Sbattimento dell'Occhio (Eye-Blink)
    // "Quando ci passo sopra sbatte l'occhio UNA volta!"
    // ------------------------------------------------------------------------
    float dist = glm::distance(playerPos, m_dronePos);
    bool isOver = (dist < 65.0f);

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
    // L'anello esterno ruota a 22 gradi/secondo, il vortice interno ruota vorticosamente a 100 gradi/secondo.
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
            eyeScaleY = std::abs(std::sin(progress * 3.14159f));
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
        glm::mat4 pupilMat = glm::scale(eyeMat, glm::vec3(1.0f, 0.9f, 1.0f));
        shader.setMat4("model", pupilMat);
        m_pupilGeo.draw();

        // E) Sopracciglio Destro inclinato di +28 gradi.
        glm::mat4 browRight = glm::translate(headRoot, glm::vec3(10.0f, 15.0f, 0.0f));
        browRight = glm::rotate(browRight, glm::radians(28.0f), glm::vec3(0, 0, 1));
        browRight = glm::scale(browRight, glm::vec3(18.0f, 5.0f, 1.0f));
        shader.setMat4("model", browRight);
        m_eyebrowsGeo.draw();

        // F) Sopracciglio Sinistro inclinato di -28 gradi.
        glm::mat4 browLeft = glm::translate(headRoot, glm::vec3(-10.0f, 15.0f, 0.0f));
        browLeft = glm::rotate(browLeft, glm::radians(-28.0f), glm::vec3(0, 0, 1));
        browLeft = glm::scale(browLeft, glm::vec3(-18.0f, 5.0f, 1.0f));
        shader.setMat4("model", browLeft);
        m_eyebrowsGeo.draw();

        // G) Bocca a ghigno malefico posizionata nella parte inferiore del volto.
        glm::mat4 mouthMat = glm::translate(headRoot, glm::vec3(0.0f, -18.0f, 0.0f));
        mouthMat = glm::scale(mouthMat, glm::vec3(22.0f, 8.0f, 1.0f));
        shader.setMat4("model", mouthMat);
        m_evilMouthGeo.draw();

        // Ripristina uColorTint predefinito (1.0f) per le geometrie successive:
        shader.setVec4("uColorTint", glm::vec4(1.0f));
    }

    // ------------------------------------------------------------------------
    // 2. Disegno del Cosmic Warp Gate (Portale Monumentale)
    // ------------------------------------------------------------------------
    if (m_hasGate)
    {
        glm::mat4 gateRoot = glm::mat4(1.0f);
        gateRoot = glm::translate(gateRoot, glm::vec3(m_gatePos, 0.0f));

        bool activeVisual = (gateActive || m_relicPulseTimer > 0.0f);
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
            glm::mat4 vortexMat = glm::rotate(gateRoot, glm::radians(m_vortexRotation * 1.5f), glm::vec3(0, 0, 1));
            vortexMat = glm::scale(vortexMat, glm::vec3(1.4f * pulse, 1.4f * pulse, 1.0f));
            shader.setVec4("uColorTint", glm::vec4(0.3f, 1.0f, 0.95f, 1.0f));
            shader.setMat4("model", vortexMat);
            m_vortexGeo.draw();

            // Anello monumentale Hermite.
            glm::mat4 ringMat = glm::rotate(gateRoot, glm::radians(m_gateRotation * 1.5f), glm::vec3(0, 0, 1));
            ringMat = glm::scale(ringMat, glm::vec3(1.35f * pulse, 1.35f * pulse, 1.0f));
            shader.setVec4("uColorTint", glm::vec4(0.4f, 0.85f, 1.0f, 1.0f));
            shader.setMat4("model", ringMat);
            m_gateGeo.draw();
        }
        else
        {
            // Portale Dormiente (in attesa che il giocatore raccolga le 5 monete).
            glm::mat4 vortexMat = glm::rotate(gateRoot, glm::radians(m_vortexRotation * 0.4f), glm::vec3(0, 0, 1));
            vortexMat = glm::scale(vortexMat, glm::vec3(1.15f, 1.15f, 1.0f));
            shader.setVec4("uColorTint", glm::vec4(0.2f, 0.4f, 0.8f, 0.75f));
            shader.setMat4("model", vortexMat);
            m_vortexGeo.draw();

            glm::mat4 ringMat = glm::rotate(gateRoot, glm::radians(m_gateRotation * 0.5f), glm::vec3(0, 0, 1));
            ringMat = glm::scale(ringMat, glm::vec3(1.15f, 1.15f, 1.0f));
            shader.setVec4("uColorTint", glm::vec4(0.35f, 0.6f, 0.95f, 0.9f));
            shader.setMat4("model", ringMat);
            m_gateGeo.draw();
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

    // Raggio di attivazione del trigger circolare: 58 pixel.
    float distSq = glm::dot(playerCol.center - m_gatePos, playerCol.center - m_gatePos);
    float triggerRad = 58.0f;
    return distSq <= (triggerRad * triggerRad);
}

// ============================================================================
// Rilevamento Collisione con il Drone Sentinella
// ============================================================================
bool HermiteManager::checkDroneCollision(const CircleCollider& playerCol) const
{
    if (!m_hasDrone) return false;
    float dist = glm::distance(playerCol.center, m_dronePos);
    float droneRadius = (m_weakenedTimer > 0.0f) ? 28.0f : 36.0f;
    return dist <= (playerCol.radius + droneRadius);
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

    // Calcola la distanza cartesiana tra il dardo laser e il centro del Drone Rosso
    float dist = glm::distance(bulletPos, m_dronePos);
    float hitRadius = (m_weakenedTimer > 0.0f) ? 30.0f : 36.0f;

    if (dist <= (hitRadius + bulletRadius))
    {
        // 1. Applica o estende lo stato di indebolimento (4.5 secondi di durata)
        m_weakenedTimer = 4.5f;
        m_hitFlashTimer = 0.18f;
        m_hitCount++;

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
