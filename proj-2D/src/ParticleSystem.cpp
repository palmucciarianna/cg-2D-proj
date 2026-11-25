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
#include "MathConstants.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>
#include "ShapeGenerator.h"

namespace
{
    // Descrittore parametrico condiviso per generare un burst di particelle con qualsiasi palette/range.
    struct ParticleBurstSpec
    {
        float minOffset = 0.0f, maxOffset = 0.0f;
        float minSpeed = 80.0f, maxSpeed = 200.0f;
        float minSize = 8.0f, maxSize = 18.0f;
        float maxRotSpeed = 200.0f;
        float minLife = 0.4f, maxLife = 1.0f;
    };

    // Helper unificato che genera 'count' particelle radiali attorno a 'origin' usando i parametri di 'spec'.
    template <typename ColorFn>
    void spawnRadialBurst(
        std::vector<Particle>& particles,
        const glm::vec2& origin,
        int count,
        const ParticleBurstSpec& spec,
        ColorFn&& pickColor)
    {
        for (int i = 0; i < count; ++i)
        {
            float angle = MathConstants::randRange(0.0f, MathConstants::TWO_PI); // Direzione casuale [0, 2*PI]
            float speed = MathConstants::randRange(spec.minSpeed, spec.maxSpeed); // Velocità iniziale casuale
            float offset = (spec.maxOffset > 0.0f) ? MathConstants::randRange(spec.minOffset, spec.maxOffset) : 0.0f;

            Particle p;
            p.pos = origin + glm::vec2(std::cos(angle), std::sin(angle)) * offset; // Posizione iniziale
            p.vel = glm::vec2(std::cos(angle), std::sin(angle)) * speed;           // Vettore velocità cartesiano
            p.color = pickColor();                                                 // Colore dalla palette scelta
            p.size = MathConstants::randRange(spec.minSize, spec.maxSize);         // Dimensione casuale
            p.rotation = MathConstants::randRange(0.0f, 360.0f);                   // Orientamento iniziale [0, 360]
            p.rotSpeed = MathConstants::randRange(-spec.maxRotSpeed, spec.maxRotSpeed); // Spin casuale
            p.maxLife = MathConstants::randRange(spec.minLife, spec.maxLife);      // Durata vita
            p.life = p.maxLife;                                                    // Inizializza vita residua
            particles.push_back(p);                                                // Accoda nel pool attivo
        }
    }
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
    m_rockFragGeo.uploadMesh(fragMesh, false);

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

    for (const auto& p : m_particles) // scorre tutte le particelle attive
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
        // in pratica riduce gradualmente la trasparenza
        // progress = 1.0 -> alpha = 1.0, progress = 0.0 -> alpha = 0.0
        glm::vec4 c = p.color;
        c.a *= progress;

        // Imposta le uniform del colore e della matrice di modellazione.
        // invio delle uniform al shader -> GPU
        shader.setVec4("uColorTint", c);
        shader.setMat4("model", m);

        // Disegna il frammento triangolare.
        m_rockFragGeo.draw();
    }

    // Ripristina la tinta uniforme a bianco opaco (1.0).
    // rimette a bianco opaco (1,1,1,1)
    shader.setVec4("uColorTint", glm::vec4(1.0f));
}

// ============================================================================
// Emissione Detriti Esplosione Asteroide (Frantumazione in Mille Pezzi)
// ============================================================================
void ParticleSystem::emitAsteroidExplosion(const glm::vec2& origin, int count)
{
    const ParticleBurstSpec spec{ 2.0f, 18.0f, 80.0f, 320.0f, 10.0f, 24.0f, 350.0f, 0.5f, 1.1f };
    spawnRadialBurst(m_particles, origin, count, spec, []() {
        // Variazione cromatica dei frammenti: toni rocciosi e fiammate arancioni da impatto.
        float tone = MathConstants::randRange(0.4f, 0.9f);
        return glm::vec4(
            tone * MathConstants::randRange(0.9f, 1.2f),
            tone * MathConstants::randRange(0.6f, 0.9f),
            tone * MathConstants::randRange(0.4f, 0.6f),
            1.0f
        );
    });
}

// ============================================================================
// Emissione Scintille Dorate Raccolta Moneta
// ============================================================================
void ParticleSystem::emitCoinSparkles(const glm::vec2& origin, int count)
{
    const ParticleBurstSpec spec{ 0.0f, 0.0f, 60.0f, 200.0f, 6.0f, 14.0f, 180.0f, 0.4f, 0.75f };
    spawnRadialBurst(m_particles, origin, count, spec, []() {
        // Colore giallo oro scintillante.
        return glm::vec4(1.0f, MathConstants::randRange(0.8f, 1.0f), MathConstants::randRange(0.1f, 0.4f), 1.0f);
    });
}

// ============================================================================
// Emissione Coriandoli e Fuochi per "HURRÀ! Nuovo Livello"
// ============================================================================
void ParticleSystem::emitHurraCelebration(float screenWidth, float screenHeight, int count)
{
    // Palette cromatica festosa: oro brillante, ciano cosmico, magenta e verde smeraldo.
    static const glm::vec4 celebrationColors[] = {
        glm::vec4(1.0f, 0.85f, 0.1f, 1.0f),
        glm::vec4(0.2f, 0.95f, 1.0f, 1.0f),
        glm::vec4(1.0f, 0.25f, 0.7f, 1.0f),
        glm::vec4(0.25f, 1.0f, 0.4f, 1.0f)
    };
    const ParticleBurstSpec spec{ 0.0f, 0.0f, 120.0f, 380.0f, 12.0f, 26.0f, 280.0f, 1.0f, 1.8f };
    for (int i = 0; i < count; ++i)
    {
        // Ogni coriandolo nasce in un punto casuale nell'area centrale dello schermo.
        glm::vec2 pos(
            MathConstants::randRange(screenWidth * 0.15f, screenWidth * 0.85f),
            MathConstants::randRange(screenHeight * 0.3f, screenHeight * 0.7f)
        );
        spawnRadialBurst(m_particles, pos, 1, spec, []() {
            return celebrationColors[rand() % 4];
        });
    }
}

// ============================================================================
// Emissione Onde di Energia dalla Reliquia Spaziale
// ============================================================================
void ParticleSystem::emitRelicEnergy(const glm::vec2& origin, int count)
{
    // Tonalità eteree ciano, viola quantico e bianco energia.
    static const glm::vec4 relicColors[] = {
        glm::vec4(0.2f, 0.9f, 1.0f, 1.0f),
        glm::vec4(0.5f, 0.4f, 1.0f, 1.0f),
        glm::vec4(0.8f, 1.0f, 1.0f, 1.0f)
    };
    const ParticleBurstSpec spec{ 10.0f, 35.0f, 90.0f, 260.0f, 8.0f, 18.0f, 200.0f, 0.6f, 1.2f };
    spawnRadialBurst(m_particles, origin, count, spec, []() {
        return relicColors[rand() % 3];
    });
}

// ============================================================================
// Emissione Scintille Elettrificate all'Impatto Laser col Drone Rosso
// ============================================================================
void ParticleSystem::emitDroneHitSparks(const glm::vec2& origin, int count)
{
    // Scintille ad alto contrasto: cremisi, oro rovente e azzurro ionico da dispersione di campo.
    static const glm::vec4 sparkColors[] = {
        glm::vec4(1.0f, 0.2f, 0.35f, 1.0f),
        glm::vec4(1.0f, 0.85f, 0.2f, 1.0f),
        glm::vec4(0.3f, 0.9f, 1.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };
    const ParticleBurstSpec spec{ 4.0f, 18.0f, 70.0f, 260.0f, 7.0f, 16.0f, 320.0f, 0.4f, 0.85f };
    spawnRadialBurst(m_particles, origin, count, spec, []() {
        return sparkColors[rand() % 4];
    });
}
