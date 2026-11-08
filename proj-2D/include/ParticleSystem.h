// ============================================================================
// File: include/ParticleSystem.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Sistema di simulazione particellare basato su cinematica fisica.
//              Genera e gestisce effetti visivi dinamici:
//              1. Frantumazione in mille pezzi degli asteroidi colpiti dal laser
//              2. Scintille dorate alla raccolta delle monete
//              3. Celebrazione pirotecnica e coriandoli "HURRÀ!" per il completamento livello
//              4. Bagliori energetici di risonanza della reliquia cosmica
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

    // Cancella istantaneamente tutte le particelle attualmente attive.
    void clear() { m_particles.clear(); }

private:
    std::vector<Particle> m_particles; // Pool di particelle dinamiche
    Geometry m_rockFragGeo;            // Geometria triangolare base condivisa per il rendering
};
