// ============================================================================
// File: include/LaserManager.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Gestore balistico dei colpi laser (proiettili plasma).
//              Gestisce il ciclo di vita (sparo con barra spaziatrice), la traiettoria rettilinea
//              ad alta velocità (720 px/s), la collisione con gli asteroidi e l'assegnazione punti.
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
