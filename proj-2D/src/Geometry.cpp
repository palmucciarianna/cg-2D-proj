// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: Geometry.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Incapsula e gestisce le risorse OpenGL su VRAM: VAO, VBO posizioni, VBO colori.
//    - Trasferisce i dati da CPU a GPU con upload() ed emette il disegno con draw().
// 2. A CHI È COLLEGATO:
//    - Riceve i vertici da MeshData; utilizzata da tutte le entità per il rendering.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - VAO registra lo stato degli attributi (glVertexAttribPointer) (Domanda 17).
//    - VBO memorizza fisicamente i dati in memoria VRAM (glBufferData) (Domande 1, 17).
//    - Gestione RAII con glDeleteVertexArrays e glDeleteBuffers nel distruttore (Domanda 16).
// ============================================================================

#include "Geometry.h"
#include "MeshData.h"
#include <utility>

// ============================================================================
// Costruttore di Movimento (Move Constructor)
// ============================================================================
Geometry::Geometry(Geometry&& other) noexcept
{
    // Delega il trasferimento all'operatore di assegnazione per movimento.
    *this = std::move(other);
}

// ============================================================================
// Operatore di Assegnazione per Movimento (Move Assignment Operator)
// ============================================================================
Geometry& Geometry::operator=(Geometry&& other) noexcept
{
    // Verifica l'auto-assegnazione (es. a = std::move(a)).
    if (this != &other)
    {
        // 1. Distrugge eventuali risorse GPU attualmente allocate da questa istanza.
        destroy();

        // 2. Sposta i vettori CPU residenti in RAM (senza riallocazioni, scambio O(1) di puntatori).
        m_positions = std::move(other.m_positions);
        m_colors = std::move(other.m_colors);

        // 3. Ruba gli identificatori GPU (handle hardware) dall'oggetto sorgente.
        m_VAO = other.m_VAO;
        m_VBO_positions = other.m_VBO_positions;
        m_VBO_colors = other.m_VBO_colors;

        // 4. Copia i parametri geometrici e di stato.
        m_primitiveType = other.m_primitiveType;
        m_uploaded = other.m_uploaded;
        m_model = other.m_model;

        // 5. Invalida l'oggetto sorgente 'other' per evitare che il suo distruttore
        //    deallochi le risorse OpenGL appena trasferite.
        other.m_VAO = 0;
        other.m_VBO_positions = 0;
        other.m_VBO_colors = 0;
        other.m_uploaded = false;
    }

    return *this;
}

// ============================================================================
// Distruttore (RAII - Resource Acquisition Is Initialization)
// ============================================================================
Geometry::~Geometry()
{
    // Rilascia automaticamente la memoria video alla distruzione dell'oggetto C++.
    destroy();
}

// ============================================================================
// Assegnazione dei Dati CPU
// ============================================================================
void Geometry::setData(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec4>& colors,
    GLenum primitiveType)
{
    // Copia i vettori di vertici e colori nella memoria locale dell'istanza.
    m_positions = positions;
    m_colors = colors;
    m_primitiveType = primitiveType;
}

// ============================================================================
// Helper Compatto: setData + upload in un'unica chiamata
// ============================================================================
void Geometry::uploadMesh(const MeshData& data, bool dynamic)
{
    setData(data.positions, data.colors, data.primitiveType); // Copia i vertici e i colori CPU
    upload(dynamic);                                          // Alloca e invia i VAO/VBO in VRAM
}

// ============================================================================
// Caricamento Dati dalla CPU alla Memoria Video GPU (Upload)
// ============================================================================
void Geometry::upload(bool dynamic)
{
    // Se la geometria possedeva già buffer su GPU, li deallochiamo per sicurezza.
    destroy();

    // Sceglie la modalità di allocazione per il driver grafico:
    // - GL_DYNAMIC_DRAW: comunica alla GPU che i dati verranno riscritti frequentemente.
    // - GL_STATIC_DRAW: comunica alla GPU che i dati rimarranno immutati (ottimizzazione in VRAM).
    const GLenum usage = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    // ------------------------------------------------------------------------
    // 1. Generazione e Binding del Vertex Array Object (VAO)
    // ------------------------------------------------------------------------
    // Il VAO incapsula e "registra" tutti i binding dei VBO e i formati degli attributi.
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // ------------------------------------------------------------------------
    // 2. Creazione del VBO per le Posizioni (Vertex Coordinate Buffer)
    // ------------------------------------------------------------------------
    // Alloca 1 handle per il buffer delle posizioni.
    glGenBuffers(1, &m_VBO_positions);
    // Lega il buffer come array corrente per i vertici.
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO_positions);
    // Invia i byte dei vertici dalla RAM di sistema alla VRAM della scheda video.
    // Parametri: target, dimensione in byte totale, puntatore ai byte raw, modalità di utilizzo.
    glBufferData(
        GL_ARRAY_BUFFER,
        m_positions.size() * sizeof(glm::vec3),
        m_positions.data(),
        usage
    );

    // Definisce la struttura dell'attributo posizione nel vertex shader (location = 0):
    // Parametri:
    // - index: 0 (corrisponde a layout (location = 0) in in_Position nello shader)
    // - size: 3 (ogni vertice ha 3 coordinate: x, y, z)
    // - type: GL_FLOAT (ogni componente è un float a 32 bit)
    // - normalized: GL_FALSE (i valori non devono essere normalizzati in [0, 1])
    // - stride: sizeof(glm::vec3) (distanza in byte tra un vertice e il successivo)
    // - pointer: offset 0 dall'inizio del buffer
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );
    // Abilita l'attributo al canale 0 nel VAO corrente.
    glEnableVertexAttribArray(0);

    // ------------------------------------------------------------------------
    // 3. Creazione del VBO per i Colori (Vertex Color Buffer)
    // ------------------------------------------------------------------------
    // Alloca 1 handle per il buffer dei colori RGBA.
    glGenBuffers(1, &m_VBO_colors);
    // Lega il buffer come array corrente.
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO_colors);
    // Invia i dati di colore alla memoria GPU.
    glBufferData(
        GL_ARRAY_BUFFER,
        m_colors.size() * sizeof(glm::vec4),
        m_colors.data(),
        usage
    );

    // Definisce la struttura dell'attributo colore nel vertex shader (location = 1):
    // Parametri:
    // - index: 1 (corrisponde a layout (location = 1) in in_Color nello shader)
    // - size: 4 (4 componenti: R, G, B, A)
    // - type: GL_FLOAT (float a 32 bit)
    // - normalized: GL_FALSE
    // - stride: sizeof(glm::vec4)
    // - pointer: offset 0
    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec4),
        (void*)0
    );
    // Abilita l'attributo al canale 1 nel VAO corrente.
    glEnableVertexAttribArray(1);

    // Scollega il VAO per evitare modifiche accidentali dello stato (Unbind).
    glBindVertexArray(0);

    // Contrassegna la geometria come caricata e pronta al rendering.
    m_uploaded = true;
}

// ============================================================================
// Aggiornamento Parziale delle Posizioni (Dynamic Buffer Streaming)
// ============================================================================
void Geometry::updatePositions(const std::vector<glm::vec3>& newPositions)
{
    // Aggiorna la copia locale in RAM.
    m_positions = newPositions;

    // Se la geometria non è caricata su GPU, non c'è buffer da aggiornare.
    if (!m_uploaded)
        return;

    // Lega il VBO delle posizioni.
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO_positions);

    // glBufferSubData sovrascrive i dati esistenti senza riallocare la memoria GPU.
    // È significativamente più efficiente di glBufferData per geometrie animate a ogni frame.
    // Parametri: target, offset di partenza in byte (0), dimensione in byte, puntatore dati.
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        m_positions.size() * sizeof(glm::vec3),
        m_positions.data()
    );
}

// ============================================================================
// Aggiornamento Parziale dei Colori (Dynamic Color Streaming)
// ============================================================================
void Geometry::updateColors(const std::vector<glm::vec4>& newColors)
{
    // Aggiorna la copia locale in RAM.
    m_colors = newColors;

    if (!m_uploaded)
        return;

    // Lega il VBO dei colori.
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO_colors);

    // Sovrascrive i dati del colore in VRAM a partire dall'offset 0.
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        m_colors.size() * sizeof(glm::vec4),
        m_colors.data()
    );
}

// ============================================================================
// Esecuzione del Disegno Grafico (Draw Call)
// ============================================================================
void Geometry::draw() const
{
    // Se la geometria non è su GPU, non è possibile disegnarla.
    if (!m_uploaded)
        return;

    // Lega il VAO: OpenGL ripristina istantaneamente tutte le associazioni ai VBO
    // e le configurazioni degli attributi registrate durante l'upload().
    glBindVertexArray(m_VAO);

    // Emette la draw call che invia i vertici alla pipeline di rasterizzazione.
    // Parametri:
    // - mode: tipo di primitiva (es. GL_TRIANGLES, GL_TRIANGLE_FAN, GL_LINE_STRIP)
    // - first: indice del primo vertice da cui iniziare il rendering (0)
    // - count: numero totale di vertici da rasterizzare
    glDrawArrays(
        m_primitiveType,
        0,
        vertexCount()
    );
}

// ============================================================================
// Deallocazione delle Risorse Hardware OpenGL
// ============================================================================
void Geometry::destroy()
{
    // Se il buffer delle posizioni esiste, lo rilasciamo sulla scheda video.
    if (m_VBO_positions != 0)
    {
        glDeleteBuffers(1, &m_VBO_positions);
        m_VBO_positions = 0; // Azzera l'handle per evitare dangling handles
    }

    // Se il buffer dei colori esiste, lo rilasciamo.
    if (m_VBO_colors != 0)
    {
        glDeleteBuffers(1, &m_VBO_colors);
        m_VBO_colors = 0;
    }

    // Se il VAO esiste, lo deallochiamo.
    if (m_VAO != 0)
    {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }

    // Segna la geometria come non presente in memoria video.
    m_uploaded = false;
}

// ============================================================================
// Gestione della Matrice di Trasformazione Model
// ============================================================================
void Geometry::setModelMatrix(const glm::mat4& model)
{
    m_model = model;
}

const glm::mat4& Geometry::modelMatrix() const
{
    return m_model;
}
