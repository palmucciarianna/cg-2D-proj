// ORIGINE: Gestione input e callback GLFW basata sul pattern dei laboratori della prof, incapsulata in classe.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: Input.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Implementa le callback di sistema GLFW (keyCallback, cursorPositionCallback, framebufferSizeCallback).
// 2. A CHI È COLLEGATO:
//    - Registrato nel main.cpp; aggiorna la viewport OpenGL e notifica eventi discreti.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - Separazione eventi discreti (callback) da eventi continui (polling con glfwGetKey) (Domanda 19).
//    - Aggiornamento Viewport: glViewport(0, 0, width, height) mappa NDC [-1, 1] a pixel finestra (Domanda 9).
// ============================================================================

#include <glad/glad.h>
#include "Input.h"

namespace Input
{
    // Variabili statiche locali per memorizzare le coordinate del cursore del mouse.
    // L'uso di 'static' a livello di file ne confina la visibilità a questa unità di compilazione,
    // garantendo zero variabili globali esportabili in conformità alle specifiche di progetto.
    static double s_mouseX = 0.0;
    static double s_mouseY = 0.0;

    // ========================================================================
    // Callback di Pressione Tasti
    // ========================================================================
    // [Domanda 19: Input, eventi e callback GLFW (vs polling continuo in Player)]
    void keyCallback(
        GLFWwindow* window,
        int key,
        int scancode,
        int action,
        int mods)
    {
        // Se l'utente preme il tasto ESCAPE (GLFW_PRESS), ordiniamo a GLFW di chiudere la finestra.
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            // Imposta il flag 'WindowShouldClose' su true, che farà terminare il game-loop in main().
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    // ========================================================================
    // Callback di Movimento del Cursore
    // ========================================================================
    void cursorPositionCallback(
        GLFWwindow* window,
        double xpos,
        double ypos)
    {
        int width;
        int height;

        // Recupera le dimensioni fisiche del framebuffer per gestire correttamente display HiDPI/Retina.
        glfwGetFramebufferSize(window, &width, &height);

        // Memorizza la coordinata orizzontale X così come restituita da GLFW (0 a sinistra).
        s_mouseX = xpos;

        // Sistema di coordinate GLFW vs Sistema di coordinate OpenGL:
        // GLFW misura la coordinata Y dall'alto verso il basso (0 in alto, height in basso).
        // OpenGL (e la nostra matrice ortografica) colloca l'origine (0,0) nell'angolo in basso a sinistra.
        // Invertiamo quindi l'asse Y sottraendo la coordinata ypos dall'altezza totale del framebuffer.
        s_mouseY = static_cast<double>(height) - ypos;
    }

    // ========================================================================
    // Callback di Ridimensionamento del Framebuffer
    // ========================================================================
    void framebufferSizeCallback(
        GLFWwindow* window,
        int width,
        int height)
    {
        // glViewport definisce la porzione rettangolare della finestra in cui OpenGL effettua il disegno.
        // Parametri:
        // - x, y: coordinate dell'angolo inferiore sinistro del viewport (0, 0).
        // - width, height: larghezza e altezza del nuovo rettangolo di rendering in pixel fisici.
        // In questo modo, se l'utente allarga o ridimensiona la finestra, l'immagine OpenGL
        // si adatta immediatamente alla nuova risoluzione senza distorsioni o tagli.
        glViewport(0, 0, width, height);
    }
}
