// ============================================================================
// File: src/Player.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione dettagliata della navicella mineraria Player.
//              Mostra la modellazione gerarchica tramite matrici di trasformazione,
//              il mapping di texture sulla cabina, l'integrazione del moto Euleriano,
//              la riflessione speculare delle ali via S(-1, 1, 1), l'animazione
//              reattiva delle fiamme e il controllo simultaneo WASD / Frecce direzionali.
// ============================================================================

#include "Player.h"
#include "ShapeGenerator.h"
#include "ShaderMaker.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

// ============================================================================
// Costruttore
// ============================================================================
Player::Player()
{
}

// ============================================================================
// Distruttore RAII
// ============================================================================
Player::~Player()
{
    // Dealloca i buffer GPU del quad della texture della cabina.
    if (m_faceVAO != 0)
    {
        glDeleteVertexArrays(1, &m_faceVAO);
        m_faceVAO = 0;
    }
    if (m_faceVBO != 0)
    {
        glDeleteBuffers(1, &m_faceVBO);
        m_faceVBO = 0;
    }
}

// ============================================================================
// Inizializzazione Geometrie e Risorse Hardware del Player
// ============================================================================
void Player::init()
{
    // ------------------------------------------------------------------------
    // 1. Caricamento della Texture del Viso del Pilota
    // ------------------------------------------------------------------------
    // Cerca il file immagine del pilota verificando diversi percorsi e formati.
    if (m_faceTexture.load("assets/textures/player_face.bmp") ||
        m_faceTexture.load("assets/textures/player_face.png") ||
        m_faceTexture.load("assets/textures/player_face.jpg") ||
        m_faceTexture.load("build/assets/textures/player_face.bmp"))
    {
        m_hasFaceTexture = true;
    }

    // Carica lo shader con supporto texture mapping per il quad del viso.
    m_textureShader = std::make_unique<Shader>();
    if (!m_textureShader->load("assets/shaders/texture.vert", "assets/shaders/texture.frag"))
    {
        std::cerr << "[Player] Avviso: Shader texture fallback non caricato." << std::endl;
    }

    // ------------------------------------------------------------------------
    // 2. Creazione del Quad Geometrico per la Cabina (Texture Coordinates)
    // ------------------------------------------------------------------------
    // Vertici con coordinate spaziali (x, y, z) e coordinate di texture (u, v):
    float quadVertices[] = {
        // x,     y,    z,     u,    v
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,

        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f
    };

    // Genera e lega VAO e VBO per il quad di texture.
    glGenVertexArrays(1, &m_faceVAO);
    glGenBuffers(1, &m_faceVBO);

    glBindVertexArray(m_faceVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_faceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Attributo 0: Coordinate di posizione spaziale vec3 (x, y, z), stride 5 float.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Attributo 1: Coordinate di mappatura texture vec2 (u, v), offset 3 float.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    // ------------------------------------------------------------------------
    // 3. Creazione e Upload dei Singoli Componenti Geometrici
    // ------------------------------------------------------------------------

    // Fusoliera centrale (trapezio metallico con sfumatura di profondità).
    MeshData fuseData = ShapeGenerator::createTrapezoid(24.0f, 38.0f, 48.0f,
        glm::vec4(0.18f, 0.45f, 0.72f, 1.0f),  // Davanti
        glm::vec4(0.08f, 0.16f, 0.32f, 1.0f)); // Dietro
    m_fuselageGeo.setData(fuseData.positions, fuseData.colors, fuseData.primitiveType);
    m_fuselageGeo.upload();

    // Muso anteriore affilato (triangolo con punta ciano brillante).
    MeshData noseData = ShapeGenerator::createTriangle(
        glm::vec4(0.20f, 0.75f, 0.95f, 1.0f),  // Punta
        glm::vec4(0.12f, 0.30f, 0.55f, 1.0f),
        glm::vec4(0.12f, 0.30f, 0.55f, 1.0f));
    m_noseConeGeo.setData(noseData.positions, noseData.colors, noseData.primitiveType);
    m_noseConeGeo.upload();

    // Tettuccio cabina in cristallo (ellisse con bordo turchese).
    MeshData canopyData = ShapeGenerator::createCircle(0.0f, 0.0f, 9.0f, 16.0f, 32,
        glm::vec4(0.85f, 0.98f, 1.0f, 0.95f),
        glm::vec4(0.10f, 0.55f, 0.85f, 0.85f));
    m_canopyGeo.setData(canopyData.positions, canopyData.colors, canopyData.primitiveType);
    m_canopyGeo.upload();

    // Riflesso geometrico 2D del vetro sul cupolino (ellisse bianca semi-trasparente).
    MeshData shineData = ShapeGenerator::createCircle(2.0f, 3.0f, 3.5f, 7.0f, 16,
        glm::vec4(1.0f, 1.0f, 1.0f, 0.85f),
        glm::vec4(0.9f, 0.95f, 1.0f, 0.0f));
    m_canopyShineGeo.setData(shineData.positions, shineData.colors, shineData.primitiveType);
    m_canopyShineGeo.upload();

    // Ala a freccia con pannellatura.
    MeshData wingData = ShapeGenerator::createTriangle(
        glm::vec4(0.12f, 0.35f, 0.65f, 1.0f),
        glm::vec4(0.25f, 0.65f, 0.90f, 1.0f),
        glm::vec4(0.06f, 0.15f, 0.30f, 1.0f));
    m_wingsGeo.setData(wingData.positions, wingData.colors, wingData.primitiveType);
    m_wingsGeo.upload();

    // Cannoni laser sulle estremità alari.
    MeshData cannonData = ShapeGenerator::createRectangle(5.0f, 26.0f,
        glm::vec4(0.20f, 0.90f, 1.0f, 1.0f),
        glm::vec4(0.15f, 0.18f, 0.22f, 1.0f));
    m_cannonsGeo.setData(cannonData.positions, cannonData.colors, cannonData.primitiveType);
    m_cannonsGeo.upload();

    // Ugelli propulsori posteriori in titanio scuro.
    MeshData nozzleData = ShapeGenerator::createRectangle(8.0f, 10.0f,
        glm::vec4(0.25f, 0.28f, 0.35f, 1.0f),
        glm::vec4(0.08f, 0.08f, 0.12f, 1.0f));
    m_thrusterNozzleGeo.setData(nozzleData.positions, nozzleData.colors, nozzleData.primitiveType);
    m_thrusterNozzleGeo.upload();

    // Nucleo fiamma di propulsione (bianco-azzurro termico).
    MeshData coreFlame = ShapeGenerator::createTriangle(
        glm::vec4(0.70f, 0.95f, 1.0f, 1.0f),
        glm::vec4(0.20f, 0.60f, 1.0f, 0.9f),
        glm::vec4(0.10f, 0.30f, 1.0f, 0.0f));
    m_flameCoreGeo.setData(coreFlame.positions, coreFlame.colors, coreFlame.primitiveType);
    m_flameCoreGeo.upload();

    // Alone esterno della fiamma (arancio-fuoco).
    MeshData haloFlame = ShapeGenerator::createTriangle(
        glm::vec4(1.0f, 0.85f, 0.3f, 0.95f),
        glm::vec4(1.0f, 0.35f, 0.05f, 0.80f),
        glm::vec4(0.8f, 0.05f, 0.00f, 0.00f));
    m_flameHaloGeo.setData(haloFlame.positions, haloFlame.colors, haloFlame.primitiveType);
    m_flameHaloGeo.upload();

    // Luci stroboscopiche di navigazione: verde a destra e rossa a sinistra.
    MeshData beaconGreen = ShapeGenerator::createCircle(0.0f, 0.0f, 3.5f, 3.5f, 12,
        glm::vec4(0.3f, 1.0f, 0.4f, 1.0f),
        glm::vec4(0.1f, 0.8f, 0.2f, 0.2f));
    m_beaconGreenGeo.setData(beaconGreen.positions, beaconGreen.colors, beaconGreen.primitiveType);
    m_beaconGreenGeo.upload();

    MeshData beaconRed = ShapeGenerator::createCircle(0.0f, 0.0f, 3.5f, 3.5f, 12,
        glm::vec4(1.0f, 0.2f, 0.3f, 1.0f),
        glm::vec4(0.8f, 0.1f, 0.1f, 0.2f));
    m_beaconRedGeo.setData(beaconRed.positions, beaconRed.colors, beaconRed.primitiveType);
    m_beaconRedGeo.upload();
}

// ============================================================================
// Reset dello Stato di Gioco della Navicella
// ============================================================================
void Player::reset(const glm::vec2& startPos)
{
    // Ripristina coordinate e cinematica.
    m_pos = startPos;
    m_velocity = glm::vec2(0.0f);
    m_angle = 0.0f;
    m_thrustIntensity = 0.0f;
    m_fireCooldown = 0.0f;
    m_lives = 3;
    // IMPORTANTE: m_score NON viene azzerato, garantendo la cumulatività del punteggio
    // durante l'intera durata dell'applicazione fino alla chiusura della finestra.
    m_invulnerableTimer = 0.0f;
}

// ============================================================================
// Aggiornamento Cinematica ed Input da Tastiera (WASD + Frecce)
// ============================================================================
void Player::update(float dt, GLFWwindow* window, int screenWidth, int screenHeight)
{
    // Decrementa i timer temporizzati con il delta time.
    if (m_fireCooldown > 0.0f) m_fireCooldown -= dt;
    if (m_invulnerableTimer > 0.0f) m_invulnerableTimer -= dt;

    // Rileva la pressione dei tasti direzionali: supporta sia WASD che le Frecce Direzionali.
    glm::vec2 inputDir(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    inputDir.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  inputDir.y -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  inputDir.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) inputDir.x += 1.0f;

    bool isMoving = (glm::length(inputDir) > 0.01f);
    if (isMoving)
    {
        // Normalizza il vettore per evitare velocità diagonale maggiorata (sqrt(2) = 1.414).
        inputDir = glm::normalize(inputDir);

        // Integrazione numerica di Eulero: posizione = posizione + velocita * delta_t.
        m_pos += inputDir * m_speed * dt;

        // Calcola l'angolo di orientamento cartesiano in gradi: theta = atan2(dy, dx).
        m_angle = glm::degrees(std::atan2(inputDir.y, inputDir.x));

        // Incrementa la spinta dei propulsori con rampa graduale fino a 1.0.
        m_thrustIntensity = std::min(m_thrustIntensity + dt * 8.0f, 1.0f);
    }
    else
    {
        // Dissolve gradualmente la spinta del motore all'arresto.
        m_thrustIntensity = std::max(m_thrustIntensity - dt * 5.0f, 0.0f);
    }

    // Vincola la posizione all'interno dei confini visibili dello schermo (Clamping).
    m_pos.x = glm::clamp(m_pos.x, 38.0f, static_cast<float>(screenWidth) - 38.0f);
    m_pos.y = glm::clamp(m_pos.y, 38.0f, static_cast<float>(screenHeight) - 38.0f);
}

// ============================================================================
// Rendering Gerarchico dell'Oggetto Composto Navicella
// ============================================================================
void Player::draw(const Shader& shader, float currentTime, const glm::mat4& projection)
{
    // Feedback visivo di invulnerabilità: lampeggio stroboscopico con onda sinusoidale.
    if (m_invulnerableTimer > 0.0f)
    {
        float blink = std::sin(currentTime * 28.0f);
        if (blink < 0.0f)
            shader.setVec4("uColorTint", glm::vec4(1.0f, 0.35f, 0.35f, 0.85f));
        else
            shader.setVec4("uColorTint", glm::vec4(1.0f));
    }
    else
    {
        shader.setVec4("uColorTint", glm::vec4(1.0f));
    }

    // ------------------------------------------------------------------------
    // Matrice Radice della Navicella (Root Transformation)
    // ------------------------------------------------------------------------
    // Trasla all'attuale posizione del mondo ed applica la rotazione d'orientamento.
    // L'offset -90° compensa l'asse predefinito Y delle coordinate locali della mesh.
    glm::mat4 playerRoot = glm::mat4(1.0f);
    playerRoot = glm::translate(playerRoot, glm::vec3(m_pos, 0.0f));
    playerRoot = glm::rotate(playerRoot, glm::radians(m_angle - 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // ------------------------------------------------------------------------
    // 1. Ali a Freccia Gerarchiche (Simmetria tramite Scala Negativa)
    // ------------------------------------------------------------------------
    // Ala Destra:
    glm::mat4 wingRight = glm::translate(playerRoot, glm::vec3(18.0f, -6.0f, 0.0f));
    wingRight = glm::scale(wingRight, glm::vec3(26.0f, 32.0f, 1.0f));
    shader.setMat4("model", wingRight);
    m_wingsGeo.draw();

    // Ala Sinistra: ottenuta specchiando la stessa geometria dell'ala destra lungo l'asse X locale
    // tramite una matrice di riflessione con scala negativa: S(-26.0f, 32.0f, 1.0f).
    glm::mat4 wingLeft = glm::translate(playerRoot, glm::vec3(-18.0f, -6.0f, 0.0f));
    wingLeft = glm::scale(wingLeft, glm::vec3(-26.0f, 32.0f, 1.0f));
    shader.setMat4("model", wingLeft);
    m_wingsGeo.draw();

    // ------------------------------------------------------------------------
    // 2. Doppi Cannoni Laser sulle Punte Alari
    // ------------------------------------------------------------------------
    glm::mat4 cannonRight = glm::translate(playerRoot, glm::vec3(31.0f, 4.0f, 0.0f));
    shader.setMat4("model", cannonRight);
    m_cannonsGeo.draw();

    glm::mat4 cannonLeft = glm::translate(playerRoot, glm::vec3(-31.0f, 4.0f, 0.0f));
    shader.setMat4("model", cannonLeft);
    m_cannonsGeo.draw();

    // ------------------------------------------------------------------------
    // 3. Fusoliera Centrale Rastremata
    // ------------------------------------------------------------------------
    glm::mat4 fuseMat = glm::translate(playerRoot, glm::vec3(0.0f, -4.0f, 0.0f));
    shader.setMat4("model", fuseMat);
    m_fuselageGeo.draw();

    // ------------------------------------------------------------------------
    // 4. Muso Anteriore Affilato
    // ------------------------------------------------------------------------
    glm::mat4 noseMat = glm::translate(playerRoot, glm::vec3(0.0f, 30.0f, 0.0f));
    noseMat = glm::scale(noseMat, glm::vec3(24.0f, 22.0f, 1.0f));
    shader.setMat4("model", noseMat);
    m_noseConeGeo.draw();

    // ------------------------------------------------------------------------
    // 5. Cupolino Cabina & Texture Viso Pilota SOPRA la Cabina
    // ------------------------------------------------------------------------
    glm::mat4 canopyMat = glm::translate(playerRoot, glm::vec3(0.0f, 5.0f, 0.0f));
    shader.setMat4("model", canopyMat);
    m_canopyGeo.draw(); // Disegna prima la base della cabina in cristallo turchese

    // Proietta la texture del volto dell'astronauta SOPRA il cupolino:
    if (m_hasFaceTexture && m_textureShader)
    {
        // Attiva lo shader delle texture.
        m_textureShader->use();
        m_textureShader->setMat4("projection", projection);

        // Calcola la sottomatrice locale per centrare e ridimensionare il quad della foto.
        glm::mat4 faceMat = glm::translate(playerRoot, glm::vec3(0.0f, 5.0f, 0.0f));
        faceMat = glm::scale(faceMat, glm::vec3(8.5f, 15.0f, 1.0f));
        m_textureShader->setMat4("model", faceMat);

        // Applica il medesimo effetto di lampeggio se invulnerabile.
        if (m_invulnerableTimer > 0.0f)
        {
            float blink = std::sin(currentTime * 28.0f);
            m_textureShader->setVec4("uColorTint", (blink < 0.0f) ? glm::vec4(1.0f, 0.35f, 0.35f, 0.85f) : glm::vec4(1.0f));
        }
        else
        {
            m_textureShader->setVec4("uColorTint", glm::vec4(1.0f));
        }

        // Lega la texture alla slot 0.
        m_faceTexture.bind(0);
        m_textureShader->setInt("uTexture", 0);

        // Emette la draw call per il quad rettangolare (6 vertici).
        glBindVertexArray(m_faceVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        m_faceTexture.unbind();

        // Riattiva lo shader generale dei colori per i rimanenti componenti geometrici.
        shader.use();
    }

    // Disegna il riflesso geometrico in vetro sul cupolino (sopra la foto).
    shader.setMat4("model", canopyMat);
    m_canopyShineGeo.draw();

    // ------------------------------------------------------------------------
    // 6. Ugelli Reattori Posteriori
    // ------------------------------------------------------------------------
    glm::mat4 nozzleRight = glm::translate(playerRoot, glm::vec3(10.0f, -28.0f, 0.0f));
    shader.setMat4("model", nozzleRight);
    m_thrusterNozzleGeo.draw();

    glm::mat4 nozzleLeft = glm::translate(playerRoot, glm::vec3(-10.0f, -28.0f, 0.0f));
    shader.setMat4("model", nozzleLeft);
    m_thrusterNozzleGeo.draw();

    // ------------------------------------------------------------------------
    // 7. Fiamme di Propulsione Animate (Doppio Post-Bruciatore)
    // ------------------------------------------------------------------------
    if (m_thrustIntensity > 0.02f)
    {
        // Lunghezza pulsante modulata da funzione armonica sinusoidale ad alta frequenza (35 rad/s).
        float flameLength = (24.0f + 14.0f * std::sin(currentTime * 35.0f)) * m_thrustIntensity;
        float flameWidth  = 12.0f * m_thrustIntensity;

        // Propulsore Destro:
        glm::mat4 flameRight = glm::translate(playerRoot, glm::vec3(10.0f, -32.0f, 0.0f));
        flameRight = glm::rotate(flameRight, glm::radians(180.0f), glm::vec3(0, 0, 1));
        flameRight = glm::scale(flameRight, glm::vec3(flameWidth, flameLength, 1.0f));
        shader.setMat4("model", flameRight);
        m_flameHaloGeo.draw();

        glm::mat4 coreRight = glm::scale(flameRight, glm::vec3(0.6f, 0.7f, 1.0f));
        shader.setMat4("model", coreRight);
        m_flameCoreGeo.draw();

        // Propulsore Sinistro:
        glm::mat4 flameLeft = glm::translate(playerRoot, glm::vec3(-10.0f, -32.0f, 0.0f));
        flameLeft = glm::rotate(flameLeft, glm::radians(180.0f), glm::vec3(0, 0, 1));
        flameLeft = glm::scale(flameLeft, glm::vec3(flameWidth, flameLength, 1.0f));
        shader.setMat4("model", flameLeft);
        m_flameHaloGeo.draw();

        glm::mat4 coreLeft = glm::scale(flameLeft, glm::vec3(0.6f, 0.7f, 1.0f));
        shader.setMat4("model", coreLeft);
        m_flameCoreGeo.draw();
    }

    // ------------------------------------------------------------------------
    // 8. Luci di Navigazione Alari Stroboscopiche
    // ------------------------------------------------------------------------
    float strobe = std::sin(currentTime * 8.0f);
    if (strobe > 0.0f)
    {
        glm::mat4 beaconRight = glm::translate(playerRoot, glm::vec3(31.0f, -12.0f, 0.0f));
        shader.setMat4("model", beaconRight);
        m_beaconGreenGeo.draw();

        glm::mat4 beaconLeft = glm::translate(playerRoot, glm::vec3(-31.0f, -12.0f, 0.0f));
        shader.setMat4("model", beaconLeft);
        m_beaconRedGeo.draw();
    }

    // Ripristina la tinta colore globale a bianco (1.0).
    shader.setVec4("uColorTint", glm::vec4(1.0f));
}

// ============================================================================
// Restituzione Bounding Volume di Collisione
// ============================================================================
CircleCollider Player::getCollider() const
{
    // Bounding circle di raggio 28px centrato sulla fusoliera.
    return CircleCollider{ m_pos, 28.0f };
}

// ============================================================================
// Applicazione Danni e Attivazione Invulnerabilità
// ============================================================================
void Player::takeDamage()
{
    // Subisce danno solo se la finestra di invulnerabilità post-impatto è esaurita.
    if (m_invulnerableTimer <= 0.0f)
    {
        m_lives--;
        m_invulnerableTimer = 1.6f; // 1.6 secondi di immunità temporanea
    }
}

// ============================================================================
// Hot-Reload Texture Viso
// ============================================================================
bool Player::reloadFaceTexture(const std::string& path)
{
    if (m_faceTexture.load(path))
    {
        m_hasFaceTexture = true;
        return true;
    }
    return false;
}
