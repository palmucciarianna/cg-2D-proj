// ============================================================================
// File: src/ShaderMaker.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del caricatore e compilatore di shader GLSL.
//              Include il controllo degli errori di compilazione (glGetShaderiv)
//              e degli errori di linking del programma eseguibile (glGetProgramiv).
// ============================================================================

#include "OpenGL_Headers.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "ShaderMaker.h"

// Disabilita i warning di deprecazione fopen su compilatori MSVC Windows.
#pragma warning(disable:4996)

#ifdef __EMSCRIPTEN__
// Adatta gli header GLSL 3.30 Core alle specifiche WebGL 2.0 / OpenGL ES 3.0
static std::string adaptShaderForWebGL(const char* source)
{
    if (!source) return "";
    std::string s(source);
    size_t pos = s.find("#version 330");
    if (pos != std::string::npos)
    {
        size_t endLine = s.find('\n', pos);
        if (endLine != std::string::npos)
        {
            s.replace(pos, endLine - pos + 1, "#version 300 es\nprecision highp float;\nprecision highp int;\n");
        }
    }
    return s;
}
#endif

// ============================================================================
// Lettura del Codice Sorgente dello Shader da File di Testo
// ============================================================================
char* ShaderMaker::readShaderSource(const char* shaderFile)
{
    // Apre il file specificato in modalità binaria di sola lettura.
    FILE* fp = fopen(shaderFile, "rb");
    if (fp == nullptr)
    {
        std::cerr << "[ERRORE] Impossibile aprire il file shader: " << shaderFile << std::endl;
        return nullptr;
    }

    // Sposta il cursore del file alla fine per determinarne la lunghezza in byte.
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);

    // Riporta il cursore all'inizio del file.
    fseek(fp, 0L, SEEK_SET);

    // Alloca un buffer in memoria per contenere l'intero file più il terminatore nullo '\0'.
    char* buf = new char[size + 1];

    // Legge 'size' elementi da 1 byte ciascuno inserendoli nel buffer.
    fread(buf, 1, size, fp);

    // Inserisce il terminatore di stringa C-style alla fine del buffer.
    buf[size] = '\0';

    // Chiude il descrittore del file su disco.
    fclose(fp);

    // Restituisce il puntatore al buffer di testo contenente il codice GLSL.
    return buf;
}

// ============================================================================
// Compilazione e Linking del Programma Shader Completo
// ============================================================================
GLuint ShaderMaker::createProgram(const char* vertexfilename, const char* fragmentfilename)
{
    int success;
    char infoLog[512];

    // ------------------------------------------------------------------------
    // 1. Compilazione del Vertex Shader
    // ------------------------------------------------------------------------
    // Legge il codice sorgente del vertex shader dal file su disco.
    char* vertexSource = readShaderSource(vertexfilename);
    if (!vertexSource)
    {
        std::cerr << "[ERRORE] File Vertex Shader non trovato: " << vertexfilename << std::endl;
        return 0;
    }

    // Richiede a OpenGL di allocare un nuovo oggetto shader di tipo GL_VERTEX_SHADER.
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);

    // Associa la stringa di testo GLSL all'oggetto shader appena allocato.
#ifdef __EMSCRIPTEN__
    std::string webglVert = adaptShaderForWebGL(vertexSource);
    const char* vSrc = webglVert.c_str();
    glShaderSource(vertexShaderId, 1, &vSrc, nullptr);
#else
    glShaderSource(vertexShaderId, 1, (const char**)&vertexSource, nullptr);
#endif

    // Avvia la compilazione del codice sorgente nel microcodice GPU per il vertex stage.
    glCompileShader(vertexShaderId);

    // Rilascia la memoria RAM allocata per il buffer di testo del vertex shader.
    delete[] vertexSource;

    // Interroga lo stato di compilazione del vertex shader.
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // Se la compilazione è fallita, estrae il log testuale dell'errore restituito dal driver GPU.
        glGetShaderInfoLog(vertexShaderId, 512, nullptr, infoLog);
        std::cerr << "[ERRORE] Compilazione Vertex Shader Fallita (" << vertexfilename << "):\n" 
                  << infoLog << std::endl;
        glDeleteShader(vertexShaderId);
        return 0;
    }

    // ------------------------------------------------------------------------
    // 2. Compilazione del Fragment Shader
    // ------------------------------------------------------------------------
    // Legge il codice sorgente del fragment shader dal file su disco.
    char* fragmentSource = readShaderSource(fragmentfilename);
    if (!fragmentSource)
    {
        std::cerr << "[ERRORE] File Fragment Shader non trovato: " << fragmentfilename << std::endl;
        glDeleteShader(vertexShaderId);
        return 0;
    }

    // Alloca l'oggetto shader di tipo GL_FRAGMENT_SHADER.
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Carica il testo sorgente del fragment shader.
#ifdef __EMSCRIPTEN__
    std::string webglFrag = adaptShaderForWebGL(fragmentSource);
    const char* fSrc = webglFrag.c_str();
    glShaderSource(fragmentShaderId, 1, &fSrc, nullptr);
#else
    glShaderSource(fragmentShaderId, 1, (const char**)&fragmentSource, nullptr);
#endif

    // Compila il fragment shader.
    glCompileShader(fragmentShaderId);

    // Rilascia la memoria RAM del buffer di testo.
    delete[] fragmentSource;

    // Interroga lo stato di compilazione del fragment shader.
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, 512, nullptr, infoLog);
        std::cerr << "[ERRORE] Compilazione Fragment Shader Fallita (" << fragmentfilename << "):\n" 
                  << infoLog << std::endl;
        glDeleteShader(vertexShaderId);
        glDeleteShader(fragmentShaderId);
        return 0;
    }

    // ------------------------------------------------------------------------
    // 3. Creazione del Programma ed Esecuzione del Linking
    // ------------------------------------------------------------------------
    // Alloca un nuovo programma eseguibile OpenGL (pipeline completa).
    GLuint programId = glCreateProgram();

    // Collega il vertex shader compilato al programma.
    glAttachShader(programId, vertexShaderId);
    // Collega il fragment shader compilato al programma.
    glAttachShader(programId, fragmentShaderId);

    // Esegue il linking unendo i due stadi della pipeline e risolvendo le variabili 'out'/'in'.
    glLinkProgram(programId);

    // Verifica l'esito del linking del programma.
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        std::cerr << "[ERRORE] Linking dello Shader Program Fallito:\n" << infoLog << std::endl;
        glDeleteProgram(programId);
        glDeleteShader(vertexShaderId);
        glDeleteShader(fragmentShaderId);
        return 0;
    }

    // Una volta linkati nel programma eseguibile, i singoli oggetti shader intermedi
    // possono essere contrassegnati per l'eliminazione: verranno deallocati non appena
    // il programma verrà distrutto o scollegato.
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    // Restituisce l'identificatore del programma shader compilato e pronto all'uso.
    return programId;
}
