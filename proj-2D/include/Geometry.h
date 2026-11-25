// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: Geometry (Incapsulamento Risorse GPU)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Incapsula e gestisce il ciclo di vita delle risorse hardware geometriche della GPU:
//      * VAO (Vertex Array Object): memorizza la configurazione degli attributi;
//      * VBO Positions (Vertex Buffer Object): coordinate spaziali (x, y, z);
//      * VBO Colors: attributi colore per vertice (r, g, b, a).
//    - Carica i dati in VRAM (upload()) ed emette la chiamata di disegno (draw() -> glDrawArrays).
//    - Implementa rigorosamente l'idioma RAII e la Rule of Five (move semantics abilitate, copia vietata).
//
// 2. A CHI È COLLEGATA:
//    - Utilizzata da praticamente tutte le entità del motore grafico:
//      Player, AsteroidManager, CollectibleManager, HermiteManager, LaserManager, HazardManager, ParticleSystem.
//    - Riceve i dati CPU da: MeshData (creata da ShapeGenerator o da PlayerShapeLoader).
//    - Interagisce direttamente con OpenGL Core 3.3 (GLAD) e gli shader GLSL via location di attributo.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Ruolo di VBO vs VAO (Requisito 4 & Domanda 17):
//      * VBO: contiene l'array grezzo di byte memorizzato in memoria VRAM della GPU (glBufferData);
//      * VAO: memorizza lo "stato" e il formato dei dati (glVertexAttribPointer + glEnableVertexAttribArray).
//        Il VAO non contiene vertici di per sé, ma la "ricetta" con cui la GPU legge il VBO.
//    - Mapping Attributi nello Shader (Domande 17, 18):
//      Location 0: vec3 aPosition -> 3 float, stride = sizeof(glm::vec3);
//      Location 1: vec4 aColor    -> 4 float, stride = sizeof(glm::vec4).
//    - Primitiva e Draw Call (Domanda 17):
//      glBindVertexArray(m_vao);  glDrawArrays(m_primitiveType, 0, m_vertexCount);
//    - Gestione della Memoria RAII (Requisito 8):
//      I buffer vengono deallocati nel distruttore con glDeleteVertexArrays e glDeleteBuffers,
//      azzerando qualsiasi possibilità di memory leak sulla GPU.
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct MeshData;

class Geometry
{
public:
    // Costruttore predefinito: inizializza una geometria vuota non allocata su GPU.
    Geometry() = default;

    // Regola dei Cinque (Rule of Five) e gestione delle risorse GPU:
    // Una geometria possiede identificatori OpenGL hardware univoci (VAO, VBO).
    // Disabilitiamo il costruttore di copia e l'operatore di assegnazione per copia
    // per impedire duplicazioni accidentali di handle GPU che causerebbero doppie deallocazioni (double free).
    Geometry(const Geometry&) = delete;
    Geometry& operator=(const Geometry&) = delete;

    // Abilitiamo invece il costruttore e l'operatore di assegnazione per movimento (move semantics).
    // Trasferiscono la proprietà degli handle OpenGL da un oggetto temporaneo all'altro a costo zero.
    Geometry(Geometry&& other) noexcept;
    Geometry& operator=(Geometry&& other) noexcept;

    // Distruttore virtuale/standard: rilascia automaticamente la memoria GPU chiamando destroy().
    ~Geometry();

    // ========================================================================
    // Gestione dei Dati lato CPU
    // ========================================================================

    // Popola i vettori CPU delle posizioni e dei colori e memorizza la primitiva da usare per il disegno.
    void setData(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::vec4>& colors,
        GLenum primitiveType
    );

    // ========================================================================
    // Trasferimento Dati CPU -> GPU (Buffer Allocation)
    // ========================================================================

    // Alloca e invia i dati dalla memoria RAM alla VRAM della GPU.
    // - dynamic = false: utilizza GL_STATIC_DRAW (i dati non cambieranno spesso, massime prestazioni).
    // - dynamic = true:  utilizza GL_DYNAMIC_DRAW (i dati cambieranno ogni frame, es. particelle o curve animate).
    void upload(bool dynamic = false);

    // Helper compatto: copia una MeshData CPU e la carica subito su GPU (setData + upload).
    void uploadMesh(const MeshData& data, bool dynamic = false);

    // Aggiorna solo il buffer delle posizioni tramite glBufferSubData senza riallocare il buffer GPU.
    void updatePositions(const std::vector<glm::vec3>& newPositions);

    // Aggiorna solo il buffer dei colori tramite glBufferSubData.
    void updateColors(const std::vector<glm::vec4>& newColors);

    // ========================================================================
    // Pipeline di Rendering
    // ========================================================================

    // Esegue il binding del Vertex Array Object (VAO) ed emette la draw call glDrawArrays.
    void draw() const;

    // ========================================================================
    // Metodi di Accesso e Ispezione (Getters / Setters)
    // ========================================================================

    // Accesso in scrittura/lettura ai vettori CPU.
    std::vector<glm::vec3>& positions() { return m_positions; }
    std::vector<glm::vec4>& colors() { return m_colors; }

    // Accesso a sola lettura (costante).
    const std::vector<glm::vec3>& positions() const { return m_positions; }
    const std::vector<glm::vec4>& colors() const { return m_colors; }

    // Restituisce il numero totale di vertici presenti nella geometria.
    int vertexCount() const { return static_cast<int>(m_positions.size()); }

    // Assegna e legge la matrice di modellazione locale M (Model Matrix 4x4).
    void setModelMatrix(const glm::mat4& model);
    const glm::mat4& modelMatrix() const;

    // Rilascia esplicitamente tutte le risorse GPU (VAO e VBO) azzerando gli handle.
    void destroy();

private:
    // Dati dei vertici residenti nella memoria principale (RAM).
    std::vector<glm::vec3> m_positions; // Coordinate geometriche 3D (X, Y, Z)
    std::vector<glm::vec4> m_colors;    // Componenti di colore RGBA normalizzate in [0, 1]

    // Identificatori univoci (handle) delle risorse residenti nella memoria video (VRAM).
    GLuint m_VAO = 0;           // Vertex Array Object: memorizza lo stato delle configurazioni degli attributi
    GLuint m_VBO_positions = 0; // Vertex Buffer Object per le coordinate spaziali
    GLuint m_VBO_colors = 0;    // Vertex Buffer Object per i colori dei vertici

    // Tipo di primitiva geometrica OpenGL (GL_TRIANGLES, GL_TRIANGLE_FAN, GL_LINE_STRIP, ecc.)
    GLenum m_primitiveType = GL_TRIANGLES;

    // Flag booleano che indica se la geometria è attualmente caricata e pronta sulla GPU.
    bool m_uploaded = false;

    // Matrice di trasformazione di modellazione associata all'oggetto geometrico.
    glm::mat4 m_model = glm::mat4(1.0f);
};
