// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: HazardManager (Pericoli Perimetrali)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Gestisce i pericoli dinamici ad alta velocità (missili di plasma da 880 px/s).
//    - Implementa la meccanica arcade "telegraph-and-fire":
//      * Fase 1: Spia stroboscopica d'allerta sul bordo per 0.85s (dà tempo al giocatore di reagire);
//      * Fase 2: Lancio del dardo supersonico che attraversa trasversalmente la schermata.
//    - Se impatta la navicella, sottrae 1 vita, attiva Screen Shake e genera detriti particellari.
//
// 2. A CHI È COLLEGATA:
//    - Creata e gestita da: GameWorld (in init(), spawnLevel(), update(dt), checkCollisions(), draw()).
//    - Interagisce con: Player (collisioni cerchio-AABB), ParticleSystem (esplosione da impatto).
//    - Utilizza: Geometry per il proiettile a punta di diamante e lo Shader basic.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Lampeggio Stroboscopico con Funzione Seno (Requisito 6 & Domanda 20):
//      alpha(t) = 0.5 + 0.5 * sin(freq * t);  invia 'uColorTint' con alpha variabile per l'allerta.
//    - Collisione Cerchio vs AABB (Domanda 5 & Collider2D):
//      Calcola il punto più vicino sul rettangolo del dardo tramite clamping del centro della navicella:
//      p_clamped = clamp(player_pos, box_min, box_max);  verifica se ||player_pos - p_clamped||^2 <= r^2.
//    - Trasformazione di Scala e Posizione Spaziale (Domanda 6):
//      La matrice Model mappa la freccia di plasma posizionandola e orientandola lungo la traiettoria.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"
#include "Collider2D.h"
#include "ParticleSystem.h"

// Enumerazione del bordo di provenienza del pericolo spaziale.
enum class HazardSide
{
    LEFT,   // Proviene dal bordo sinistro
    RIGHT,  // Proviene dal bordo destro
    TOP,    // Proviene dal bordo superiore
    BOTTOM  // Proviene dal bordo inferiore
};

// Struttura dati per un singolo pericolo in transito.
struct HazardShot
{
    glm::vec2 pos;      // Posizione 2D corrente
    glm::vec2 dir;      // Versore di direzione del lancio
    float speed;        // Velocità di propagazione (880 px/s)
    float radius;       // Raggio del volume di collisione (16px)
    float warningTimer; // Timer di preavviso lampeggiante prima del lancio
    bool fired;         // true se il proiettile è stato sparato ed è in volo
    bool active;        // true se il proiettile è attivo e pericoloso
    HazardSide side;    // Bordo da cui ha origine il pericolo
};

class HazardManager
{
public:
    // Costruttore predefinito.
    HazardManager();

    // Inizializza le geometrie del dardo di plasma (rombo brillante) e del mirino di allerta.
    void init();

    // Ripristina lo stato del gestore delle minacce.
    void reset();

    // Imposta il livello corrente per scalare la frequenza e le direzioni di attacco.
    void setLevel(int level) { m_level = level; }

    // Aggiorna timer di spawn casuale, animazione dei preavvisi e moto dei proiettili.
    void update(float dt, float screenWidth, float screenHeight, ParticleSystem& particles);

    // Disegna sia gli indicatori di allerta lampeggianti sui bordi sia i dardi di plasma in volo.
    void draw(const Shader& shader, float currentTime, float screenWidth, float screenHeight);

    // Verifica se un dardo di plasma attivo ha colpito il giocatore.
    // uno dei proiettili nemici laterali ha colpito la mia navicella?
    bool checkPlayerHit(const CircleCollider& playerCol, ParticleSystem& particles);

    // Cancella tutte le minacce attive nella scena.
    void clear() { m_hazards.clear(); }

private:
    int m_level = 1;                    // Livello corrente: incrementa la frequenza e abilita attacchi dall'alto (>= 5)
    float m_spawnCooldown = 3.5f;       // Cooldown tra lanci consecutivi di pericoli
    std::vector<HazardShot> m_hazards;  // Vettore delle minacce attive

    Geometry m_projectileGeo;           // Mesh a rombo per il proiettile di plasma rosso
    Geometry m_warningGeo;              // Mesh rettangolare per l'indicatore stroboscopico di allerta
};
