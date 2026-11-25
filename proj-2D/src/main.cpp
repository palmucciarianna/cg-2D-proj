// ORIGINE: Scheletro standard di avvio/chiusura GLFW + GLAD + ImGui dai laboratori della prof 
// (glfwInit, contesto 3.3 Core Profile, gladLoadGLLoader, ImGui_Impl*, swap/terminate), 
// esteso con GameWorld, Input, V-Sync, Alpha Blending e Delta-Time Clamping.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - ENTRY-POINT: main.cpp (Architettura & Render Loop)
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - È il punto di ingresso dell'applicazione (main function).
//    - Inizializza le librerie fondamentali: GLFW, GLAD, Dear ImGui.
//    - Configura il contesto OpenGL 3.3 Core Profile e il double buffering V-Sync.
//    - Esegue il game-loop principale a framerate controllato (con Delta-Time Clamping a 0.05s).
//    - Rilascia ordinatamente tutte le risorse software e hardware alla chiusura della finestra.
//
// 2. A CHI È COLLEGATO:
//    - Crea e coordina: GameWorld (mondo di gioco 2D) e Input (callback periferiche).
//    - Si interfaccia direttamente con i driver grafici del sistema operativo e della GPU.
//
// 3. FORMULE & PRINCIPI DI COMPUTER GRAPHICS:
//    - Sequenza di Inizializzazione OpenGL (Domanda 16):
//      1) glfwInit();
//      2) glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR/MINOR, 3) + GLFW_OPENGL_CORE_PROFILE;
//      3) window = glfwCreateWindow(1280, 720, ...);
//      4) glfwMakeContextCurrent(window);  // FONDAMENTALE prima di GLAD!
//      5) gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
//    - Configurazione Blending per Trasparenza (Domanda 20):
//      glEnable(GL_BLEND);  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//      C_out = alpha_s * C_s + (1 - alpha_s) * C_d.
//    - Delta-Time Clamping & Indipendenza da Framerate (Requisito 6):
//      dt = currentFrame - lastFrame;  dt = min(dt, 0.05f);
//      Evita glitch di attraversamento muri o instabilità numerica in caso di lag/pausa.
//    - Double Buffering & Swap Buffers (Domande 1, 16):
//      Disegna nel Back Buffer invisibile e lo scambia col Front Buffer a video (glfwSwapBuffers).
// ============================================================================

// Disabilita l'inclusione automatica degli header OpenGL legacy da parte di GLFW,
// poiché utilizzeremo GLAD per caricare il profilo Core di OpenGL 3.3.
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Header ufficiali della libreria Dear ImGui per l'interfaccia utente grafica (HUD).
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// Moduli del nostro motore grafico e di gioco.
#include "GameWorld.h"
#include "Input.h"

#include <iostream>

// Dimensioni logiche iniziali della finestra di gioco in pixel.
// Il sistema di coordinate ortografiche del mondo mapperà lo spazio [0, 1280] x [0, 720].
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

int main()
{
    // ========================================================================
    // 1. INIZIALIZZAZIONE DI GLFW (Finestra, Contesto e Callback di Sistema)
    // ========================================================================

    // Inizializza la libreria GLFW; restituisce false se fallisce (es. driver video assente).
    if (!glfwInit())
    {
        std::cerr << "[ERRORE] Inizializzazione GLFW fallita!\n";
        return -1;
    }

    // Specifica i requisiti minimi del contesto OpenGL: versione 3.3 Core Profile.
    // Questo esclude tutte le funzioni a pipeline fissa deprecate (glBegin, glEnd, glMatrixMode).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    // Su macOS è obbligatorio abilitare il forward compatibility per i contesti OpenGL >= 3.2.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Crea la finestra GLFW e il contesto OpenGL associato.
    // Parametri: larghezza, altezza, titolo finestra, puntatore monitor (nullptr = finestra), contesto condiviso.
    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH, 
        SCR_HEIGHT, 
        "Astro-Miner 2D: Cosmic Coin Rush", 
        nullptr, 
        nullptr
    );

    // Verifica che la finestra sia stata creata correttamente.
    if (!window)
    {
        std::cerr << "[ERRORE] Creazione della finestra GLFW fallita!\n";
        glfwTerminate();
        return -1;
    }

    // Rende il contesto OpenGL appena creato "corrente" per il thread principale di esecuzione.
    glfwMakeContextCurrent(window);

    // Abilita la sincronizzazione verticale (V-Sync) a 1 intervallo di refresh rate del monitor.
    // Evita il tearing e sincronizza i cicli a 60/120 Hz.
    glfwSwapInterval(1);

    // Registra le funzioni di callback per la gestione degli eventi di input e di sistema:
    // - keyCallback: rileva la pressione dei tasti (es. ESC per uscire).
    glfwSetKeyCallback(window, Input::keyCallback);
    // - cursorPositionCallback: aggiorna la posizione assoluta del cursore del mouse.
    glfwSetCursorPosCallback(window, Input::cursorPositionCallback);
    // - framebufferSizeCallback: gestisce il ridimensionamento della finestra aggiornando il viewport.
    glfwSetFramebufferSizeCallback(window, Input::framebufferSizeCallback);

    // ========================================================================
    // 2. INIZIALIZZAZIONE DI GLAD (Caricatore Funzioni OpenGL)
    // ========================================================================

    // GLAD carica dinamicamente a runtime i puntatori a funzione OpenGL implementati dal driver GPU.
    // Passiamo glfwGetProcAddress come caricatore di indirizzi di sistema.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "[ERRORE] Inizializzazione GLAD fallita!\n";
        return -1;
    }

    // Stampa su console le specifiche del dispositivo grafico e della versione del driver.
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version:  " << glGetString(GL_VERSION) << std::endl;

    // Configura il blending alfa (trasparenza):
    // Abilita il canale alfa per il rendering corretto di particelle traslucide e texture PNG/RGBA.
    glEnable(GL_BLEND);
    // Formula di blending standard: Color = SrcColor * SrcAlpha + DstColor * (1 - SrcAlpha)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ========================================================================
    // 3. CONFIGURAZIONE DI DEAR IMGUI (HUD e Interfaccia Utente)
    // ========================================================================

    // Verifica che le versioni degli header e dei binari di ImGui coincidano.
    IMGUI_CHECKVERSION();
    // Alloca il contesto globale di Dear ImGui.
    ImGui::CreateContext();
    // Recupera la struttura di configurazione Input/Output di ImGui.
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Disabilita la persistenza del file imgui.ini sul disco, rispettando il vincolo
    // accademico di non generare file di configurazione temporanei non tracciati.
    io.IniFilename = nullptr;

    // Imposta il tema visivo scuro (Dark Theme) per l'interfaccia.
    ImGui::StyleColorsDark();

    // Inizializza i backend ImGui per GLFW (gestione eventi finestra/input)
    // e per OpenGL 3.3 Core Profile (rendering dei vertici della GUI).
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // ========================================================================
    // 4. INIZIALIZZAZIONE DEL MONDO DI GIOCO (GameWorld)
    // ========================================================================

    // Alloca l'istanza del mondo di gioco incapsulando tutte le entità e i sistemi.
    GameWorld gameWorld(SCR_WIDTH, SCR_HEIGHT);
    // Carica gli shader, compila le geometrie e alloca i buffer nella cartella "assets".
    gameWorld.init("assets");

    // Memorizza il tempo di sistema iniziale (in secondi) per il calcolo del delta-time.
    float lastTime = static_cast<float>(glfwGetTime());

    // ========================================================================
    // 5. GAME-LOOP PRINCIPALE (Ciclo di Aggiornamento e Rendering)
    // ========================================================================

    // Esegue finché l'utente non richiede la chiusura della finestra (es. premendo la X o ESC).
    while (!glfwWindowShouldClose(window))
    {
        // --------------------------------------------------------------------
        // A. Calcolo del Delta-Time (Tempo trascorso tra due frame consecutivi)
        // --------------------------------------------------------------------
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // "Delta-Time Clamping": limita il delta time a un massimo di 50 millisecondi (0.05s).
        // Se la finestra viene spostata o l'applicazione va in pausa momentanea, questo clamping
        // impedisce glitch fisici catastrofici (es. la navicella che attraversa i muri o le rocce).
        if (deltaTime > 0.05f) 
            deltaTime = 0.05f;

        // --------------------------------------------------------------------
        // B. Elaborazione degli Eventi di Sistema GLFW (Tastiera, Mouse, Finestra)
        // --------------------------------------------------------------------
        glfwPollEvents();

        // --------------------------------------------------------------------
        // C. Inizio del Frame Grafico ImGui
        // --------------------------------------------------------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --------------------------------------------------------------------
        // D. Aggiornamento della Logica del Gioco (Fisica, IA, Collisioni)
        // --------------------------------------------------------------------
        gameWorld.update(deltaTime, window);

        // --------------------------------------------------------------------
        // E. Preparazione del Viewport e Pulizia del Framebuffer
        // --------------------------------------------------------------------
        int fbWidth, fbHeight;
        // Recupera la reale dimensione del framebuffer in pixel (fondamentale su display Retina/HiDPI).
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        // Imposta il rettangolo di disegno OpenGL sulla totalità dei pixel della finestra.
        glViewport(0, 0, fbWidth, fbHeight);
        // Pulisce il buffer colore e il buffer di profondità prima di iniziare a disegnare.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --------------------------------------------------------------------
        // F. Rendering della Scena 2D (Sfondo, Navicella, Asteroidi, Spline, Particelle)
        // --------------------------------------------------------------------
        gameWorld.render(currentTime);

        // --------------------------------------------------------------------
        // G. Costruzione dell'Interfaccia Utente Grafica (HUD di Missione)
        // --------------------------------------------------------------------
        gameWorld.renderUI(deltaTime);

        // --------------------------------------------------------------------
        // H. Disegno Effettivo di ImGui tramite OpenGL
        // --------------------------------------------------------------------
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // --------------------------------------------------------------------
        // I. Scambio dei Buffer (Double Buffering)
        // --------------------------------------------------------------------
        // Scambia il back-buffer (in cui abbiamo appena renderizzato) con il front-buffer (visibile).
        glfwSwapBuffers(window);
    }

    // ========================================================================
    // 6. TERMINAZIONE E DEALLOCAZIONE DELLE RISORSE
    // ========================================================================

    // Dealloca i contesti grafici di ImGui.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Distrugge la finestra creata e rilascia la memoria associata.
    glfwDestroyWindow(window);
    // Termina il sottosistema GLFW.
    glfwTerminate();

    return 0;
}
