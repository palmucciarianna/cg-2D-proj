// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: ParticleSystem.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Simula la cinematica e il rendering di centinaia di particelle per effetti visivi reattivi.
//    - Esplosione asteroidi (32 detriti rocciosi), scintille dorate, coriandoli HURRÀ e scia fumo.
// 2. A CHI È COLLEGATO:
//    - Invocato da GameWorld, AsteroidManager, CollectibleManager e LaserManager.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - Smorzamento per attrito viscoso: v(t+dt) = v(t) * (1 - gamma * dt) (Domanda 4).
//    - Dissolvenza lineare dell'alpha: a(t) = a0 * (life / maxLife) (Domanda 20).
//    - Composizione in Alpha Blending: Cout = as*Cs + (1-as)*Cd (Domanda 20).
// ============================================================================

#include "ParticleSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>
#include "ShapeGenerator.h"

// Funzione statica di supporto per generare numeri pseudo-casuali continui nell'intervallo [minVal, maxVal].
static float randRange(float minVal, float maxVal)
{
    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return minVal + r * (maxVal - minVal);
}

// ============================================================================
// Costruttore
// ============================================================================
ParticleSystem::ParticleSystem()
{
}

// ============================================================================
// Inizializzazione della Mesh Condivisa per le Particelle
// ============================================================================
void ParticleSystem::init()
{
    // Genera una micro-mesh triangolare con sfumatura di profondità.
    // Viene riutilizzata per tutti i tipi di particella (frammenti, scintille, coriandoli).
    MeshData fragMesh = ShapeGenerator::createTriangle(
        glm::vec4(0.9f, 0.9f, 0.9f, 1.0f),
        glm::vec4(0.7f, 0.7f, 0.7f, 1.0f),
        glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
    );

    // Carica la geometria in GPU.
    m_rockFragGeo.setData(fragMesh.positions, fragMesh.colors, fragMesh.primitiveType);
    m_rockFragGeo.upload(false);

    // Pre-alloca la capacità del vettore per evitare frequenti riallocazioni di memoria dinamica.
    m_particles.reserve(256);
}

// ============================================================================
// Aggiornamento Fisico e Ciclo di Vita delle Particelle
// ============================================================================
void ParticleSystem::update(float dt)
{
    for (size_t i = 0; i < m_particles.size(); )
    {
        Particle& p = m_particles[i];

        // 1. Decrementa il tempo di vita residuo.
        p.life -= dt;

        // 2. Se la vita è esaurita (<= 0), elimina la particella dal vettore.
        if (p.life <= 0.0f)
        {
            m_particles.erase(m_particles.begin() + i);
            continue; // Non incrementa l'indice 'i' poiché il vettore è scalato
        }

        // 3. Integrazione di Eulero della posizione: p = p + v * dt.
        p.pos += p.vel * dt;

        // 4. Modello di attrito fluido lineare dello spazio cosmico:
        // La velocità decade del 75% per secondo (v = v * (1 - 0.75 * dt)).
        p.vel *= (1.0f - 0.75f * dt);

        // 5. Aggiornamento dell'orientamento angolare: rot = rot + rotSpeed * dt.
        p.rotation += p.rotSpeed * dt;

        // Passa alla particella successiva.
        ++i;
    }
}

// ============================================================================
// Disegno delle Particelle con Dissolvenza Alfa
// ============================================================================
void ParticleSystem::draw(const Shader& shader)
{
    // Se non ci sono particelle attive, esce subito.
    if (m_particles.empty()) return;

    for (const auto& p : m_particles)
    {
        // Calcola la percentuale di vita rimanente in [0.0, 1.0]:
        // progress = 1.0 alla nascita, progress = 0.0 alla morte.
        float progress = p.life / p.maxLife;

        // Ridimensionamento dimensionale: la particella si rimpicciolisce mentre invecchia.
        float currentScale = p.size * (0.35f + 0.65f * progress);

        // Calcola la Model Matrix locale: M = T * R * S.
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(p.pos, 0.0f));
        m = glm::rotate(m, glm::radians(p.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        m = glm::scale(m, glm::vec3(currentScale, currentScale, 1.0f));

        // Applica la dissolvenza alfa: il canale di trasparenza sfuma a zero verso fine vita.
        glm::vec4 c = p.color;
        c.a *= progress;

        // Imposta le uniform del colore e della matrice di modellazione.
        shader.setVec4("uColorTint", c);
        shader.setMat4("model", m);

        // Disegna il frammento triangolare.
        m_rockFragGeo.draw();
    }

    // Ripristina la tinta uniforme a bianco opaco (1.0).
    shader.setVec4("uColorTint", glm::vec4(1.0f));
}

// ============================================================================
// Emissione Detriti Esplosione Asteroide (Frantumazione in Mille Pezzi)
// ============================================================================
void ParticleSystem::emitAsteroidExplosion(const glm::vec2& origin, int count)
{
    for (int i = 0; i < count; ++i)
    {
        // Direzione angolare casuale a 360 gradi [0, 2*PI].
        float angle = randRange(0.0f, 6.28318f);
        // Velocità esplosiva di proiezione [80, 320] px/s.
        float speed = randRange(80.0f, 320.0f);

        Particle p;
        // Posizione iniziale con piccolo offset casuale attorno al centro dell'asteroide.
        p.pos = origin + glm::vec2(std::cos(angle), std::sin(angle)) * randRange(2.0f, 18.0f);
        // Vettore velocità cartesiano.
        p.vel = glm::vec2(std::cos(angle), std::sin(angle)) * speed;

        // Variazione cromatica dei frammenti: toni rocciosi e fiammate arancioni da impatto.
        float tone = randRange(0.4f, 0.9f); // Luminosità di base casuale
        float r = tone * randRange(0.9f, 1.2f); // Componente rossa calda
        float g = tone * randRange(0.6f, 0.9f); // Componente verde media
        float b = tone * randRange(0.4f, 0.6f); // Componente blu attenuata
        p.color = glm::vec4(r, g, b, 1.0f);     // Assegna il colore RGBA opaco iniziale

        p.size = randRange(10.0f, 24.0f);        // Dimensione casuale della scheggia in pixel
        p.rotation = randRange(0.0f, 360.0f);    // Angolo iniziale casuale
        p.rotSpeed = randRange(-350.0f, 350.0f); // Velocità di rotazione su se stessa (deg/s)
        p.maxLife = randRange(0.5f, 1.1f);       // Durata totale della particella in secondi
        p.life = p.maxLife;                      // Inizializza la vita residua al massimo

        m_particles.push_back(p); // Aggiunge la particella al pool attivo
    }
}

// ============================================================================
// Emissione Scintille Dorate Raccolta Moneta
// ============================================================================
void ParticleSystem::emitCoinSparkles(const glm::vec2& origin, int count)
{
    for (int i = 0; i < count; ++i)
    {
        float angle = randRange(0.0f, 6.28318f); // Direzione radiale casuale [0, 2*PI]
        float speed = randRange(60.0f, 200.0f);  // Velocità iniziale [60, 200] px/s

        Particle p;                              // Nuova particella scintilla
        p.pos = origin;                          // Parte dal centro della moneta o scia
        p.vel = glm::vec2(std::cos(angle), std::sin(angle)) * speed; // Vettore velocità cartesiano
        // Colore giallo oro scintillante.
        p.color = glm::vec4(1.0f, randRange(0.8f, 1.0f), randRange(0.1f, 0.4f), 1.0f);
        p.size = randRange(6.0f, 14.0f);         // Dimensione ridotta per effetto scintilla
        p.rotation = randRange(0.0f, 360.0f);    // Orientamento iniziale casuale
        p.rotSpeed = randRange(-180.0f, 180.0f); // Rotazione moderata (deg/s)
        p.maxLife = randRange(0.4f, 0.75f);      // Vita breve per effetto lampo
        p.life = p.maxLife;                      // Inizializza il timer di vita

        m_particles.push_back(p); // Inserisce la scintilla nel pool
    }
}

// ============================================================================
// Emissione Coriandoli e Fuochi per "HURRÀ! Nuovo Livello"
// ============================================================================
void ParticleSystem::emitHurraCelebration(float screenWidth, float screenHeight, int count)
{
    for (int i = 0; i < count; ++i)
    {
        Particle p; // Nuovo coriandolo celebrativo
        // Posizione casuale distribuita su un ampio rettangolo centrale dello schermo.
        p.pos = glm::vec2(
            randRange(screenWidth * 0.15f, screenWidth * 0.85f),
            randRange(screenHeight * 0.3f, screenHeight * 0.7f)
        );

        float angle = randRange(0.0f, 6.28318f); // Angolo casuale a 360 gradi
        float speed = randRange(120.0f, 380.0f); // Velocità di espansione esplosiva
        p.vel = glm::vec2(std::cos(angle), std::sin(angle)) * speed; // Vettore velocità

        // Palette cromatica festosa: oro brillante, ciano cosmico, magenta e verde smeraldo.
        static const glm::vec4 celebrationColors[] = {
            glm::vec4(1.0f, 0.85f, 0.1f, 1.0f), // Oro brillante
            glm::vec4(0.2f, 0.95f, 1.0f, 1.0f), // Ciano cosmico
            glm::vec4(1.0f, 0.25f, 0.7f, 1.0f), // Magenta festoso
            glm::vec4(0.25f, 1.0f, 0.4f, 1.0f)  // Verde smeraldo
        };
        p.color = celebrationColors[rand() % 4]; // Seleziona uno dei 4 colori a caso

        p.size = randRange(12.0f, 26.0f);        // Dimensione grande per i coriandoli
        p.rotation = randRange(0.0f, 360.0f);    // Angolo iniziale
        p.rotSpeed = randRange(-280.0f, 280.0f); // Spin rapido in aria
        p.maxLife = randRange(1.0f, 1.8f);       // Durata lunga per coprire la schermata HURRÀ!
        p.life = p.maxLife;                      // Inizializza la vita residua

        m_particles.push_back(p); // Inserisce il coriandolo nel pool
    }
}

// ============================================================================
// Emissione Onde di Energia dalla Reliquia Spaziale
// ============================================================================
void ParticleSystem::emitRelicEnergy(const glm::vec2& origin, int count)
{
    for (int i = 0; i < count; ++i)
    {
        float angle = randRange(0.0f, 6.28318f); // Direzione radiale casuale
        float speed = randRange(90.0f, 260.0f);  // Velocità delle onde di energia

        Particle p; // Particella energetica della reliquia
        p.pos = origin + glm::vec2(std::cos(angle), std::sin(angle)) * randRange(10.0f, 35.0f); // Spawn su anello attorno all'origine
        p.vel = glm::vec2(std::cos(angle), std::sin(angle)) * speed; // Vettore velocità radiale

        // Tonalità eteree ciano, viola quantico e bianco energia.
        static const glm::vec4 relicColors[] = {
            glm::vec4(0.2f, 0.9f, 1.0f, 1.0f), // Ciano etereo
            glm::vec4(0.5f, 0.4f, 1.0f, 1.0f), // Viola quantico
            glm::vec4(0.8f, 1.0f, 1.0f, 1.0f)  // Bianco energia
        };
        p.color = relicColors[rand() % 3]; // Sceglie casualmente tra i 3 colori

        p.size = randRange(8.0f, 18.0f);         // Dimensione media
        p.rotation = randRange(0.0f, 360.0f);    // Orientamento casuale
        p.rotSpeed = randRange(-200.0f, 200.0f); // Velocità di rotazione
        p.maxLife = randRange(0.6f, 1.2f);       // Durata vita
        p.life = p.maxLife;                      // Inizializza la vita residua

        m_particles.push_back(p); // Accoda la particella nel sistema
    }
}

// ============================================================================
// Emissione Scintille Elettrificate all'Impatto Laser col Drone Rosso
// ============================================================================
void ParticleSystem::emitDroneHitSparks(const glm::vec2& origin, int count)
{
    for (int i = 0; i < count; ++i)
    {
        float angle = randRange(0.0f, 6.28318f); // Angolo casuale a 360 gradi
        float speed = randRange(70.0f, 260.0f);  // Velocità di dispersione scintille

        Particle p; // Particella scintilla elettrica
        p.pos = origin + glm::vec2(std::cos(angle), std::sin(angle)) * randRange(4.0f, 18.0f); // Spawn vicino al punto d'impatto
        p.vel = glm::vec2(std::cos(angle), std::sin(angle)) * speed; // Vettore velocità

        // Scintille ad alto contrasto: cremisi, oro rovente e azzurro ionico da dispersione di campo
        static const glm::vec4 sparkColors[] = {
            glm::vec4(1.0f, 0.2f, 0.35f, 1.0f),  // Cremisi vivo
            glm::vec4(1.0f, 0.85f, 0.2f, 1.0f),  // Oro rovente
            glm::vec4(0.3f, 0.9f, 1.0f, 1.0f),   // Azzurro laser ionizzato
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)    // Bianco plasma
        };
        p.color = sparkColors[rand() % 4]; // Colore casuale dalla palette ad alto contrasto

        p.size = randRange(7.0f, 16.0f);         // Dimensione scintilla
        p.rotation = randRange(0.0f, 360.0f);    // Angolo iniziale
        p.rotSpeed = randRange(-320.0f, 320.0f); // Rotazione veloce
        p.maxLife = randRange(0.4f, 0.85f);      // Vita rapida
        p.life = p.maxLife;                      // Inizializza vita residua

        m_particles.push_back(p); // Aggiunge la scintilla al pool
    }
}
