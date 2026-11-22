// ============================================================================
// GUIDA RAPIDA PER L'ORALE - MODULO: Input (Gestione Eventi & Callback GLFW)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Gestisce la ricezione degli eventi periferici di sistema (tastiera, mouse, finestra).
//    - Fornisce le tre funzioni di callback statiche registrate presso GLFW:
//      * keyCallback: intercetta tasti singoli discreti (es. ESC per uscire, R per riavviare);
//      * cursorPositionCallback: traccia le coordinate cartesiane del mouse;
//      * framebufferSizeCallback: aggiorna la viewport OpenGL al ridimensionamento della finestra.
//    - Separa nettamente gli eventi discreti (callback) dalla cinematica continua (polling).
//
// 2. A CHI È COLLEGATA:
//    - Registrata da: main.cpp (tramite glfwSetKeyCallback, glfwSetCursorPosCallback, glfwSetFramebufferSizeCallback).
//    - Interagisce con: GameWorld e Player (che leggono lo stato continuo tramite glfwGetKey).
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Polling vs Callback con GLFW (Domanda 19):
//      * Callback: funzione asincrona invocata dal driver OS quando si verifica un evento discreto
//        (durante glfwPollEvents). Ideale per cambi di stato, menu, restart o sparo singolo.
//      * Polling: interrogazione sincrona attiva ad ogni frame (glfwGetKey). Ideale per il movimento
//        continuo della navicella integrato con la fisica del delta time: p(t+dt) = p(t) + v*dt.
//    - Trasformazione di Viewport su Resize (Domanda 9):
//      framebufferSizeCallback invoca glViewport(0, 0, width, height), aggiornando la matrice
//      di trasformazione da coordinate NDC [-1, 1] ai pixel effettivi del framebuffer del display.
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
