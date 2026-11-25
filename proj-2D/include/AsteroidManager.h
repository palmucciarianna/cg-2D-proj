// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: AsteroidManager
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Gestisce la cintura di asteroidi ostili (da 4 a 10 a seconda del livello di gioco).
//    - Implementa la tecnica cardine del Buffer Sharing (Requisito 4):
//      tutti gli asteroidi condividono la STESSA geometria su VRAM (un solo VAO e VBO).
//    - Gestisce la distruzione degli asteroidi all'impatto con i laser o la navicella,
//      innescando il sistema particellare di frammentazione rocciosa.
//
// 2. A CHI È COLLEGATA:
//    - Creata e gestita da: GameWorld (in init(), spawnLevel(), update(dt), draw(shader)).
//    - Collabora con:
//      * ShapeGenerator: crea la mesh CPU poligonale irregolare (cerchio perturbato a 13 segmenti).
//      * Geometry: memorizza l'unico blocco di dati su VRAM (m_asteroidGeo).
//      * Collider2D: fornisce il CircleCollider (r = 30px * scale) per i test di collisione.
//      * ParticleSystem: chiamato da GameWorld per generare l'esplosione di 32 detriti rocciosi.
//      * Shader basic: riceve la matrice 'uniform mat4 model' per ogni istanza disegnata.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Buffer Sharing & Instancing Logico (Requisito 4 & Domande 4, 17):
//      Invece di inviare vertici alla GPU a ogni frame o allocare N buffer identici in VRAM,
//      esiste 1 sola copia geometrica caricata con GL_STATIC_DRAW. Nel render loop la CPU invia
//      soltanto la matrice Model: M_i = T(pos_i) * R(rot_i) * S(scale_i).
//    - Distribuzione Spaziale in Coordinate Polari (Domanda 5):
//      Gli asteroidi sono generati in posizioni polari attorno al centro dello schermo:
//      x = x_centro + cos(theta) * r,  y = y_centro + sin(theta) * r, con theta ed r casuali,
//      impedendo che compaiano direttamente sopra la navicella del giocatore.
//    - Differenza tra Geometria e Istanza (Domanda 17):
//      'Geometry' è il dato oggettivo della forma locale condivisa; 'Asteroid' (Instance)
//      rappresenta lo stato individuale (posizione, rotazione, scala, stato vitale attivo).
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"
#include "Collider2D.h"
#include "ParticleSystem.h"

// Struttura dati CPU per una singola istanza di asteroide nel mondo di gioco.
struct Asteroid
{
    glm::vec2 pos;      // Posizione 2D nel sistema di coordinate del mondo
    float rotation;     // Angolo di rotazione corrente attorno al proprio baricentro (in gradi)
    float rotSpeed;     // Velocità angolare di rotazione in gradi/secondo (oraria o antioraria)
    float scale;        // Fattore di scala dimensionale (variazione di grandezza)
    float radius;       // Raggio effettivo del collider circolare (30px * scale)
    bool active = true; // Stato di vita: true = intatto e visibile, false = distrutto dal laser
};

class AsteroidManager
{
public:
    // Costruttore: inizializza il gestore delle istanze.
    AsteroidManager();

    // Alloca e carica sulla GPU l'unica geometria condivisa dell'asteroide (13 segmenti conici).
    void init();

    // Genera proceduralmente gli asteroidi distribuiti a raggiera attorno al punto di spawn del giocatore.
    void spawnLevel(int levelCount, float screenWidth, float screenHeight);

    // Aggiorna l'angolo di rotazione di ciascun asteroide attivo integrando la velocità angolare col delta-time.
    void update(float dt);

    // Renderizza tutti gli asteroidi attivi iterando sull'unica mesh condivisa e impostando le matrici Uniform.
    void draw(const Shader& shader, float currentTime);

    // Verifica la collisione tra un colpo laser del giocatore e gli asteroidi:
    // in caso di impatto, disattiva l'asteroide ed emette un'esplosione di 32 frammenti rocciosi.
    bool checkLaserHit(const glm::vec2& bulletPos, float bulletRadius, ParticleSystem& particles);

    // Verifica se il giocatore è entrato in collisione con uno qualsiasi degli asteroidi attivi.
    bool checkPlayerCollision(const CircleCollider& playerCol);

    // Restituisce il conteggio degli asteroidi ancora intatti nella scena.
    int activeCount() const;

private:
    std::vector<Asteroid> m_asteroids; // Collezione delle istanze di asteroidi residenti nel livello
    Geometry m_asteroidGeo;            // Singola geometria condivisa in memoria GPU (Buffer Sharing)
};
