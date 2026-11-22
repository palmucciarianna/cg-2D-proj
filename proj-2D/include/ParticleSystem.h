// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: ParticleSystem (Simulatore Particellare)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Simula la fisica e il rendering di centinaia di particelle dinamiche (Requisiti 6, 7).
//    - Produce feedback visivi immediati ed spettacolari:
//      * Polverizzazione di 32 frammenti rocciosi alla distruzione di un asteroide;
//      * Scintille dorate alla raccolta di ogni moneta;
//      * Esplosione trionfale di coriandoli colorati nello stato LEVEL_CLEARED_HURRA;
//      * Scia di fumo del reattore della navicella.
//
// 2. A CHI È COLLEGATA:
//    - Creata e gestita da: GameWorld (chiamata in init(), update(dt), draw(shader)).
//    - Invocata da: GameWorld, AsteroidManager, CollectibleManager, LaserManager per emettere particelle.
//    - Utilizza: Geometry per il quad/punto particellare e Shader basic con abilitazione del Blending.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Integrazione Fisica Euleriana con Smorzamento Viscoso (Requisito 6 & Domande 4, 16):
//      * v(t + dt) = v(t) * (1 - gamma * dt)  (attrito viscoso spaziale);
//      * p(t + dt) = p(t) + v(t) * dt;
//      * theta(t + dt) = theta(t) + omega * dt.
//    - Dissolvenza Lineare dell'Alpha & Restringimento Geometrico (Domanda 20):
//      Fattore di vita: ratio = life / maxLife;
//      alpha(t) = color.a * ratio;  size(t) = base_size * ratio.
//    - Equazione di Alpha Blending (Domanda 20):
//      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) fonde i colori:
//      C_out = alpha_s * C_s + (1 - alpha_s) * C_d, eliminando l'effetto "quadrato" nero dei detriti.
//    - Allocazione Dinamica & Compattazione Vettori:
//      Le particelle morte (life <= 0) vengono eliminate con l'idioma remove_if (Erase-Remove).
// ============================================================================

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Geometry.h"
#include "Shader.h"

// Struttura dati che definisce lo stato fisico di una singola particella.
struct Particle
{
    glm::vec2 pos;      // Posizione cartesiana 2D nello spazio mondo
    glm::vec2 vel;      // Vettore velocità istantanea (pixel/secondo)
    glm::vec4 color;    // Colore di base in formato RGBA
    float size;         // Dimensione geometrica di base
    float rotation;     // Angolo di orientamento corrente attorno al centro (in gradi)
    float rotSpeed;     // Velocità di rotazione (gradi/secondo)
    float life;         // Tempo di vita residuo (in secondi)
    float maxLife;      // Durata di vita iniziale massima per il calcolo della dissolvenza
};

class ParticleSystem
{
public:
    // Costruttore: inizializza il sistema particellare.
    ParticleSystem();

    // Alloca e carica sulla GPU la mesh triangolare elementare utilizzata per renderizzare ciascuna particella.
    void init();

    // Aggiorna lo stato fisico di tutte le particelle attive (moto Euleriano, attrito viscoso, rotazione, decadimento).
    void update(float dt);

    // Renderizza le particelle attive applicando traslazione, rotazione, ridimensionamento e dissolvenza alfa.
    void draw(const Shader& shader);

    // Emette un'esplosione radiale di detriti rocciosi da un punto d'impatto (quando l'asteroide va in pezzi).
    void emitAsteroidExplosion(const glm::vec2& origin, int count = 28);

    // Emette scintille dorate brillanti al contatto con una moneta cosmica.
    void emitCoinSparkles(const glm::vec2& origin, int count = 16);

    // Emette coriandoli multicolore e fuochi d'artificio per la vittoria e passaggio di livello ("HURRÀ!").
    void emitHurraCelebration(float screenWidth, float screenHeight, int count = 75);

    // Emette particelle energetiche ciano/viola dalla reliquia cosmica.
    void emitRelicEnergy(const glm::vec2& origin, int count = 20);

    // Emette scintille ed energia di dispersione all'impatto laser con il Drone Rosso.
    void emitDroneHitSparks(const glm::vec2& origin, int count = 18);

    // Cancella istantaneamente tutte le particelle attualmente attive.
    void clear() { m_particles.clear(); }

private:
    std::vector<Particle> m_particles; // Pool di particelle dinamiche
    Geometry m_rockFragGeo;            // Geometria triangolare base condivisa per il rendering
};
