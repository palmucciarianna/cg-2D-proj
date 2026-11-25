// ============================================================================
// File: src/Shader.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione della classe Shader.
//              Incapsula le chiamate OpenGL glGetUniformLocation e glUniform*
//              garantendo la corretta associazione dei parametri shader da C++.
// ============================================================================

#include "Shader.h"
#include "ShaderMaker.h"
#include <glm/gtc/type_ptr.hpp>

// ============================================================================
// Distruttore
// ============================================================================
Shader::~Shader()
{
    // Rilascia le risorse GPU quando l'oggetto C++ esce dallo scope.
    destroy();
}

// ============================================================================
// Caricamento e Compilazione dello Shader
// ============================================================================
bool Shader::load(const char* vertexShaderFile, const char* fragmentShaderFile)
{
    // Se esisteva già un programma compilato in precedenza, lo rilascia.
    destroy();

    // Delega la lettura, compilazione e linking alla classe di utilità ShaderMaker.
    m_programId = ShaderMaker::createProgram(vertexShaderFile, fragmentShaderFile);

    // Se m_programId è diverso da zero, il caricamento è avvenuto con successo.
    return m_programId != 0;
}

// ============================================================================
// Attivazione del Programma Shader nella Pipeline
// ============================================================================
void Shader::use() const
{
    // glUseProgram imposta questo programma shader come attivo per tutte le successive draw call.
    glUseProgram(m_programId);
}

// ============================================================================
// Impostazione di una Matrice 4x4 (Uniform mat4)
// ============================================================================
void Shader::setMat4(const char* name, const glm::mat4& matrix) const
{
    // Se lo shader non è valido, interrompe l'operazione.
    if (m_programId == 0) return;

    // Recupera l'indice di memoria (location) della variabile uniform all'interno dello shader linkato.
    GLint location = glGetUniformLocation(m_programId, name);

    // Se la uniform non esiste nello shader (o è stata ottimizzata via dal compilatore), esce silenziosamente.
    if (location == -1) return;

    // glUniformMatrix4fv trasferisce una matrice 4x4 di float alla GPU:
    // Parametri:
    // - location: indice della variabile uniform
    // - count: 1 (stiamo inviando una singola matrice, non un array)
    // - transpose: GL_FALSE (GLM memorizza le matrici in formato column-major come richiesto da OpenGL)
    // - value: puntatore ai 16 float consecutivi della matrice restituito da glm::value_ptr
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

// ============================================================================
// Impostazione di un Vettore 4D (Uniform vec4)
// ============================================================================
void Shader::setVec4(const char* name, const glm::vec4& value) const
{
    if (m_programId == 0) return;

    GLint location = glGetUniformLocation(m_programId, name);
    if (location != -1)
    {
        // glUniform4fv trasferisce 4 valori float consecutivi (x, y, z, w oppure r, g, b, a).
        glUniform4fv(location, 1, glm::value_ptr(value));
    }
}

// ============================================================================
// Impostazione di uno Scalare Float (Uniform float)
// ============================================================================
void Shader::setFloat(const char* name, float value) const
{
    if (m_programId == 0) return;

    GLint location = glGetUniformLocation(m_programId, name);
    if (location != -1)
    {
        // glUniform1f trasferisce un singolo valore in virgola mobile a 32 bit.
        glUniform1f(location, value);
    }
}

// ============================================================================
// Impostazione di un Intero (Uniform int)
// ============================================================================
void Shader::setInt(const char* name, int value) const
{
    if (m_programId == 0) return;

    GLint location = glGetUniformLocation(m_programId, name);
    if (location != -1)
    {
        // glUniform1i trasferisce un valore intero a 32 bit (usato per legare le texture units GL_TEXTURE0, ecc.).
        glUniform1i(location, value);
    }
}

// ============================================================================
// Deallocazione delle Risorse del Programma Shader
// ============================================================================
void Shader::destroy()
{
    // Se il programma è valido, ne richiede l'eliminazione alla GPU.
    if (m_programId != 0)
    {
        glDeleteProgram(m_programId);
        m_programId = 0; // Azzera l'handle per evitare dangling handles
    }
}
