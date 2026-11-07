// ============================================================================
// File: include/Shader.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Classe C++ orientata agli oggetti (wrapper RAII) per la gestione
//              del ciclo di vita dei programmi shader OpenGL 3.3.
//              Fornisce metodi helper per l'impostazione rapida delle variabili Uniform
//              (matrici 4x4, vettori 4D, scalari float e interi per texture sampler).
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    // Costruttore predefinito: inizializza l'handle del programma a 0 (nessuno shader attivo).
    Shader() = default;

    // Distruttore: dealloca automaticamente il programma shader hardware chiamando destroy().
    ~Shader();

    // Carica, compila e linka il Vertex Shader e il Fragment Shader dati i rispettivi percorsi su disco.
    // Restituisce true in caso di esito positivo, false in caso di errore.
    bool load(const char* vertexShaderFile, const char* fragmentShaderFile);

    // Attiva questo shader per le successive operazioni di disegno (invoca glUseProgram).
    void use() const;

    // Metodi per l'invio delle variabili Uniform alla GPU:
    // - setMat4: invia una matrice 4x4 (es. Projection o Model Matrix)
    void setMat4(const char* name, const glm::mat4& matrix) const;
    // - setVec4: invia un vettore a 4 dimensioni (es. colore RGBA)
    void setVec4(const char* name, const glm::vec4& value) const;
    // - setFloat: invia un valore in virgola mobile a 32 bit (es. tempo 'u_time' o opacità 'u_alpha')
    void setFloat(const char* name, float value) const;
    // - setInt: invia un intero a 32 bit (es. unità texture per i sampler2D o flag booleani)
    void setInt(const char* name, int value) const;

    // Restituisce l'identificatore numerico del programma OpenGL.
    GLuint id() const { return m_programId; }

    // Rilascia le risorse GPU associate al programma shader azzerando l'handle.
    void destroy();

private:
    // Identificatore univoco del programma shader linkato sulla GPU.
    GLuint m_programId = 0;
};
