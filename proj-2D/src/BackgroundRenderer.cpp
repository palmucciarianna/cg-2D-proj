// ============================================================================
// File: src/BackgroundRenderer.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del BackgroundRenderer.
//              Alloca due triangoli che coprono l'intervallo [-1.0, +1.0] su X e Y.
//              Il fragment shader calcola un campo stellare con scintillio pseudo-casuale
//              e gradienti di nebulosa dipendenti dal tempo.
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
// Disegno dello Sfondo Procedurale con Animazione Temporale
// ============================================================================
void BackgroundRenderer::draw(float timeSeconds) const
{
    // 1. Attiva il programma shader dedicato al background cosmico.
    m_shader.use();

    // 2. Recupera l'handle del programma attivo e cerca la uniform "uTime".
    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    const GLint timeLocation = glGetUniformLocation(static_cast<GLuint>(currentProgram), "uTime");

    // 3. Se la uniform è presente, vi invia il tempo corrente in secondi.
    // Il fragment shader utilizzerà questo valore per far pulsare le stelle e muovere la nebulosa.
    if (timeLocation >= 0)
    {
        glUniform1f(timeLocation, timeSeconds);
    }

    // 4. Lega il VAO del quad ed emette la draw call per i 6 vertici (2 triangoli).
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 5. Ripristina il binding a 0.
    glBindVertexArray(0);
}
