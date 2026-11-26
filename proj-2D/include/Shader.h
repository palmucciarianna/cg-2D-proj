// ORIGINE: Gestione Shader Program e Uniform dai laboratori della prof, incapsulata in classe C++ RAII.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - CLASSE: Shader (Gestione Shader Program GLSL)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Incapsula e gestisce l'handle OpenGL dello Shader Program (`m_programId`).
//    - Carica, compila e collega a runtime il Vertex Shader e il Fragment Shader (Requisito 5).
//    - Fornisce i metodi setter per inviare variabili Uniform dalla CPU alla GPU:
//      * setMat4: invia matrici Model e Projection (glUniformMatrix4fv);
//      * setVec4: invia tinte e colori modulati (glUniform4fv);
//      * setFloat / setInt: invia parametri dinamici come 'uTime', 'uPsychedelic', 'uLevel'.
//    - Attiva il programma nel pipeline state con glUseProgram(m_programId).
//
// 2. A CHI È COLLEGATA:
//    - Creata e conservata da: GameWorld (m_bgShader, m_basicShader).
//    - Invocata per il disegno da: BackgroundRenderer, Player, AsteroidManager, CollectibleManager, HermiteManager.
//    - Collabora con: ShaderMaker (modulo che legge i file sorgente GLSL e controlla gli errori di compilazione).
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Pipeline Programmabile & Ruolo degli Shader (Domande 3, 4, 18):
//      Sostituisce i vecchi algoritmi a funzione fissa con programmi GLSL eseguibili sui core della GPU:
//      * Vertex Shader: riceve attributi dai VBO, calcola gl_Position = P * M * aPosition;
//      * Fragment Shader: calcola il colore finale per ogni campione/frammento rasterizzato (FragColor).
//    - Concetto di Variabile Uniform (Domanda 18):
//      Valore costante comune per tutti i vertici e frammenti di una singola draw call.
//      Viene inviato dalla CPU prima del disegno (es. glUniformMatrix4fv).
//    - Gestione delle Risorse RAII (Requisito 8):
//      Il distruttore invoca glDeleteProgram(m_programId), rilasciando le risorse GPU.
// ============================================================================

#pragma once

#include "OpenGL_Headers.h"
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
    // - setVec2: invia un vettore a 2 dimensioni (es. coordinate UV o offset)
    void setVec2(const char* name, const glm::vec2& value) const;
    // - setFloat: invia un valore in virgola mobile a 32 bit (es. tempo 'u_time' o opacità 'u_alpha')
    void setFloat(const char* name, float value) const;
    // - setInt: invia un intero a 32 bit (es. unità texture per i sampler2D o flag booleani)
    void setInt(const char* name, int value) const;
    // - setBool: invia un flag booleano come intero 0 o 1
    void setBool(const char* name, bool value) const { setInt(name, value ? 1 : 0); }

    // Restituisce l'identificatore numerico del programma OpenGL.
    GLuint id() const { return m_programId; }

    // Rilascia le risorse GPU associate al programma shader azzerando l'handle.
    void destroy();

private:
    // Identificatore univoco del programma shader linkato sulla GPU.
    GLuint m_programId = 0;
};
