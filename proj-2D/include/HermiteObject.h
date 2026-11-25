// ORIGINE: In parte basato sul modulo Hermite dei laboratori della prof (spline cubiche), esteso con logica di gioco Alien Sentinel Drone e Warp Gate.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: HermiteManager (Alien Drone & Warp Gate)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Implementa le due geometrie curvilinee avanzate a spline cubiche di Hermite (Requisito 3.2):
//      * 1. Alien Sentinel Drone (Ostacolo Dinamico Ostile): pattuglia lungo una curva armonica
//           a figura di 8 (Lissajous) e sbatte l'occhio (eye-blink) quando il Player ci vola sopra.
//      * 2. Cosmic Warp Gate (Portale Monumentale): anello circolare cavo a curve di Hermite
//           con vortice rotante bifase (dormiente -> attivo alla raccolta delle 5 monete).
//    - Gestisce la meccanica di indebolimento del Drone se colpito dai laser (rallenta e si stordisce).
//
// 2. A CHI È COLLEGATA:
//    - Creata e gestita da: GameWorld (in init(), update(dt, time, playerPos), draw(shader, ...)).
//    - Collabora con:
//      * PlayerShapeLoader: carica e triangola i file Wavefront 'alien_drone.obj' e 'warp_gate.obj'.
//      * ShapeGenerator: crea i dettagli del viso (occhio dorato, pupilla a fessura, sopracciglia, bocca).
//      * Geometry: memorizza i buffer VBO/VAO della testa e delle parti del portale.
//      * Collider2D: fornisce cerchi di collisione per il drone (ostacolo) e per il portale (traguardo).
//      * Shader basic: disegna le mesh applicando tinte cromatiche dinamiche ('uColorTint').
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Spline Cubiche di Hermite & Continuità C^1 (Requisito 3.2 & Domanda 12):
//      Un arco cubico interpola due punti P0, P1 e due derivate tangenti D0, D1:
//      C(t) = h_00(t)*P0 + h_01(t)*P1 + h_10(t)*D0 + h_11(t)*D1,
//      con h_00=2t^3-3t^2+1, h_01=-2t^3+3t^2, h_10=t^3-2t^2+t, h_11=t^3-t^2.
//      I tratti sono raccordati a continuità C^1 condividendo sia la posizione sia la derivata tangente.
//    - Traiettoria Orbitale a 8 di Lissajous (Requisito 6 & Domanda 11):
//      Curva parametrica piana regolare con rapporto armonico 1:2:
//      x(t) = x_0 + A * cos(omega * t);  y(t) = y_0 + B * sin(2 * omega * t).
//    - Animazione Interattiva di Ammiccamento (Eye-Blink) (Requisito 6):
//      Quando il player è a distanza critica (d < 65px), un edge-trigger attiva una contrazione
//      sinusoidale della scala verticale Y dell'occhio: s_y = max(0.08, |sin(pi * progress)|).
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <string>
#include "Geometry.h"
#include "Shader.h"
#include "Collider2D.h"
#include "ParticleSystem.h"

class HermiteManager
{
public:
    // Costruttore predefinito.
    HermiteManager();

    // Carica le mesh Hermite dai file OBJ su disco e inizializza i sotto-componenti geometrici su GPU.
    void init(const std::string& alienObjPath, const std::string& warpGateObjPath);

    // Imposta il livello di gioco corrente per scalare l'aggressività e la frequenza degli scatti.
    void setLevel(int level) { m_level = level; }

    // Aggiorna l'orbita di Lissajous del Drone alieno, il timer di sbattimento dell'occhio e la rotazione del Warp Gate.
    void update(float dt, float currentTime, const glm::vec2& playerPos);

    // Renderizza gli oggetti Hermite assemblando gerarchicamente teste, occhi, pupille, sopracciglia e vortice.
    void draw(const Shader& shader, float currentTime, bool gateActive);

    // Rileva l'ingresso del giocatore nel portale dimensionale (condizione di vittoria a 5 monete).
    bool checkPlayerGateEnter(const CircleCollider& playerCol, bool gateActive);

    // Rileva il passaggio del giocatore vicino alla reliquia energetica (ricarica scudi/punti).
    bool checkRelicInteraction(const CircleCollider& playerCol, ParticleSystem& particles, int& score);

    // Rileva l'impatto fisico col Drone Alieno sentinella.
    bool checkDroneCollision(const CircleCollider& playerCol) const;

    // Rileva l'impatto di un colpo laser sul Drone Rosso: lo indebolisce, ne decelera il moto e ne annulla gli scatti.
    bool checkLaserHit(const glm::vec2& bulletPos, float bulletRadius, ParticleSystem& particles, int& score);

    // Verifica se il Drone Rosso è attualmente nello stato indebolito (e ottiene i secondi residui).
    bool isWeakened() const { return m_weakenedTimer > 0.0f; }
    float getWeakenedTime() const { return m_weakenedTimer; }

    // Resetta lo stato di indebolimento del Drone.
    void resetWeakness() { m_weakenedTimer = 0.0f; m_hitFlashTimer = 0.0f; m_hitCount = 0; }

    // Attiva manualmente lo sbattimento dell'occhio del drone (durata 0.38 secondi).
    void triggerEyeBlink() { m_eyeBlinkTimer = 0.55f; }

    // Bounding volume del Drone sentinella (raggio 32px).
    CircleCollider getDroneCollider() const;

    // Bounding volume del Warp Gate (raggio 48px).
    CircleCollider getGateCollider() const;

    const glm::vec2& gatePosition() const { return m_gatePos; }
    const glm::vec2& dronePosition() const { return m_dronePos; }
    bool isRelicActive() const { return m_relicPulseTimer > 0.0f; }

private:
    // Componenti della Faccina Cattiva Hermite (Alien Sentinel Drone):
    Geometry m_headGeo;        // Testa cornuta modellata con spline cubiche di Hermite
    Geometry m_evilEyeGeo;     // Occhio giallo/arancio minaccioso
    Geometry m_pupilGeo;       // Pupilla a fessura verticale cremisi
    Geometry m_eyebrowsGeo;    // Sopracciglia inclinate aggressive
    Geometry m_evilMouthGeo;   // Bocca con denti appuntiti e ghigno malefico

    // Componenti del Portale Cosmico (Cosmic Warp Gate):
    Geometry m_gateGeo;        // Anello monumentale cavo Hermite
    Geometry m_vortexGeo;      // Vortice energetico rotante ad alta frequenza

    bool m_hasDrone = false;   // Flag di caricamento completato per il drone
    bool m_hasGate = false;    // Flag di caricamento completato per il portale

    // Stato cinematico e animazione della Faccina Cattiva:
    int m_level = 1;                        // Livello di difficoltà corrente
    glm::vec2 m_dronePos{ 640.0f, 480.0f }; // Posizione istantanea lungo l'orbita di Lissajous
    float m_droneAngle = 0.0f;              // Angolo di inclinazione / rollio istantaneo
    float m_eyeBlinkTimer = 0.0f;           // Timer per l'animazione di chiusura dell'occhio (blink)
    bool m_wasPlayerOver = false;           // Memoria di stato per attivare il blink UNA SOLA volta per passaggio

    // Meccanica di scatto imprevedibile ("giri strani" casuali per poi tornare all'orbita):
    float m_orbitPhase = 0.0f;              // Fase angolare accumulata lungo l'orbita di Lissajous
    float m_dashTimer = 0.0f;               // Timer attivo dello scatto
    float m_dashCooldown = 5.0f;            // Conto alla rovescia per il prossimo scatto imprevedibile
    glm::vec2 m_dashOffset{ 0.0f, 0.0f };   // Deviazione rapida momentanea dalla traiettoria base
    glm::vec2 m_dashDirection{ 1.0f, 0.0f };// Vettore di slancio dello scatto casuale

    // Meccanica di indebolimento del Drone Rosso al colpo laser:
    float m_weakenedTimer = 0.0f;           // Secondi rimanenti di stato indebolito
    float m_hitFlashTimer = 0.0f;           // Timer del lampo bianco di impatto
    int m_hitCount = 0;                     // Totale colpi laser andati a segno

    // Stato cinematico del Warp Gate:
    glm::vec2 m_gatePos{ 1080.0f, 560.0f }; // Posizione fluttuante in coordinate mondo
    float m_gateRotation = 0.0f;            // Angolo di rotazione dell'anello monumentale
    float m_vortexRotation = 0.0f;          // Angolo di rotazione del vortice dimensionale
    float m_relicPulseTimer = 0.0f;         // Timer del bagliore ad onda di risonanza
    bool m_wasPlayerNearRelic = false;      // Stato di prossimità per l'interazione
};
