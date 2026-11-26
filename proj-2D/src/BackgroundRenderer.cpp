// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: BackgroundRenderer.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Alloca e disegna un Fullscreen Quad (2 triangoli, 6 vertici) per lo sfondo cosmico.
//    - Esegue il rendering procedurale puro senza immagini o texture di memoria.
// 2. A CHI È COLLEGATO:
//    - Invocato da GameWorld all'inizio del render frame; pilota background.vert e frag.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - Coordinate NDC pure [-1, 1] x [-1, 1]: zero matrici Model e Projection (Domanda 9).
//    - Gestione risorse RAII e Rule of Five per buffer hardware OpenGL (Domanda 17).
//    - Shader procedurale SIMD: hash, parallasse stellare e domain warping (Domande 1, 3, 5).
// ============================================================================

#include "BackgroundRenderer.h"

// ============================================================================
// Costruttore: Allocazione del Fullscreen Quad in NDC
// ============================================================================
BackgroundRenderer::BackgroundRenderer(const Shader& shader)
    : m_shader(shader)
{
    // Coordinate dei 6 vertici (2 triangoli complanari) nello spazio NDC (Normalized Device Coordinates).
    // Coprono esattamente l'area visibile dello schermo da (-1, -1) a (+1, +1):
    // Triangolo 1: (-1,-1), (1,-1), (1,1)
    // Triangolo 2: (-1,-1), (1,1), (-1,1)
    // Non necessitano di trasformazione da matrice Model o Projection.
    const float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,

        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    // 1. Genera e lega il Vertex Array Object (VAO).
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // 2. Genera e lega il Vertex Buffer Object (VBO).
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // 3. Trasferisce i vertici nella memoria GPU in modalità GL_STATIC_DRAW.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 4. Configura l'attributo di posizione (location = 0):
    // 2 coordinate float per vertice (x, y), stride di 2 * sizeof(float).
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // 5. Unbind di sicurezza dei buffer.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// ============================================================================
// Distruttore: Rilascio Risorse GPU
// ============================================================================
BackgroundRenderer::~BackgroundRenderer()
{
    if (m_vbo != 0)
    {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }

    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

// ============================================================================
// Disegno dello Sfondo Procedurale con Animazione Temporale ed Effetti
// ============================================================================
void BackgroundRenderer::draw(float timeSeconds, float psychedelic, int level) const
{
    // 1. Attiva il programma shader dedicato al background cosmico.
    m_shader.use();

    // 2. Invia i parametri dinamici uniformi alla GPU:
    m_shader.setFloat("uTime", timeSeconds);
    m_shader.setFloat("uPsychedelic", psychedelic);
    m_shader.setInt("uLevel", level);

    // 3. Lega il VAO del quad ed emette la draw call per i 6 vertici (2 triangoli).
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 4. Ripristina il binding a 0.
    glBindVertexArray(0);
}
