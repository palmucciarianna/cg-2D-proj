// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: LaserManager (Proiettili Plasma)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Gestisce i proiettili laser sparati dalla navicella con la barra spaziatrice (Requisito 7.1).
//    - Propaga i raggi a velocità supersonica (720-950 px/s) lungo la direzione frontale della nave.
//    - Rileva gli impatti contro gli asteroidi (frantumandoli) e contro l'Alien Drone (indebolendolo).
//    - Implementa il clipping logico eliminando i proiettili che escono dai confini dello schermo.
//
// 2. A CHI È COLLEGATA:
//    - Creata e gestita da: GameWorld (in init(), update(dt), checkCollisions(), draw(shader)).
//    - Riceve comandi da: Player (spawn con posizione muso e versore di direzione d).
//    - Interagisce con: AsteroidManager (distrugge asteroidi), HermiteManager (stordisce drone),
//      ParticleSystem (innesca l'esplosione rocciosa dei 32 frammenti).
//    - Utilizza: Geometry per il quad del raggio laser e lo Shader basic.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Cinematica Rettilinea & Versore Direzione (Domande 4, 5):
//      Dato l'angolo theta della navicella, il versore di direzione e' d = (cos(theta), sin(theta)).
//      La posizione si aggiorna a ogni frame con: p(t + dt) = p(t) + d * v_laser * dt.
//    - Bounding Box & Clipping Software 2D (Domanda 10):
//      Verifica se p.x o p.y eccedono la finestra [0, 1280] x [0, 720]; se sì, dealloca il raggio
//      per risparmiare memoria (analogo logico del clipping di Cohen-Sutherland).
//    - Orientamento Matrice Model per Segmento/Quad (Domanda 6):
//      M = T(pos) * R(theta) * S(lunghezza, spessore), allineando il quad alla direzione di sparo.
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"
#include "ParticleSystem.h"
#include "AsteroidManager.h"

class HermiteManager;

// Struttura dati per un singolo colpo laser.
struct LaserBullet
{
    glm::vec2 pos;  // Posizione cartesiana 2D istantanea
    glm::vec2 dir;  // Vettore versore di direzione normalizzato
    float life;     // Tempo di vita residuo del raggio (in secondi)
};

class LaserManager
{
public:
    // Costruttore predefinito.
    LaserManager();

    // Inizializza la geometria del fascio laser (rettangolo rastremato bianco/azzurro) su GPU.
    void init();

    // Spara un nuovo dardo laser partendo dalla posizione di origine nella direzione specificata.
    void fire(const glm::vec2& origin, const glm::vec2& direction);

    // Aggiorna la posizione dei colpi, verifica le collisioni con asteroidi e con il Drone Rosso ed eroga punti bonus.
    void update(float dt, AsteroidManager& asteroids, HermiteManager& hermite, ParticleSystem& particles, int& score);

    // Renderizza tutti i proiettili laser orientandoli lungo il rispettivo vettore di propagazione.
    void draw(const Shader& shader);

    // Rimuove tutti i proiettili attivi (es. al reset del livello).
    void clear() { m_bullets.clear(); }

private:
    std::vector<LaserBullet> m_bullets; // Collezione dei colpi laser attivi
    Geometry m_bulletGeo;               // Risorsa geometrica condivisa su GPU (VAO/VBO)
};
