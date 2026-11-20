// ============================================================================
// File: include/Player.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Definizione della classe Player (Starfighter minerario avanzato).
//              Implementa la modellazione gerarchica di un oggetto composto
//              multilivello (Requisito 3.1): fusoliera, ali speculari, cannoni,
//              cabina con texture personalizzata del pilota, post-bruciatori e luci stroboscopiche.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "Geometry.h"
#include "Shader.h"
#include "Collider2D.h"
#include "Transform.h"
#include "Texture.h"

struct GLFWwindow;

class Player
{
public:
    // Costruttore: inizializza lo stato interno e i vettori a valori di default.
    Player();

    // Distruttore RAII: dealloca VAO e VBO associati al quad della texture del pilota.
    ~Player();

    // Inizializza tutte le mesh geometriche dei componenti, carica texture e shader dedicati.
    void init();

    // Ripristina la posizione iniziale e le vite della navicella dopo un game-over.
    // NOTA: il punteggio totale NON viene azzerato (rimane cumulativo durante l'intera sessione).
    void reset(const glm::vec2& startPos);

    // Aggiorna cinematica, orientamento angolare, timer di invulnerabilità e input (WASD e Frecce Direzionali).
    void update(float dt, GLFWwindow* window, int screenWidth, int screenHeight);

    // Renderizza la navicella assemblando gerarchicamente tutti i sotto-componenti mediante matrici Model.
    void draw(const Shader& shader, float currentTime, const glm::mat4& projection);

    // Ricarica a caldo (hot-reload) la texture personalizzata del viso del pilota da percorso specificato.
    bool reloadFaceTexture(const std::string& path);

    // Restituisce il volume di collisione circolare centrato sulla fusoliera (raggio 28px).
    CircleCollider getCollider() const;

    // Infligge 1 punto danno scalando le vite e attivando il timer di invulnerabilità temporanea.
    void takeDamage();

    // Restituisce true se la navicella ha ancora vite residue (> 0).
    bool isAlive() const { return m_lives > 0; }

    // Restituisce il numero di vite correnti.
    int lives() const { return m_lives; }

    // Restituisce true se la navicella è nella finestra temporale di invulnerabilità post-danno.
    bool isInvulnerable() const { return m_invulnerableTimer > 0.0f; }

    // Incrementa il punteggio cumulativo di sessione.
    void addScore(int points) { m_score += points; }

    // Restituisce il punteggio totale accumulato.
    int score() const { return m_score; }

    // Coordinate cartesiane 2D correnti della navicella nel mondo.
    const glm::vec2& position() const { return m_pos; }

    // Angolo di beccheggio/rotta corrente in gradi rispetto all'asse X.
    float angle() const { return m_angle; }

    // Versore di direzione verso cui punta la navetta.
    glm::vec2 direction() const {
        float r = glm::radians(m_angle);
        return glm::vec2(std::cos(r), std::sin(r));
    }

    // Intensità istantanea di spinta dei motori [0.0..1.0].
    float thrustIntensity() const { return m_thrustIntensity; }

    // Velocità lineare di traslazione in pixel al secondo.
    float speed() const { return m_speed; }

    // Modifica dinamica della velocità lineare (tramite slider ImGui o shortcut).
    void setSpeed(float s) { m_speed = s; }

    // Verifica se il cooldown del cannone laser è esaurito e la navicella può sparare.
    bool canFire() const { return m_fireCooldown <= 0.0f; }

    // Reimposta il cooldown di sparo al valore specificato (default 0.20s).
    void resetFireCooldown(float cd = 0.20f) { m_fireCooldown = cd; }

private:
    // Posizione 2D nel sistema di coordinate del mondo (in pixel).
    glm::vec2 m_pos{ 640.0f, 360.0f };
    // Vettore velocità istantanea.
    glm::vec2 m_velocity{ 0.0f, 0.0f };
    // Angolo di direzione della navicella (in gradi sessagesimali).
    float m_angle{ 0.0f };
    // Modulo della velocità massima di crociera.
    float m_speed{ 360.0f };
    // Intensità della spinta dei motori per l'animazione della fiamma [0.0, 1.0].
    float m_thrustIntensity{ 0.0f };
    // Timer di ricarica tra colpi laser consecutivi.
    float m_fireCooldown{ 0.0f };

    // Contatore delle vite della navicella (inizia a 3).
    int m_lives{ 3 };
    // Punteggio accumulato (permanente tra i restart di livello).
    int m_score{ 0 };
    // Timer rimanente per l'invulnerabilità dopo aver subito un impatto.
    float m_invulnerableTimer{ 0.0f };

    // Texture 2D e risorse OpenGL per il viso del pilota (applicata SOPRA la cabina).
    Texture m_faceTexture;
    bool m_hasFaceTexture = false;
    std::unique_ptr<Shader> m_textureShader;
    GLuint m_faceVAO = 0;
    GLuint m_faceVBO = 0;

    // Componenti geometrici che costituiscono l'oggetto composto multilivello:
    Geometry m_fuselageGeo;       // Fusoliera centrale trapezoidale
    Geometry m_noseConeGeo;       // Muso aerodinamico triangolare
    Geometry m_canopyGeo;         // Cupolino in cristallo ciano trasparente
    Geometry m_canopyShineGeo;    // Bagliore / riflesso geometrico 2D del vetro del cupolino
    Geometry m_wingsGeo;          // Ali a delta simmetriche
    Geometry m_cannonsGeo;        // Cannoni plasma montati sulle ali
    Geometry m_thrusterNozzleGeo; // Ugelli di scarico del motore
    Geometry m_flameCoreGeo;      // Nucleo interno caldo della fiamma di propulsione
    Geometry m_flameHaloGeo;      // Alone esterno espanso della fiamma
    Geometry m_beaconGreenGeo;    // Luce di navigazione verde a dritta
    Geometry m_beaconRedGeo;      // Luce di navigazione rossa a sinistra
};
