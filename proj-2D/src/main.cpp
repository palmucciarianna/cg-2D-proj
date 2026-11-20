// ============================================================================
// File: src/main.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Entry-point principale dell'applicazione. Inizializza il sottosistema
//              grafico GLFW, carica i puntatori a funzione OpenGL con GLAD, configura
//              il contesto dell'interfaccia Dear ImGui, istanzia il GameWorld ed
//              esegue il game-loop a framerate controllato (con gestione del delta-time).
// ============================================================================

// Inclusione centralizzata cross-platform OpenGL / WebGL
#include "OpenGL_Headers.h"
#include <GLFW/glfw3.h>

// Header ufficiali della libreria Dear ImGui per l'interfaccia utente grafica (HUD).
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// Moduli del nostro motore grafico e di gioco.
#include "GameWorld.h"
#include "Input.h"

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

// Dimensioni logiche iniziali della finestra di gioco in pixel.
// Il sistema di coordinate ortografiche del mondo mapperà lo spazio [0, 1280] x [0, 720].
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Contesto di esecuzione per il frame loop (permette l'integrazione sia del while loop desktop
// che del callback asincrono del browser tramite emscripten_set_main_loop_arg)
struct MainLoopContext
{
    GLFWwindow* window;
    GameWorld* gameWorld;
    float lastTime;
};

// Funzione eseguita ad ogni singolo frame di rendering e logica
void mainLoopIteration(MainLoopContext& ctx)
{
    // A. Calcolo del Delta-Time (Tempo trascorso tra due frame consecutivi)
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - ctx.lastTime;
    ctx.lastTime = currentTime;

    // "Delta-Time Clamping": limita il delta time a un massimo di 50 millisecondi (0.05s).
    if (deltaTime > 0.05f) 
        deltaTime = 0.05f;

    // B. Elaborazione degli Eventi di Sistema GLFW (Tastiera, Mouse, Finestra)
    glfwPollEvents();

    // C. Inizio del Frame Grafico ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // D. Aggiornamento della Logica del Gioco (Fisica, IA, Collisioni)
    ctx.gameWorld->update(deltaTime, ctx.window);

    // E. Preparazione del Viewport e Pulizia del Framebuffer
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(ctx.window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // F. Rendering della Scena 2D (Sfondo, Navicella, Asteroidi, Spline, Particelle)
    ctx.gameWorld->render(currentTime);

    // G. Costruzione dell'Interfaccia Utente Grafica (HUD di Missione)
    ctx.gameWorld->renderUI(deltaTime);

    // H. Disegno Effettivo di ImGui tramite OpenGL
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // I. Scambio dei Buffer (Double Buffering)
    glfwSwapBuffers(ctx.window);
}

#ifdef __EMSCRIPTEN__
static void emscriptenMainLoop(void* arg)
{
    MainLoopContext* ctx = static_cast<MainLoopContext*>(arg);
    mainLoopIteration(*ctx);
}
#endif

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

    // Specifica i requisiti minimi del contesto OpenGL / WebGL
#ifdef __EMSCRIPTEN__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
    // Versione 3.3 Core Profile su Desktop.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    // Su macOS è obbligatorio abilitare il forward compatibility per i contesti OpenGL >= 3.2.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
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

#ifndef __EMSCRIPTEN__
    // GLAD carica dinamicamente a runtime i puntatori a funzione OpenGL implementati dal driver GPU.
    // Passiamo glfwGetProcAddress come caricatore di indirizzi di sistema.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "[ERRORE] Inizializzazione GLAD fallita!\n";
        return -1;
    }
#endif

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
    // e per OpenGL 3.3 Core Profile o WebGL 2.0 (rendering dei vertici della GUI).
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Init("#version 300 es");
#else
    ImGui_ImplOpenGL3_Init("#version 330 core");
#endif

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

    MainLoopContext context{ window, &gameWorld, lastTime };

#ifdef __EMSCRIPTEN__
    // In ambiente WebAssembly (browser), cediamo il ciclo di rendering al browser loop
    emscripten_set_main_loop_arg(emscriptenMainLoop, &context, 0, true);
#else
    // Esegue finché l'utente non richiede la chiusura della finestra (es. premendo la X o ESC).
    while (!glfwWindowShouldClose(window))
    {
        mainLoopIteration(context);
    }
#endif

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
