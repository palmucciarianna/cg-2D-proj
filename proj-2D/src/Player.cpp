// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: Player.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Realizza la navicella composta a 11 nodi gerarchici guidata dal pilota (Requisito 3.1).
//    - Integra la cinematica fisica Euleriana continua (accelerazione, trascinamento inerziale).
//    - Gestisce la texture 2D della foto del pilota con ritaglio ellittico.
// 2. A CHI È COLLEGATO:
//    - Coordinato da GameWorld; riceve input continui da GLFW (frecce/WASD).
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - Gerarchia matriciale M_world = M_player * (T_loc * R_loc * S_loc) (Domanda 6).
//    - Riflessione speculare alare con scala negativa S(-1, 1, 1) (Domanda 6).
//    - Integrazione Euleriana: v(t+dt) = v(t) + a*dt; p(t+dt) = p(t) + v(t)*dt (Requisito 6).
//    - Fiamme propulsore reattive alla velocita' e texture mapping UV cabina (Domande 14, 20).
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
    m_fuselageGeo.uploadMesh(ShapeGenerator::createTrapezoid(24.0f, 38.0f, 48.0f,
        glm::vec4(0.18f, 0.45f, 0.72f, 1.0f),  // Colore bordo anteriore
        glm::vec4(0.08f, 0.16f, 0.32f, 1.0f))); // Colore bordo posteriore

    // Muso anteriore affilato (triangolo con punta ciano brillante).
    m_noseConeGeo.uploadMesh(ShapeGenerator::createTriangle(
        glm::vec4(0.20f, 0.75f, 0.95f, 1.0f),  // Vertice 1: punta ciano
        glm::vec4(0.12f, 0.30f, 0.55f, 1.0f),  // Vertice 2: base blu
        glm::vec4(0.12f, 0.30f, 0.55f, 1.0f))); // Vertice 3: base blu

    // Tettuccio cabina in cristallo (ellisse con bordo turchese).
    m_canopyGeo.uploadMesh(ShapeGenerator::createCircle(0.0f, 0.0f, 9.0f, 16.0f, 32,
        glm::vec4(0.85f, 0.98f, 1.0f, 0.95f),  // Centro vetro luminoso
        glm::vec4(0.10f, 0.55f, 0.85f, 0.85f))); // Bordo turchese semi-trasparente

    // Riflesso geometrico 2D del vetro sul cupolino (ellisse bianca semi-trasparente).
    m_canopyShineGeo.uploadMesh(ShapeGenerator::createCircle(2.0f, 3.0f, 3.5f, 7.0f, 16,
        glm::vec4(1.0f, 1.0f, 1.0f, 0.85f), // Centro riflesso bianco
        glm::vec4(0.9f, 0.95f, 1.0f, 0.0f))); // Bordo sfumato a alpha 0

    // Ala a freccia con pannellatura (riutilizzata sia per SX che per DX).
    m_wingsGeo.uploadMesh(ShapeGenerator::createTriangle(
        glm::vec4(0.12f, 0.35f, 0.65f, 1.0f),  // Vertice interno blu
        glm::vec4(0.25f, 0.65f, 0.90f, 1.0f),  // Vertice esterno azzurro
        glm::vec4(0.06f, 0.15f, 0.30f, 1.0f))); // Vertice posteriore scuro

    // Cannoni laser sulle estremità alari.
    m_cannonsGeo.uploadMesh(ShapeGenerator::createRectangle(5.0f, 26.0f,
        glm::vec4(0.20f, 0.90f, 1.0f, 1.0f),   // Punta cannone ciano
        glm::vec4(0.15f, 0.18f, 0.22f, 1.0f))); // Base cannone metallica

    // Ugelli propulsori posteriori in titanio scuro.
    m_thrusterNozzleGeo.uploadMesh(ShapeGenerator::createRectangle(8.0f, 10.0f,
        glm::vec4(0.25f, 0.28f, 0.35f, 1.0f),  // Parte superiore grigia
        glm::vec4(0.08f, 0.08f, 0.12f, 1.0f))); // Bordo inferiore scuro

    // Nucleo fiamma di propulsione (bianco-azzurro termico).
    m_flameCoreGeo.uploadMesh(ShapeGenerator::createTriangle(
        glm::vec4(0.70f, 0.95f, 1.0f, 1.0f),  // Base calda bianco-azzurra
        glm::vec4(0.20f, 0.60f, 1.0f, 0.9f),  // Lato azzurro
        glm::vec4(0.10f, 0.30f, 1.0f, 0.0f))); // Punta sfumata trasparente

    // Alone esterno della fiamma (arancio-fuoco).
    m_flameHaloGeo.uploadMesh(ShapeGenerator::createTriangle(
        glm::vec4(1.0f, 0.85f, 0.3f, 0.95f),  // Base giallo acceso
        glm::vec4(1.0f, 0.35f, 0.05f, 0.80f), // Lato arancio vivo
        glm::vec4(0.8f, 0.05f, 0.00f, 0.00f))); // Punta rossa trasparente

    // Luci stroboscopiche di navigazione: verde a destra e rossa a sinistra.
    m_beaconGreenGeo.uploadMesh(ShapeGenerator::createCircle(0.0f, 0.0f, 3.5f, 3.5f, 12,
        glm::vec4(0.3f, 1.0f, 0.4f, 1.0f),  // Centro verde brillante
        glm::vec4(0.1f, 0.8f, 0.2f, 0.2f))); // Bordo verde tenue

    m_beaconRedGeo.uploadMesh(ShapeGenerator::createCircle(0.0f, 0.0f, 3.5f, 3.5f, 12,
        glm::vec4(1.0f, 0.2f, 0.3f, 1.0f),  // Centro rosso brillante
        glm::vec4(0.8f, 0.1f, 0.1f, 0.2f))); // Bordo rosso tenue
}

// ============================================================================
// Reset dello Stato di Gioco della Navicella
// ============================================================================
void Player::reset(const glm::vec2& startPos)
{
    m_pos = startPos;              // Posiziona la navicella al punto iniziale
    m_velocity = glm::vec2(0.0f);  // Azzera il vettore velocità residuo
    m_angle = 0.0f;                // Orienta la prua a 0 gradi (verso destra)
    m_thrustIntensity = 0.0f;      // Spegne la spinta dei propulsori
    m_fireCooldown = 0.0f;         // Rende il cannone immediatamente pronto a sparare
    m_lives = 3;                   // Ripristina le 3 vite massime
    // IMPORTANTE: m_score NON viene azzerato, garantendo la cumulatività del punteggio
    // durante l'intera durata dell'applicazione fino alla chiusura della finestra.
    m_invulnerableTimer = 0.0f;    // Disattiva l'invulnerabilità post-danno
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
// Helper: Tinta Stroboscopica di Invulnerabilità
// ============================================================================
glm::vec4 Player::computeDamageTint(float currentTime) const
{
    if (m_invulnerableTimer > 0.0f && std::sin(currentTime * 28.0f) < 0.0f)
    {
        return glm::vec4(1.0f, 0.35f, 0.35f, 0.85f); // Rosso semitrasparente durante la semionda negativa
    }
    return glm::vec4(1.0f); // Bianco opaco neutro negli altri casi
}

// ============================================================================
// Rendering Gerarchico dell'Oggetto Composto Navicella
// ============================================================================
void Player::draw(const Shader& shader, float currentTime, const glm::mat4& projection)
{
    // Feedback visivo di invulnerabilità calcolato una volta tramite helper condiviso.
    const glm::vec4 damageTint = computeDamageTint(currentTime);
    shader.setVec4("uColorTint", damageTint);

    // ------------------------------------------------------------------------
    // Matrice Radice della Navicella (Root Transformation)
    // ------------------------------------------------------------------------
    // Trasla all'attuale posizione del mondo ed applica la rotazione d'orientamento.
    // L'offset -90° compensa l'asse predefinito Y delle coordinate locali della mesh.
    // [Domanda 8: Cambiamento di sistema di riferimento (frame locale -> mondo)]
    glm::mat4 playerRoot = glm::mat4(1.0f);
    playerRoot = glm::translate(playerRoot, glm::vec3(m_pos, 0.0f));
    playerRoot = glm::rotate(playerRoot, glm::radians(m_angle - 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // ------------------------------------------------------------------------
    // 1. Ali a Freccia Gerarchiche (Simmetria tramite Scala Negativa)
    // ------------------------------------------------------------------------
    // [Domanda 6: Trasformazioni affini - riflessione speculare S(-1, 1, 1) senza duplicare vertici]
    // Ala Destra (+1) e Ala Sinistra (-1 specchiata lungo X locale con scala negativa).
    for (float side : { 1.0f, -1.0f })
    {
        glm::mat4 wingMat = glm::translate(playerRoot, glm::vec3(18.0f * side, -6.0f, 0.0f));
        wingMat = glm::scale(wingMat, glm::vec3(26.0f * side, 32.0f, 1.0f));
        shader.setMat4("model", wingMat); // Invia la matrice gerarchica allo shader
        m_wingsGeo.draw();                // Riusa lo stesso VAO dell'ala senza duplicare vertici
    }

    // ------------------------------------------------------------------------
    // 2. Doppi Cannoni Laser sulle Punte Alari
    // ------------------------------------------------------------------------
    for (float side : { 1.0f, -1.0f })
    {
        glm::mat4 cannonMat = glm::translate(playerRoot, glm::vec3(31.0f * side, 4.0f, 0.0f)); // Offset alare DX/SX
        shader.setMat4("model", cannonMat); // Imposta la model matrix
        m_cannonsGeo.draw();                // Disegna il cannone
    }

    // ------------------------------------------------------------------------
    // 3. Fusoliera Centrale Rastremata
    // ------------------------------------------------------------------------
    glm::mat4 fuseMat = glm::translate(playerRoot, glm::vec3(0.0f, -4.0f, 0.0f)); // Centra la fusoliera rispetto al root
    shader.setMat4("model", fuseMat); // Invia la model matrix allo shader
    m_fuselageGeo.draw();             // Disegna il trapezio della fusoliera

    // ------------------------------------------------------------------------
    // 4. Muso Anteriore Affilato
    // ------------------------------------------------------------------------
    glm::mat4 noseMat = glm::translate(playerRoot, glm::vec3(0.0f, 30.0f, 0.0f)); // Posiziona in punta (+30px su Y locale)
    noseMat = glm::scale(noseMat, glm::vec3(24.0f, 22.0f, 1.0f));                 // Dimensiona il triangolo di prua
    shader.setMat4("model", noseMat); // Invia la matrice composta allo shader
    m_noseConeGeo.draw();             // Disegna il muso della navicella

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

        // Applica il medesimo effetto di lampeggio condiviso se invulnerabile.
        m_textureShader->setVec4("uColorTint", damageTint);

        // Lega la texture alla slot 0.
        m_faceTexture.bind(0);
        m_textureShader->setInt("uTexture", 0);

        // Emette la draw call per il quad rettangolare (6 vertici).
        glBindVertexArray(m_faceVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        m_faceTexture.unbind(); // stacca VAO e texture per evitare side effect su altri oggetti

        // Riattiva lo shader generale dei colori per i rimanenti componenti geometrici.
        shader.use();
    }

    // Disegna il riflesso geometrico in vetro sul cupolino (sopra la foto).
    shader.setMat4("model", canopyMat);
    m_canopyShineGeo.draw();

    // ------------------------------------------------------------------------
    // 6. Ugelli Reattori Posteriori
    // ------------------------------------------------------------------------
    for (float side : { 1.0f, -1.0f })
    {
        glm::mat4 nozzleMat = glm::translate(playerRoot, glm::vec3(10.0f * side, -28.0f, 0.0f)); // Posizione ugello DX/SX
        shader.setMat4("model", nozzleMat); // Invia matrice allo shader
        m_thrusterNozzleGeo.draw();         // Disegna ugello
    }

    // ------------------------------------------------------------------------
    // 7. Fiamme di Propulsione Animate (Doppio Post-Bruciatore)
    // ------------------------------------------------------------------------
    if (m_thrustIntensity > 0.02f) // Disegna le fiamme solo se c'è spinta attiva
    {
        // Lunghezza pulsante modulata da funzione armonica sinusoidale ad alta frequenza (35 rad/s).
        float flameLength = (24.0f + 14.0f * std::sin(currentTime * 35.0f)) * m_thrustIntensity;
        float flameWidth  = 12.0f * m_thrustIntensity; // Larghezza proporzionale alla spinta

        // Disegna entrambi i propulsori (Destro +1, Sinistro -1) con gerarchia a 3 livelli (root -> fiamma -> nucleo).
        for (float side : { 1.0f, -1.0f })
        {
            glm::mat4 flameMat = glm::translate(playerRoot, glm::vec3(10.0f * side, -32.0f, 0.0f)); // Dietro ugello
            flameMat = glm::rotate(flameMat, glm::radians(180.0f), glm::vec3(0, 0, 1));             // Punta indietro (-Y)
            flameMat = glm::scale(flameMat, glm::vec3(flameWidth, flameLength, 1.0f));              // Scala pulsante
            shader.setMat4("model", flameMat); // Invia matrice alone esterno
            m_flameHaloGeo.draw();             // Disegna alone arancione

            glm::mat4 coreMat = glm::scale(flameMat, glm::vec3(0.6f, 0.7f, 1.0f)); // Nucleo interno più stretto e corto
            shader.setMat4("model", coreMat); // Invia matrice del nucleo
            m_flameCoreGeo.draw();            // Disegna nucleo bianco-azzurro
        }
    }

    // ------------------------------------------------------------------------
    // 8. Luci di Navigazione Alari Stroboscopiche
    // ------------------------------------------------------------------------
    float strobe = std::sin(currentTime * 8.0f); // Onda sinusoidale per lampeggio a 8 rad/s
    if (strobe > 0.0f) // Accende i LED solo nella semionda positiva
    {
        glm::mat4 beaconRight = glm::translate(playerRoot, glm::vec3(31.0f, -12.0f, 0.0f)); // Punta alare destra
        shader.setMat4("model", beaconRight); // Invia matrice LED destro
        m_beaconGreenGeo.draw();              // Disegna luce di posizione verde

        glm::mat4 beaconLeft = glm::translate(playerRoot, glm::vec3(-31.0f, -12.0f, 0.0f)); // Punta alare sinistra
        shader.setMat4("model", beaconLeft); // Invia matrice LED sinistro
        m_beaconRedGeo.draw();               // Disegna luce di posizione rossa
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
        m_lives--;                  // Decrementa il contatore vite
        m_invulnerableTimer = 1.6f; // Attiva 1.6 secondi di immunità temporanea
    }
}

// ============================================================================
// Hot-Reload Texture Viso
// ============================================================================
bool Player::reloadFaceTexture(const std::string& path)
{
    if (m_faceTexture.load(path)) // Tenta il ricaricamento da disco (deallocando la texture precedente)
    {
        m_hasFaceTexture = true;  // Conferma la disponibilità della texture
        return true;              // Ricaricamento riuscito
    }
    return false;                 // File non trovato o illeggibile al percorso indicato
}
