// ============================================================================
// File: include/Input.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Dichiarazione delle funzioni di callback per il sistema di input GLFW.
//              Include la gestione di tastiera, posizione del cursore e ridimensionamento
//              del framebuffer, incapsulate in un namespace dedicato senza variabili globali.
// ============================================================================

#pragma once

// Disabilita l'inclusione automatica di gl.h da parte di GLFW,
// poiché tutte le funzioni OpenGL sono caricate da GLAD.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Input
{
    // Callback invocata da GLFW quando un tasto della tastiera cambia stato (premuto, rilasciato, ripetuto).
    // Parametri:
    // - window: puntatore alla finestra GLFW che ha ricevuto l'evento.
    // - key: codice del tasto premuto (es. GLFW_KEY_ESCAPE).
    // - scancode: codice hardware specifico della piattaforma per il tasto.
    // - action: tipo di azione (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT).
    // - mods: maschera di bit per tasti modificatori attivi (Shift, Ctrl, Alt).
    void keyCallback(
        GLFWwindow* window,
        int key,
        int scancode,
        int action,
        int mods
    );

    // Callback invocata da GLFW ad ogni movimento del mouse all'interno dell'area della finestra.
    // Parametri:
    // - window: puntatore alla finestra GLFW.
    // - xpos: coordinata X del cursore (origine in alto a sinistra).
    // - ypos: coordinata Y del cursore (origine in alto a sinistra).
    void cursorPositionCallback(
        GLFWwindow* window,
        double xpos,
        double ypos
    );

    // Callback invocata da GLFW quando il buffer di rendering viene ridimensionato dall'utente o dal sistema.
    // Parametri:
    // - window: puntatore alla finestra GLFW.
    // - width: nuova larghezza del framebuffer in pixel effettivi.
    // - height: nuova altezza del framebuffer in pixel effettivi.
    void framebufferSizeCallback(
        GLFWwindow* window,
        int width,
        int height
    );
}
