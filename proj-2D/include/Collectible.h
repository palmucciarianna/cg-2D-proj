// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: CollectibleManager (Cosmic Coins)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Gestisce le 5 monete d'oro collezionabili richieste per sbloccare il Warp Gate (Requisito 7.2).
//    - Implementa l'animazione di rotazione e l'oscillazione armonica verticale (Requisito 6).
//    - Condivide un unico VAO/VBO in VRAM per tutte le 5 monete (Buffer Sharing come per gli asteroidi).
//    - Notifica GameWorld della raccolta, erogando punti e generando scintille dorate particellari.
//
// 2. A CHI È COLLEGATA:
//    - Creata e gestita da: GameWorld (in init(), spawnLevel(), update(dt), draw(shader)).
//    - Collabora con:
//      * ShapeGenerator: crea la mesh CPU della moneta d'oro con gradiente radiale a 36 segmenti.
//      * Geometry: memorizza l'unico VBO/VAO della moneta su GPU (m_coinGeo).
//      * Collider2D: fornisce il CircleCollider (r = 22px) per il test di raccolta contro il Player.
//      * Shader basic o coin.vert: riceve la Model Matrix con scalatura non uniforme e parametri uniform.
//      * ParticleSystem: chiamato da GameWorld per emettere particelle dorate alla raccolta.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Rotazione via Proiezione Ortogonale (Requisito 6 & Domande 6, 9):
//      Un disco circolare 3D che ruota attorno all'asse Y a velocità angolare omega, se proiettato
//      ortogonalmente sul piano XY, si comprime orizzontalmente come:
//      s_x(t) = s_base * |cos(omega * t)|.
//      Applicando questa legge al fattore di scala X della Model Matrix, il cerchio 2D appare all'occhio
//      umano come una solida moneta 3D che ruota su se stessa nello spazio!
//    - Oscillazione Armonica Verticale (Levitazione) (Requisito 6):
//      y(t) = y_base + A * sin(nu * t + phi), dove phi de-sincronizza il moto tra monete diverse.
//    - Interpolazione del Colore Gouraud (Domande 7, 14):
//      La moneta possiede un vertice centrale dorato brillante e vertici periferici ambrati;
//      le coordinate baricentriche della rasterizzazione interpolano i colori generando riflessi metallici.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"
#include "Collider2D.h"
#include "ParticleSystem.h"

// Struttura dati per una singola moneta d'oro nel mondo 2D.
struct CosmicCoin
{
    glm::vec2 basePos;       // Posizione centrale di riposo (ancora dell'oscillazione)
    glm::vec2 currentPos;    // Posizione istantanea calcolata con l'oscillazione armonica
    float phaseOffset = 0.0f;// Sfasamento temporale per de-sincronizzare il galleggiamento tra monete
    float radius = 22.0f;    // Raggio geometrico del collider circolare in pixel
    bool collected = false;  // Flag di stato: true quando la moneta è stata raccolta dal giocatore
};

class CollectibleManager
{
public:
    // Costruttore predefinito.
    CollectibleManager();

    // Inizializza la geometria circolare della moneta dorata a 36 segmenti e la carica su GPU.
    void init();

    // Spawna 5 monete d'oro distribuite proceduralmente nel livello.
    void spawnLevel(int levelCount, float screenWidth, float screenHeight);

    // Aggiorna l'animazione armonica verticale delle monete non ancora raccolte.
    void update(float dt, float currentTime);

    // Renderizza le monete attive applicando la scala pseudo-3D sulla coordinata X locale.
    void draw(const Shader& shader, float currentTime);

    // Verifica la collisione con il bounding circle del giocatore:
    // raccoglie la moneta, genera scintille particellari dorate e restituisce il numero di monete colte.
    int checkCollisions(const CircleCollider& playerCol, ParticleSystem& particles);

    // Metodi di ispezione dello stato della missione:
    int totalCoins() const { return static_cast<int>(m_coins.size()); }
    int collectedCoins() const { return m_collectedCount; }
    int remainingCoins() const { return totalCoins() - m_collectedCount; }
    bool allCollected() const { return totalCoins() > 0 && m_collectedCount >= totalCoins(); }

private:
    std::vector<CosmicCoin> m_coins; // Collezione delle 5 monete del livello
    Geometry m_coinGeo;              // Risorsa geometrica condivisa su GPU (VAO/VBO)
    int m_collectedCount{ 0 };       // Contatore monete raccolte nel livello corrente
};
