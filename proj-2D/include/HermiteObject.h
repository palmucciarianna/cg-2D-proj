// ============================================================================
// File: include/HermiteObject.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Gestore degli oggetti curvilinei generati con curve cubiche di Hermite (Requisito 3.2):
//              1. Alien Sentinel Drone (Faccina cattiva cornuta con animazione sbattimento occhio)
//              2. Cosmic Warp Gate (Portale dimensionale ad anello monumentale e vortice rotante)
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

    // Controllo e tracciamento didattico della rotta orbitale a 8 (Curva di Lissajous):
    bool showOrbit() const { return m_showOrbit; }
    void setShowOrbit(bool s) { m_showOrbit = s; }
    bool* getShowOrbitPtr() { return &m_showOrbit; }

    // Evoluzione Drone Alieno: Modalita' Boss Hunter & Mini-Droni Scorta (v2):
    void setBossMode(bool b) { m_isBoss = b; if (b && m_bossHp <= 0) m_bossHp = m_bossMaxHp; }
    bool isBoss() const { return m_isBoss; }
    bool* getBossModePtr() { return &m_isBoss; }
    int bossHp() const { return m_bossHp; }
    int bossMaxHp() const { return m_bossMaxHp; }
    bool isEscortActive(int i) const { return (i >= 0 && i < 2) ? m_escortsActive[i] : false; }
    const glm::vec2& escortPos(int i) const { return m_escortsPos[i]; }

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
    Geometry m_orbitLineGeo;   // Tracciato visuale dell'orbita a figura di 8 di Lissajous

    bool m_hasDrone = false;   // Flag di caricamento completato per il drone
    bool m_hasGate = false;    // Flag di caricamento completato per il portale
    bool m_showOrbit = true;   // Flag per mostrare a schermo il tracciato dell'orbita (Debug/Didattica)

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

    // Modalita' Boss Hunter e scorta (v2):
    bool m_isBoss = false;
    int m_bossHp = 8;
    int m_bossMaxHp = 8;
    bool m_escortsActive[2] = { true, true };
    glm::vec2 m_escortsPos[2]{ glm::vec2(0.0f), glm::vec2(0.0f) };
    bool m_targetingActive = false;
    glm::vec2 m_targetAimPos{ 0.0f };
    Geometry m_targetingLineGeo;
};
