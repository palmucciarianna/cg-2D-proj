# GUIDA DEFINITIVA ALLO STUDIO ORALE: ASTRO-MINER 2D
## Manuale Completo di Preparazione all'Esame di Computer Graphics
**Candidata:** Arianna Palmucci  
**Docente:** Prof.ssa Damiana Lazzaro – Università di Bologna  
**Corso:** Computer Graphics (A.A. 2026/2027 – Parziale I)  
**Progetto:** *Astro-Miner 2D: Cosmic Coin Rush* (OpenGL 3.3 Core Profile, C++17)

---

# INDICE GENERALE
1. [La Strategia per un Esame da 30 e Lode](#1-la-strategia-per-un-esame-da-30-e-lode)
2. [Architettura Globale: Dal C++ alla GPU al Framebuffer](#2-architettura-globale-dal-c-alla-gpu-al-framebuffer)
3. [Anatomia del Codice: File per File e Funzione per Funzione](#3-anatomia-del-codice-file-per-file-e-funzione-per-funzione)
4. [Tutti gli Shader e gli Asset: Il Codice non C++ Spiegato nei Dettagli](#4-tutti-gli-shader-e-gli-asset-il-codice-non-c-spiegato-nei-dettagli)
5. [Mappatura dei Requisiti di Progetto Ufficiali (3.1 – 8)](#5-mappatura-dei-requisiti-di-progetto-ufficiali-31--8)
6. [Mappatura sulle 20 Domande Ufficiali del Parziale I](#6-mappatura-sulle-20-domande-ufficiali-del-parziale-i)
7. [Come Potenziare la Presentazione Canva e la Demo Live](#7-come-potenziare-la-presentazione-canva-e-la-demo-live)

---

# 1. LA STRATEGIA PER UN ESAME DA 30 E LODE

### Come affrontare il colloquio con la professoressa
La professoressa Damiana Lazzaro valuta due cose fondamentali:
1. **Padronanza assoluta dei concetti teorici e geometrici:** non devi dare definizioni "vaghe" o "a senso", ma usare il vocabolario formale della grafica (spazio affine, coordinate omogenee, prodotto scalare come proiezioni, basi di Hermite $C^1$, interpolazione baricentrica, pipeline programmabile).
2. **Capacità di collegare ogni riga di codice alla teoria:** quando la docente ti chiede *"Dove hai usato le coordinate omogenee?"* oppure *"Come hai gestito la rasterizzazione?"*, non limitarti a dire "nel vertex shader", ma spiega:
   > *"Nel vertex shader `basic.vert` moltiplico il vettore posizione esteso a coordinate omogenee con $w=1.0$ per la matrice `model` e poi per la matrice `projection` ortografica `glm::ortho`. La GPU effettua il clipping nello spazio canonico $[-1, 1]^3$, esegue la divisione prospettica normalizzando le coordinate in NDC, e poi la fase a funzione fissa di Viewport Mapping converte le coordinate NDC nei pixel della finestra 1280x720."*

### La Regola dei Tre Passi per ogni risposta orale
Ad ogni domanda, rispondi sempre con questa struttura in tre passaggi:
1. **Definizione formale matematica/grafica** (15-20 secondi).
2. **Formula analitica o algoritmo** (citando le variabili esatte: $M = T \cdot R \cdot S$, $P = \alpha A + \beta B + \gamma C$, funzioni $h_{ij}(t)$, edge functions $E_i(x,y)$).
3. **Collegamento pratico al tuo progetto** (citando il file esatto e il comportamento visivo del gioco).

---

# 2. ARCHITETTURA GLOBALE: DAL C++ ALLA GPU AL FRAMEBUFFER

Il motore grafico di *Astro-Miner 2D* è strutturato secondo il principio di **Separation of Concerns (SoC)**, senza alcuna variabile globale e con gestione rigorosa delle risorse secondo il paradigma **RAII** (*Resource Acquisition Is Initialization*).

```
+-----------------------------------------------------------------------------------+
|                                 APPLICATION (CPU)                                 |
|                                                                                   |
|  main.cpp ---> GLFW (Finestra, Contesto, Eventi, V-Sync)                           |
|           ---> GLAD (Caricamento puntatori funzioni OpenGL Core Profile 3.3)      |
|           ---> GameWorld (Orchestrazione FSM, Fisica, Delta-Time Clamping)        |
|                  |                                                                |
|                  +--> Player (Gerarchia matrici 11 componenti, Texture UV)        |
|                  +--> AsteroidManager (Buffer Sharing: 1 VAO statico x 9 istanze)|
|                  +--> CollectibleManager (5 monete d'oro, rotazione pseudo-3D)    |
|                  +--> HermiteManager (Alien Drone su Lissajous, Warp Gate)        |
|                  +--> LaserManager & HazardManager (Proiettili e Pericoli)        |
|                  +--> ParticleSystem (Dinamica particelle con attrito viscoso)    |
|                  +--> BackgroundRenderer (Fullscreen Quad in NDC [-1,1])          |
|                  +--> ImGui (HUD diagnostico con auto-compressione non invasiva)  |
+-----------------------------------------------------------------------------------+
                                       |
                   Dati vertici (VBO/VAO) | Matrici Uniform (M, P, uTime)
                                       v
+-----------------------------------------------------------------------------------+
|                           GEOMETRY PROCESSING (GPU)                               |
|                                                                                   |
|  Vertex Shader (basic.vert / background.vert / coin.vert / texture.vert)         |
|  - Trasformazione affine: P_clip = P * M * P_local                                |
|  - Clipping nello spazio canonico [-1, 1]^3                                      |
|  - Divisione omogenea per w: P_ndc = P_clip / w                                  |
+-----------------------------------------------------------------------------------+
                                       |
                   Primitive assemblate (Triangoli)
                                       v
+-----------------------------------------------------------------------------------+
|                               RASTERIZATION (GPU)                                 |
|                                                                                   |
|  - Viewport Transform: x_s = x_0 + (x_ndc + 1)/2 * W, y_s = y_0 + (y_ndc + 1)/2 * H|
|  - Edge Functions (E0, E1, E2 <= 0) calcolate al centro del pixel (i+0.5, j+0.5)  |
|  - Interpolazione baricentrica dei colori di vertice e coordinate UV              |
+-----------------------------------------------------------------------------------+
                                       |
                   Frammenti generati
                                       v
+-----------------------------------------------------------------------------------+
|                          FRAGMENT PROCESSING & ROP (GPU)                          |
|                                                                                   |
|  Fragment Shader (basic.frag / background.frag / texture.frag)                    |
|  - Sfondo procedurale (Domain Warping, Parallasse stellare a 3 strati)            |
|  - Texture sampling con bilineare e maschera ellittica analitica                  |
|  - Alpha Blending: Cout = alpha_s * Cs + (1 - alpha_s) * Cd                       |
|  - Scrittura nel Back Buffer ---> glfwSwapBuffers() ---> Monitor                  |
+-----------------------------------------------------------------------------------+
```

---

# 3. ANATOMIA DEL CODICE: FILE PER FILE E FUNZIONE PER FUNZIONE

### Mappa dei File Aggiornati con i Link Diretti e le Schede per l'Orale:
Tutti i file del progetto contengono in cima al codice la **Guida Rapida per l'Orale** con lo schema: *1. A cosa serve*, *2. A chi è collegata*, *3. Formule e principi CG utilizzati*.

* **Entry point e Coordinamento:**
  - [`main.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/main.cpp) (Inizializzazione, GLFW, GLAD, V-Sync, Render Loop con Delta-Time Clamping)
  - [`GameWorld.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/GameWorld.h) e [`GameWorld.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/GameWorld.cpp) (FSM, `glm::ortho`, coordinamento collisioni, HUD Dear ImGui)
* **Entità di Gioco e Modellazione:**
  - [`Player.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/Player.h) e [`Player.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/Player.cpp) (Albero gerarchico a 11 nodi, $S(-1,1,1)$, texture cabina, fiamme)
  - [`AsteroidManager.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/AsteroidManager.h) e [`AsteroidManager.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/AsteroidManager.cpp) (Buffer Sharing con 1 solo VAO statico su VRAM per $N$ istanze)
  - [`Collectible.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/Collectible.h) e [`Collectible.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/Collectible.cpp) (5 monete d'oro, rotazione pseudo-3D ortogonale $s_x(t) = |\cos(\omega t)|$)
  - [`HermiteObject.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/HermiteObject.h) e [`HermiteObject.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/HermiteObject.cpp) (Spline cubiche di Hermite $C^1$, traiettoria a 8 di Lissajous, ammiccamento eye-blink, Warp Gate)
* **Balistica, Pericoli e Simulazione Fisica:**
  - [`LaserManager.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/LaserManager.h) e [`LaserManager.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/LaserManager.cpp) (Proiettili plasma a 950 px/s, clipping finestra)
  - [`HazardManager.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/HazardManager.h) e [`HazardManager.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/HazardManager.cpp) (Pericoli supersonici 880 px/s con lampeggio stroboscopico di allerta)
  - [`ParticleSystem.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/ParticleSystem.h) e [`ParticleSystem.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/ParticleSystem.cpp) (Attrito viscoso $v(t+\Delta t) = v(t)(1-\gamma\Delta t)$, decadimento alpha e rimpicciolimento)
* **Pipeline Grafica, Shader e Geometria:**
  - [`BackgroundRenderer.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/BackgroundRenderer.h) e [`BackgroundRenderer.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/BackgroundRenderer.cpp) (Fullscreen quad in NDC $[-1,1]$, RAII e Rule of Five)
  - [`Geometry.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/Geometry.h) e [`Geometry.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/Geometry.cpp) (Incapsulamento hardware VAO/VBO, upload e draw call)
  - [`ShapeGenerator.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/ShapeGenerator.h) e [`ShapeGenerator.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/ShapeGenerator.cpp) (Coordinate polari, triangolazioni e vertex color)
  - [`Shader.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/Shader.h) e [`Shader.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/Shader.cpp) (Program ID, setter uniform `glUniform*`)
  - [`ShaderMaker.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/ShaderMaker.h) e [`ShaderMaker.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/ShaderMaker.cpp) (Compilazione GLSL a runtime e controllo log di errore)
  - [`Texture.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/Texture.h) e [`Texture.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/Texture.cpp) (`stb_image.h`, flip verticale per coordinate UV di OpenGL, filtri bilineari)
* **Matematica, Input e Tipi di Base:**
  - [`Collider2D.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/Collider2D.h) (Cerchio-Cerchio con distanza al quadrato senza radice quadrata, AABB SAT)
  - [`Transform.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/Transform.h) (Composizione $M = T \cdot R \cdot S$ e grafi di scena gerarchici)
  - [`MeshData.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/MeshData.h) (Contenitore CPU per vertici e colori)
  - [`Input.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/Input.h) e [`Input.cpp`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/src/Input.cpp) (Callback GLFW vs Polling continuo)
  - [`PlayerShapeLoader.h`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/include/PlayerShapeLoader.h) (Parser per file `.obj` e `.mtl` con triangolazione a ventaglio)

---

## 3.1 `src/main.cpp` – Il Punto di Ingresso e il Ciclo di Rendering
* **A cosa serve:** Inizializza le librerie grafiche, istanzia il gioco, esegue il game-loop a tempo reale e dealloca tutto all'uscita.
* **Funzioni e chiamate chiave riga per riga:**
  * `#define GLFW_INCLUDE_NONE`: impedisce a GLFW di includere gli header obsoleti di OpenGL 1.x, garantendo che sia GLAD a caricare i simboli moderni.
  * `glfwInit()`: inizializza la libreria di gestione finestre.
  * `glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3)` e `MINOR, 3`: richiede un contesto OpenGL 3.3.
  * `glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE)`: disabilita le funzioni fixed-function deprecate (`glBegin`, `glEnd`, `glMatrixMode`).
  * `glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE)`: obbligatorio su macOS per contesti OpenGL $\ge 3.2$.
  * `glfwCreateWindow(1280, 720, ...)`: crea la finestra del sistema operativo e alloca il color framebuffer (Back Buffer e Front Buffer).
  * `glfwMakeContextCurrent(window)`: lega il contesto OpenGL al thread corrente.
  * `glfwSwapInterval(1)`: attiva la sincronizzazione verticale (V-Sync a 60/120 Hz) per eliminare il fenomeno del screen tearing.
  * `gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)`: interroga i driver grafici della GPU per mappare dinamicamente i puntatori a tutte le funzioni OpenGL 3.3.
  * `glEnable(GL_BLEND)` e `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`: configura il pipeline state per la trasparenza (Requisito 20).
  * `ImGui::CreateContext()`, `ImGui_ImplGlfw_InitForOpenGL`, `ImGui_ImplOpenGL3_Init`: collegano la libreria di interfaccia grafica al nostro contesto.
  * **Il Render Loop:**
    ```cpp
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        deltaTime = std::min(deltaTime, 0.05f); // Delta-Time Clamping

        glfwPollEvents(); // Svuota la coda eventi OS
        // Aggiorna ImGui
        world.update(deltaTime, window); // Logica e fisica
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); // Pulizia buffer colore
        world.render(); // Draw calls OpenGL
        ImGui::Render(); // Disegno HUD ImGui
        glfwSwapBuffers(window); // Double Buffering
    }
    ```
  * **Shutdown:** Dealloca ordinatamente ImGui, distrugge la finestra con `glfwDestroyWindow` e termina GLFW con `glfwTerminate`.

---

## 3.2 `src/GameWorld.cpp` e `include/GameWorld.h` – Il Coordinatore Centrale
* **A cosa serve:** È il cervello dell'applicazione. Contiene la macchina a stati finiti (FSM), le dimensioni del mondo, calcola la matrice di proiezione ortografica e orchestra la logica di tutte le entità.
* **Componenti e Metodi:**
  * `enum class GameState { PLAYING, LEVEL_CLEARED_HURRA, GAME_OVER }`: gestisce i 3 stati formali del gioco (Requisito 7.3).
  * `init(const std::string& assetDir)`: compila i due shader program (`background` e `basic`), alloca `BackgroundRenderer`, inizializza navicella, asteroidi, monete, pericoli, particelle e carica i modelli Wavefront OBJ delle curve di Hermite.
  * `update(float dt, GLFWwindow* window)`:
    * Se `PLAYING`: aggiorna il timer, legge i comandi di sparo (Spazio) o accelerazione (Shift), aggiorna la cinematica di navicella, asteroidi, pericoli e drone di Hermite.
    * Risolve le collisioni:
      * Navicella vs Asteroide: perde una vita, fa tremare la camera (Screen Shake) e attiva un'esplosione particellare. A 0 vite passa a `GAME_OVER`.
      * Navicella vs Moneta: raccoglie la moneta d'oro, emette scintille dorate, incrementa il punteggio (+250). A 5 monete sblocca il portale cosmico Warp Gate.
      * Laser vs Asteroide: l'asteroide si polverizza in 32 frammenti fisici con attrito viscoso; conferisce +100 punti.
      * Navicella vs Warp Gate: se le 5 monete sono state raccolte, il salto dimensionale ha successo $\rightarrow$ passa allo stato trionfale `LEVEL_CLEARED_HURRA` con fontana di coriandoli particellari!
  * `render()`:
    * Calcola la matrice di proiezione:
      $$\mathbf{P} = \text{glm::ortho}(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f)$$
    * Emette la prima passata con `BackgroundRenderer::draw` (disegna il fullscreen quad procedurale in NDC).
    * Emette la seconda passata con `m_basicShader->use()`, inviando la matrice `projection` via `glUniformMatrix4fv`.
    * Disegna nell'ordine corretto: portale, monete, asteroidi, droni di Hermite, pericoli, navicella, raggi laser e particelle traslucide.
  * `renderHUD()`: interfaccia ImGui intelligente. Mostra punteggio cumulativo, vite grafiche a cuori (`<3 <3 <3`), barra di avanzamento monete e slider interattivo per la velocità della navicella. Implementa il controllo su `!io.WantCaptureMouse` per non interferire con la guida.

---

## 3.3 `src/BackgroundRenderer.cpp` e `include/BackgroundRenderer.h` – Il Fullscreen Quad NDC
* **A cosa serve:** Renderizza lo sfondo dello spazio senza bisogno di una texture pre-renderizzata, coprendo l'intero schermo a qualsiasi risoluzione.
* **Perché è speciale per la teoria (Requisito 5 & Domanda 9):**
  * I suoi 6 vertici sono definiti **direttamente in NDC** (Normalized Device Coordinates) nell'intervallo $[-1, 1] \times [-1, 1]$:
    ```cpp
    const float vertices[] = {
        -1.0f, -1.0f,   1.0f, -1.0f,   1.0f,  1.0f,
        -1.0f, -1.0f,   1.0f,  1.0f,  -1.0f,  1.0f
    };
    ```
  * **Zero moltiplicazioni di matrici:** Nel suo vertex shader `background.vert`, la posizione del vertice è passata direttamente come `gl_Position = vec4(aPos, 0.0, 1.0);`. Non viene usata né la matrice Model né la matrice Projection.
  * **Gestione RAII completa:** Il costruttore genera VAO e VBO con `glGenVertexArrays` e `glGenBuffers`; il distruttore dealloca tutto con `glDeleteVertexArrays` e `glDeleteBuffers`. Implementa la **Rule of Five** (costruttore di copia e assegnamento eliminati, move constructor e move assignment implementati o disabilitati per evitare duplicazione di handle GPU).

---

## 3.4 `src/Player.cpp` e `include/Player.h` – Modellazione Gerarchica della Navicella
* **A cosa serve:** Rappresenta la navicella mineraria guidata dal giocatore, composta da **11 componenti assemblati gerarchicamente** (Requisito 3.1).
* **Albero di Trasformazione Gerarchica:**
  * **Radice (Model Matrix Globale):**
    $$M_{\text{player}} = T(x_{\text{pos}}, y_{\text{pos}}) \cdot R(\theta_{\text{angolo}}) \cdot S(s, s)$$
  * **Nodi Figli (Componenti Locali):** Ogni parte calcola la sua trasformazione locale $M_{\text{locale}} = T_{\text{loc}} \cdot R_{\text{loc}} \cdot S_{\text{loc}}$ e viene disegnata con la matrice composta:
    $$M_{\text{finale}} = M_{\text{player}} \cdot M_{\text{locale}}$$
  * **I componenti dell'albero:**
    1. *Fusoliera Centrale:* Trapezio allungato con gradiente grigio-blu metallico.
    2. *Muso a Freccia:* Triangolo isoscele aguzzo frontale.
    3. *Cabina di Pilotaggio con Texture:* Quad rettangolare mappato con texture fotografica UV.
    4. *Ala Destra:* Poligono alare inclinato con pinna stabilizzatrice.
    5. *Ala Sinistra Speculare (Algebra Lineare):* Invece di duplicare i vertici, viene disegnata con la stessa geometria dell'ala destra anteponendo una **matrice di riflessione speculare a scala negativa**:
       $$\mathbf{S}(-1, 1, 1) = \begin{pmatrix} -1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \end{pmatrix}$$
    6. *Cannone Plasma Destro:* Rettangolo d'acciaio montato sul bordo alare.
    7. *Cannone Plasma Sinistro:* Cannone specchiato simmetrico.
    8. *Ugelli di Scarico dei Reattori (Doppi):* Tubi di scarico posteriori in titanio scuro.
    9. *Fiamme Reattive dei Propulsori:* Triangoli arancio-gialli la cui scala orizzontale varia esponenzialmente con l'accelerazione della navicella:
       $$s_{\text{fiamma}}(t) = s_{\text{base}} \cdot \left(0.4 + 0.6 \cdot \frac{v}{v_{\text{max}}} + 0.15 \sin(25 t)\right)$$
    10. *Luci di Navigazione Alari (Strobe LEDs):* Cerchietti verde (dritta) e rosso (sinistra) che lampeggiano modulando il canale alpha con una funzione quadra/sinusoidale.
* **Cinematica Euleriana:**
  $$v(t + \Delta t) = v(t) + a \cdot \Delta t$$
  $$p(t + \Delta t) = p(t) + v(t) \cdot \Delta t$$
  Con attrito di trascinamento inerziale $v \leftarrow v \cdot (1 - \beta \Delta t)$ per dare una sensazione realistica di volo nello spazio.

---

## 3.5 `src/AsteroidManager.cpp` e `include/AsteroidManager.h` – Buffer Sharing & Instancing
* **A cosa serve:** Gestisce la cintura di asteroidi (da 4 a 9 a seconda del livello).
* **Perché è cruciale per la professoressa (Requisito 4 & Domanda 17):**
  * Dimostra la tecnica del **Buffer Sharing**: in memoria VRAM della GPU viene allocato **un solo VBO e un solo VAO** in modalità `GL_STATIC_DRAW` contenente la geometria base dell'asteroide (32 segmenti poligonali con variazioni radiali).
  * Nel game-loop, per disegnare tutti gli $N$ asteroidi sparsi per lo schermo:
    ```cpp
    for (const auto& a : m_asteroids) {
        if (!a.active) continue;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(a.pos, 0.0f));
        model = glm::rotate(model, glm::radians(a.rotation), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(a.scale, a.scale, 1.0f));
        shader.setMat4("model", model); // Unica cosa che viaggia verso la GPU!
        m_asteroidGeo.draw();           // glDrawArrays usa lo stesso VAO residente in VRAM
    }
    ```
  * **Vantaggio architetturale:** Zero bandwidth sprecata per inviare coordinate di vertici nel loop di rendering; viaggia solo una matrice $4 \times 4$ (64 byte) per istanza.

---

## 3.6 `src/Collectible.cpp` e `include/Collectible.h` – Monete d'Oro e Rotazione Pseudo-3D
* **A cosa serve:** Genera le 5 monete d'oro collezionabili richieste dal Requisito 7.2.
* **Perché è speciale per la teoria (Requisito 6 & Domande 6, 9):**
  * **Rotazione Pseudo-3D:** Come far apparire una moneta 2D come un oggetto tridimensionale che ruota su se stesso senza usare un motore 3D? Sfruttando la proiezione parallela ortografica! Se una moneta circolare 3D sul piano $XZ$ ruota attorno all'asse $Y$ con velocità angolare $\omega$, la sua proiezione ortogonale sul piano $XY$ è un'ellisse la cui ampiezza orizzontale varia secondo la legge:
    $$s_x(t) = s_{\text{base}} \cdot |\cos(\omega t)|$$
    Modulando la componente $X$ del vettore di scalatura della Model Matrix con il valore assoluto del coseno, la moneta si assottiglia fino a diventare una linea sottile e poi si riallarga, simulando perfettamente la rotazione solida 3D.
  * **Oscillazione Armonica Verticale:**
    $$y(t) = y_{\text{base}} + A \cdot \sin(\nu t + \phi)$$
    Implementata sia tramite CPU che tramite shader dedicato `coin.vert`.

---

## 3.7 `src/HermiteObject.cpp` e `include/HermiteObject.h` – Le Spline di Hermite
* **A cosa serve:** Implementa le due forme geometriche curvilinee avanzate a continuità $C^1$ richieste dal Requisito 3.2.
* **1. Alien Sentinel Drone (Ostacolo Dinamico Ostile):**
  * **Geometria:** Caricata da `alien_drone.obj`. I vertici del profilo della testa cornuta sono campionati lungo una spline cubica di Hermite a tratti con derivate continue nei nodi ($C^1$).
  * **Traiettoria a 8 di Lissajous:** Il drone pattuglia lo spazio secondo una curva armonica chiusa parametrizzata:
    $$x(t) = x_0 + A \cdot \cos(\omega t)$$
    $$y(t) = y_0 + B \cdot \sin(2\omega t)$$
    Il rapporto delle frequenze $1:2$ genera la classica lemniscata/figura a otto.
  * **Animazione Interattiva di Ammiccamento (Eye-Blink):** Quando la navicella del giocatore sorvola il drone entro una distanza critica ($d < 65\text{ px}$), si attiva un trigger che contrae la scala verticale $Y$ della pupilla da $1.0$ a $0.08$ e ritorno usando una campana sinusoidale:
    $$s_y(t) = \max\left(0.08,\, |\sin(\pi \cdot t_{\text{progress}})|\right)$$
  * **Meccanica di Indebolimento (Boss Mechanic):** Se colpito dal raggio laser, il drone entra in uno stato indebolito di 4 secondi in cui:
    - La velocità di pattugliamento crolla del 65%.
    - Non può eseguire lo scatto predatorio.
    - L'occhio resta socchiuso a fessura.
    - Il fragment shader pulsa con una tinta viola sbiadita (`uColorTint`).
* **2. Cosmic Warp Gate (Portale Dimensionale Monumentale):**
  * Costruito da un anello esterno a curve Hermite (`warp_gate.obj`) che ruota a $22^\circ/\text{s}$ e un vortice interno ciano a gradiente radiale che ruota vorticosamente a $100^\circ/\text{s}$.
  * Inizialmente è dormiente; quando il giocatore raccoglie tutte e 5 le monete d'oro, il portale si illumina di bagliori ciano e attiva il passaggio di vittoria.

---

## 3.8 `src/LaserManager.cpp`, `HazardManager.cpp` e `ParticleSystem.cpp`
* **`LaserManager`:** Gestisce i colpi di plasma sparati dalla barra spaziatrice. I laser avanzano a 950 px/s lungo la traiettoria della navicella. All'impatto con un asteroide, lo distruggono e attivano il `ParticleSystem`.
* **`HazardManager`:** A partire dal livello 2 (e con dardi laser dall'alto dal livello 5), lancia cariche di plasma supersoniche a 880 px/s che attraversano lo schermo orizzontalmente. Prima del lancio, compare una **spia stroboscopica d'allerta** sui bordi per permettere al pilota di reagire con i riflessi.
* **`ParticleSystem`:** Motore particellare per esplosioni rocciose (32 frammenti all'impatto asteroide), scintille dorate (raccolta monete) e fuochi d'artificio per l'HURRÀ.
  * *Attrito viscoso:* $v(t + \Delta t) = v(t) \cdot (1 - \gamma \Delta t)$, che rallenta realisticamente i detriti.
  * *Dissolvenza lineare:* $\text{alpha}(t) = \text{alpha}_0 \cdot \frac{t_{\text{residuo}}}{t_{\text{totale}}}$, disegnata con blending abilitato.

---

## 3.9 `src/Geometry.cpp` e `include/Geometry.h` – Incapsulamento Hardware OpenGL
* **A cosa serve:** È la classe fondamentale che incapsula le risorse GPU della pipeline:
  * `m_vao`: Vertex Array Object.
  * `m_vboPositions`: Vertex Buffer Object per le coordinate spaziali $(x, y, z)$.
  * `m_vboColors`: Vertex Buffer Object per i colori dei vertici $(r, g, b, a)$.
  * `m_primitiveType`: primitiva OpenGL (`GL_TRIANGLES`, `GL_TRIANGLE_FAN`, `GL_LINE_LOOP`).
* **Metodo `upload()`:**
  1. `glGenVertexArrays(1, &m_vao); glBindVertexArray(m_vao);`
  2. `glGenBuffers(1, &m_vboPositions); glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);`
  3. `glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);`
  4. `glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr); glEnableVertexAttribArray(0);` (Location 0 nello shader!)
  5. Ripete per `m_vboColors` su Location 1 con 4 float per vertice.
  6. `glBindVertexArray(0)` (unbind di sicurezza).
* **Metodo `draw()`:**
  `glBindVertexArray(m_vao); glDrawArrays(m_primitiveType, 0, m_vertexCount); glBindVertexArray(0);`
* **Distruttore RAII:** chiama `glDeleteVertexArrays` e `glDeleteBuffers` per azzerare qualsiasi rischio di memory leak sulla VRAM.

---

## 3.10 `src/PlayerShapeLoader.h` – Parser Procedurale Wavefront OBJ/MTL
* **A cosa serve:** Legge da disco i file di geometria poligonale 3D/2D Wavefront `.obj` e i cataloghi colori `.mtl` (spiegati nel laboratorio *Player Geometrie Importate*).
* **Come funziona:**
  * Legge token `v x y z` per estrarre le coordinate geometriche.
  * Legge `usemtl NomeMateriale` per associare la tavolozza definita nel file `.mtl`.
  * Legge `Kd r g b` dal file `.mtl` per estrarre la terna cromatica flat e associarla direttamente ai vertici (`Vertex Color`) in `MeshData.colors`.
  * Legge token `f v1 v2 v3 ... vn`: se la faccia contiene più di 3 vertici (es. quadrilateri o poligoni convessi), esegue una **triangolazione a ventaglio (Triangle Fan)** decomponendo il poligono nei triangoli $(v_1, v_2, v_3), (v_1, v_3, v_4), \dots, (v_1, v_{n-1}, v_n)$.

---

## 3.11 `src/Texture.cpp` e `include/Texture.h` – Caricamento Immagini
* **A cosa serve:** Carica immagini bitmap/PNG/JPG dalla memoria secondaria e crea un oggetto texture 2D in VRAM.
* **Passaggi chiave:**
  1. `stbi_set_flip_vertically_on_load(true)`: inverte l'asse verticale dell'immagine. Questo è necessario perché nei formati grafici (PNG, BMP) l'origine $(0,0)$ è nell'angolo in alto a sinistra, mentre in OpenGL l'origine dello spazio texture $(u,v)$ è in basso a sinistra.
  2. `glGenTextures(1, &m_textureId); glBindTexture(GL_TEXTURE_2D, m_textureId);`
  3. Parametri di campionamento:
     - `glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)`: interpolazione bilineare per il rimpicciolimento.
     - `glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)`: interpolazione bilineare per l'ingrandimento.
     - `glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)`: previene artefatti di ripetizione sui bordi.
  4. `glTexImage2D(...)`: trasferisce i byte dell'immagine nella memoria VRAM della GPU.
  5. `stbi_image_free(data)`: libera immediatamente la RAM di sistema (CPU).

---

## 3.12 `src/Input.cpp` e `include/Input.h` – Polling vs Callback
* **A cosa serve:** Gestisce l'interazione utente mediante GLFW (Domanda 19).
* **Separazione Concettuale:**
  * **Callback (Event-Driven):** `keyCallback`, `cursorPositionCallback`, `framebufferSizeCallback`. Vengono invocate da `glfwPollEvents()` solo quando accade un evento discreto (es. premere 'R' per riavviare, ridimensionare la finestra con `glViewport`).
  * **Polling Continuo:** `glfwGetKey(window, GLFW_KEY_UP)`. Viene interrogato a ogni frame nel game-loop per gestire la spinta continua dei motori, garantendo reattività immediata integrata col $\Delta t$.

---

## 3.13 `include/Collider2D.h` – Matematica delle Collisioni
* **Circle vs Circle (senza radice quadrata):**
  Due cerchi di centro $c_1, c_2$ e raggio $r_1, r_2$ collidono se $\|c_1 - c_2\| \le r_1 + r_2$. Per evitare il costo computazionale della radice quadrata:
  $$\|c_1 - c_2\|^2 \le (r_1 + r_2)^2 \iff \text{dot}(c_1 - c_2, c_1 - c_2) \le (r_1 + r_2)^2$$
* **AABB vs AABB (Teorema degli Assi Separatori):**
  Due rettangoli allineati agli assi si sovrappongono se e solo se si sovrappongono contemporaneamente su entrambi gli assi $X$ e $Y$:
  $$(a_{\text{min}}.x \le b_{\text{max}}.x \land a_{\text{max}}.x \ge b_{\text{min}}.x) \land (a_{\text{min}}.y \le b_{\text{max}}.y \land a_{\text{max}}.y \ge b_{\text{min}}.y)$$
* **Circle vs AABB:** Trova il punto più vicino sul rettangolo tramite clamping del centro del cerchio: $p_{\text{clamped}} = \text{clamp}(c, \text{box}_{\text{min}}, \text{box}_{\text{max}})$ e verifica se $\|c - p_{\text{clamped}}\|^2 \le r^2$.

---

# 4. TUTTI GLI SHADER E GLI ASSET: IL CODICE NON C++ SPIEGATO NEI DETTAGLI

In questa sezione analizziamo tutto ciò che risiede nella cartella `assets/` e che viene compilato sulla scheda video o caricato a runtime.

## 4.1 `assets/shaders/background.vert` e `background.frag` (Lo Shader dello Sfondo)
* **Vertex Shader (`background.vert`):**
  ```glsl
  #version 330 core
  layout(location = 0) in vec2 aPos;
  out vec2 vUV;
  void main() {
      vUV = aPos * 0.5 + 0.5; // Mappa da [-1, 1] a [0, 1] per il campionamento UV
      gl_Position = vec4(aPos, 0.0, 1.0); // Passaggio diretto in NDC
  }
  ```
* **Fragment Shader (`background.frag`):**
  È un generatore procedurale puro (senza texture):
  1. *Funzione Hash Pseudo-Casuale:*
     ```glsl
     float hash(vec2 p) {
         p = fract(p * vec2(123.34, 456.21));
         p += dot(p, p + 45.32);
         return fract(p.x * p.y);
     }
     ```
     Genera numeri pseudo-casuali deterministici sulla GPU partendo dalle coordinate pixel, senza richiedere lookup table in memoria.
  2. *Campo Stellare a 3 Strati di Parallasse (`starLayer`):*
     Divide lo spazio UV in una griglia regolare. Se l'hash della cella supera la soglia di probabilità, calcola la distanza dal centro della cella e modula la luminosità con una funzione di scintillio trigonometrica:
     $$\text{twinkle} = 0.5 + 0.5 \sin(uTime \cdot \text{speed} + h \cdot 2\pi)$$
     Tre strati con scale diverse ($35.0, 75.0, 130.0$) creano la sensazione di profondità (stelle grandi vicine, medie, lontane minuscole).
  3. *Coordinate Polari e Spirale Psichedelica:*
     Calcola raggio $r = \|p\|$ e angolo $\theta = \text{atan}(p_y, p_x)$ rispetto al centro dello schermo per generare onde e vortici di nebulosa.
  4. *Domain Warping Trigonometrico:*
     Deforma le coordinate UV campionando seni e coseni annidati per simulare il moto fluidodinamico dei gas interstellari.
  5. *Palette Swapping tra Livelli:*
     In base all'uniform intera `uLevel`, se il livello è dispari usa Ciano Elettrico e Viola Cosmico; se il livello è pari usa Oro Solare e Smeraldo Aurora Boreale.
  6. *Vignettatura Radiale:*
     Attenua la luce verso i bordi dello schermo con una funzione `smoothstep` per concentrare l'attenzione al centro.

---

## 4.2 `assets/shaders/basic.vert` e `basic.frag` (Lo Shader Universale Oggetti)
* **Vertex Shader (`basic.vert`):**
  ```glsl
  #version 330 core
  layout(location = 0) in vec3 aPosition;
  layout(location = 1) in vec4 aColor;
  uniform mat4 projection;
  uniform mat4 model;
  out vec4 vColor;
  void main() {
      gl_Position = projection * model * vec4(aPosition, 1.0);
      vColor = aColor; // Gouraud shading: trasmette il colore al rasterizzatore
  }
  ```
* **Fragment Shader (`basic.frag`):**
  ```glsl
  #version 330 core
  in vec4 vColor;
  out vec4 FragColor;
  uniform vec4 uColorTint = vec4(1.0, 1.0, 1.0, 1.0);
  void main() {
      FragColor = vColor * uColorTint; // Moltiplicazione per tinta/flash/alpha
  }
  ```

---

## 4.3 `assets/shaders/coin.vert` (Animazione Vertici su GPU)
* Calcola la traslazione oscillante verticale direttamente nei processori di vertice della GPU:
  ```glsl
  vec4 worldPosition = model * vec4(aPosition, 0.0, 1.0);
  worldPosition.y += uAmplitude * sin(uFrequency * uTime + uPhase);
  gl_Position = projection * worldPosition;
  vColor = aColor;
  ```
  Questo alleggerisce la CPU ed è il pattern mostrato nel laboratorio *Mountain Quest* per i cristalli!

---

## 4.4 `assets/shaders/texture.vert` e `texture.frag` (Texture Mapping Cabina)
* Il fragment shader esegue una **maschera ellittica analitica** con `discard`:
  ```glsl
  vec2 uvNorm = (vTexCoord - vec2(0.5, 0.5)) * 2.0;
  float dist = dot(uvNorm, uvNorm);
  if (dist > 1.0) discard; // Scarta i pixel fuori dall'ellisse del cupolino!
  vec4 texColor = texture(uTexture, vTexCoord);
  float edgeAlpha = smoothstep(1.0, 0.82, dist); // Bordo anti-aliased morbido
  FragColor = vec4(texColor.rgb, texColor.a * edgeAlpha) * uColorTint;
  ```

---

## 4.5 Modelli Geometrici OBJ e Tavolozze MTL
* `alien_drone.obj` e `warp_gate.obj`: contengono centinaia di coordinate $(x,y,z)$ che campionano curve spline di Hermite interpolate a tratti.
* `alien_drone.mtl` e `warp_gate.mtl`: contengono i valori cromatici flat `newmtl`, `Kd r g b` e `d alpha` letti dal parser `PlayerShapeLoader`.

---

# 5. MAPPATURA DEI REQUISITI DI PROGETTO UFFICIALI (3.1 – 8)

| Requisito Ufficiale | Descrizione Scheda Docente | Implementazione Esatta in Astro-Miner 2D | File Chiave |
| :--- | :--- | :--- | :--- |
| **3.1 Primitive Composte** | Almeno 1 oggetto composto da $\ge 3$ parti con albero di trasformazione | Navicella Starfighter composta da 11 nodi gerarchici (fusoliera, muso, cabina, ali specchiate con $S(-1,1,1)$, cannoni, fiamme reattive, luci LED) | `Player.cpp`, `Player.h` |
| **3.2 Curve di Hermite** | Almeno 2 geometrie distinte basate su spline cubiche di Hermite, di cui almeno 1 dinamica | 1. **Alien Drone**: nemico ostile su traiettoria a 8 di Lissajous con animazione di ammiccamento occhio (eye-blink);<br>2. **Warp Gate**: portale dimensionale monumentale a due anelli rotanti | `HermiteObject.cpp`, `alien_drone.obj`, `warp_gate.obj` |
| **4. Trasformazioni / Istanze** | Calcolo Model Matrix su CPU, passaggio a GPU via uniform; almeno 3 istanze a geometria condivisa | **Buffer Sharing**: singolo VBO/VAO caricato in VRAM in `GL_STATIC_DRAW` sia per gli asteroidi (4-9 istanze) sia per le monete (5 istanze). La CPU invia solo `model` ad ogni draw call | `AsteroidManager.cpp`, `Collectible.cpp` |
| **5. Due Shader Program** | Almeno 2 shader program con compiti distinti; sfondo procedurale puro senza immagini | 1. **Shader Sfondo**: Fullscreen quad in NDC, stelle a parallasse con hash pseudo-casuale, domain warping e onde polari;<br>2. **Shader Oggetti**: Proiezione ortografica `glm::ortho` e colore Gouraud | `background.frag`, `basic.vert`, `BackgroundRenderer.cpp` |
| **6. Animazioni e $\Delta t$** | Almeno 2 animazioni indipendenti dal framerate; interattive e autonome | **Delta-Time Clamping** a 0.05s;<br>- *Utente:* fiamme reattive all'accelerazione;<br>- *Autonome:* rotazione pseudo-3D monete $s_x=\|\cos(\omega t)\|$, orbita di Lissajous del drone, rotazione anelli Warp Gate | `main.cpp`, `Player.cpp`, `Collectible.cpp`, `HermiteObject.cpp` |
| **7. Interazioni e Collisioni** | Controllo frecce/WASD; raccolta $\ge 5$ oggetti; 3 stati di gioco; Bounding Volumes | Controllo simultaneo Frecce/WASD; raccolta 5 monete d'oro per sbloccare il portale; stati `PLAYING`, `LEVEL_CLEARED_HURRA`, `GAME_OVER`; test Cerchio-Cerchio $d^2 \le (r_1+r_2)^2$ | `GameWorld.cpp`, `Collider2D.h` |
| **8. Organizzazione Codice** | Struttura modulare SoC, zero variabili globali, pattern RAII, C++17, documentazione | Architettura modulare a classi separate, distruttori RAII con `glDelete*`, zero variabili globali esposte, CMake multipiattaforma (macOS, Linux, Windows) | Tutto il progetto |

---

# 6. MAPPATURA SULLE 20 DOMANDE UFFICIALI DEL PARZIALE I

Ecco come collegare direttamente il tuo progetto a ciascuna delle 20 domande d'esame:

### Domanda 1: Architettura della GPU
* **Cosa chiede:** Componenti scheda grafica (GPU, VRAM, color framebuffer, z-buffer), paradigma SIMD/SIMT e vantaggi.
* **Come colleghi il tuo progetto:**
  > *"Nel mio progetto i dati geometrici vengono allocati una sola volta nella VRAM della scheda video all'interno di Vertex Buffer Object (VBO) tramite `glBufferData(GL_ARRAY_BUFFER, ..., GL_STATIC_DRAW)` nella classe `Geometry`. L'architettura SIMD/SIMT della GPU viene sfruttata sia nel vertex shader, dove centinaia di vertici vengono trasformati in parallelo dalla Model Matrix, sia soprattutto nel fragment shader procedurale dello sfondo `background.frag`, dove la GPU calcola simultaneamente per tutti i 921.600 pixel (1280x720) le funzioni di hash e di domain warping. Il risultato viene scritto nel Color Framebuffer e inviato a video con double buffering (`glfwSwapBuffers`). Essendo un progetto 2D con disegno ordinato dal fondo verso la camera, ho disabilitato il depth test per ottimizzare il throughput e ho gestito la visibilità con l'ordine delle draw call."*

### Domanda 2: Grafica raster e grafica vettoriale
* **Cosa chiede:** Differenze tra descrizione vettoriale continua e griglia discreta di pixel; quando usarle.
* **Come colleghi il tuo progetto:**
  > *"Nel progetto convivono entrambi i paradigmi: tutte le entità di gioco (navicella, asteroidi, monete e le curve di Hermite del Drone e del Warp Gate) nascono come descrizioni vettoriali continue basate su vertici ed equazioni matematiche parametriche. Il rasterizzatore della GPU campiona queste primitive continue trasformandole in una matrice discreta di pixel nel color framebuffer a risoluzione 1280x720. Una risorsa prettamente raster è invece la fotografia del pilota `player_face.bmp`, caricata da disco come griglia discreta di campioni RGB e proiettata sulla cabina mediante interpolazione bilineare."*

### Domanda 3: Dalla pipeline fissa a quella programmabile
* **Cosa chiede:** Evoluzione da fixed-function a shader GLSL, motivazioni e vantaggi.
* **Come colleghi il tuo progetto:**
  > *"In `main.cpp` ho impostato il profilo `GLFW_OPENGL_CORE_PROFILE`, escludendo del tutto la pipeline fissa deprecata (`glBegin/glEnd`). Come richiesto dal Requisito 5, ho implementato una pipeline programmabile a shader multipli: `basic.vert/frag` per gli oggetti del mondo, `coin.vert` per animare l'oscillazione dei cristalli direttamente su GPU, `texture.vert/frag` per campionare e ritagliare con formula ellittica il cupolino, e `background.frag` per generare uno sfondo cosmico procedurale matematico che sarebbe stato impossibile da realizzare con una pipeline a funzione fissa."*

### Domanda 4: Stadi della Graphics Rendering Pipeline
* **Cosa chiede:** Application, Geometry Processing, Rasterization; passaggio da scena a immagine.
* **Come colleghi il tuo progetto:**
  > *"Nel mio motore la fase di **Application** risiede in `GameWorld.cpp` sulla CPU: legge gli input da `Input.cpp`, aggiorna la cinematica col delta time, risolve le collisioni con `Collider2D.h` e calcola le matrici Model e Projection. La fase di **Geometry Processing** viene eseguita dai Vertex Shader sulla GPU, calcolando `gl_Position = projection * model * vec4(aPosition, 1.0)`, effettuando il clipping contro il volume canonico $[-1, 1]^3$ e la divisione per $w$. Infine, la **Rasterization** valuta le edge functions per individuare i pixel coperti dai triangoli, interpola i colori di vertice e passa i frammenti al Fragment Shader per la scrittura finale nel framebuffer."*

### Domanda 5: Spazi vettoriali, prodotti e normale
* **Cosa chiede:** Spazio vettoriale, prodotto scalare (significato geometrico/angolo), vettoriale, normale a un triangolo.
* **Come colleghi il tuo progetto:**
  > *"Nel modulo di collisioni `Collider2D.h`, utilizzo il prodotto scalare `glm::dot(diff, diff)` per calcolare la distanza euclidea al quadrato tra la navicella e gli asteroidi: sapendo che $u \cdot u = \|u\|^2$, posso verificare la condizione di impatto $d^2 \le (r_1 + r_2)^2$ senza calcolare la costosa radice quadrata. Inoltre, nella funzione `ShapeGenerator::createCircle`, i vertici sul piano $XY$ possiedono una normale implicita perpendicolare rivolta verso l'osservatore lungo l'asse $Z$, calcolabile come prodotto vettoriale dei lati $n = (B - A) \times (C - A) = (0, 0, 1)$."*

### Domanda 6: Spazi affini, coordinate omogenee e trasformazioni
* **Cosa chiede:** Punti vs vettori ($w=1$ vs $w=0$), matrici di traslazione, rotazione, scala e non-commutatività ($TR \ne RT$).
* **Come colleghi il tuo progetto:**
  > *"In `Player.cpp` e `AsteroidManager.cpp` utilizzo coordinate omogenee a 4 componenti per poter comporre traslazioni, rotazioni e scalature in un'unica matrice $4 \times 4$. L'ordine di moltiplicazione con vettori colonna è rigorosamente:
  $$M = T(x, y) \cdot R(\theta) \cdot S(s_x, s_y)$$
  Questo significa che la geometria locale viene prima scalata, poi ruotata attorno alla propria origine e infine traslata nel mondo. Se invertissi $T$ e $R$, l'oggetto ruoterebbe attorno all'origine del mondo (0,0) con un raggio pari alla distanza di traslazione, anziché ruotare su se stesso. Inoltre, ho sfruttato l'algebra delle trasformazioni affini per generare l'ala sinistra specchiando l'ala destra con la matrice di scala $S(-1, 1, 1)$."*

### Domanda 7: Coordinate baricentriche e convessità
* **Cosa chiede:** Definizione $P = \alpha A + \beta B + \gamma C$, punti interni/esterni, convex hull, interpolazione attributi.
* **Come colleghi il tuo progetto:**
  > *"Nella classe `ShapeGenerator.cpp`, triangolo cerchi, monete e asteroidi tramite un Triangle Fan con un vertice centrale $A$ e vertici perimetrali $B, C$. Assegno un colore dorato brillante al centro e un colore ambrato al bordo. Durante la rasterizzazione hardware del triangolo, la GPU calcola le coordinate baricentriche $(\alpha, \beta, \gamma)$ per ogni frammento interno e calcola il colore interpolato come $C(P) = \alpha C_A + \beta C_B + \gamma C_C$, generando un perfetto gradiente metallico radiale (Gouraud shading)."*

### Domanda 8: Cambiamento di sistema di riferimento
* **Cosa chiede:** Frame locale vs globale, costruzione matrice cambio base, significato di Model e View.
* **Come colleghi il tuo progetto:**
  > *"Nel mio oggetto composto `Player`, ogni singolo componente (fiamme, cannoni, ali, cabina) è modellato in un frame locale $F_{\text{locale}}$ centrato nell'origine della navicella. Per portarli nello spazio del mondo $F_{\text{mondo}}$, la matrice `Model` agisce come matrice di cambio di riferimento: le prime due colonne contengono i vettori di direzione locale della navicella espressi nel sistema globale, mentre la terza colonna rappresenta la posizione attuale del pilota. La matrice composta $M_{\text{finale}} = M_{\text{player}} \cdot M_{\text{locale}}$ trasforma automaticamente i componenti mantenendoli vincolati alla navicella."*

### Domanda 9: Sottosistema geometrico: Model, View, Projection
* **Cosa chiede:** Catena $Local \to World \to View \to Clip \to NDC \to Viewport$, differenza ortografica vs prospettica.
* **Come colleghi il tuo progetto:**
  > *"In `GameWorld.cpp` utilizzo una matrice di proiezione ortografica costruita con `glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f)`. A differenza della proiezione prospettica (usata nei mondi 3D dove la coordinata $z$ divide $x$ e $y$ facendo rimpicciolire gli oggetti lontani), la proiezione ortografica mantiene i raggi di proiezione paralleli: la dimensione degli oggetti è invariante rispetto alla profondità, rendendola ideale per un gioco 2D. Nel vertex shader `basic.vert`, il vertice attraversa la catena:
  $$P_{\text{clip}} = \mathbf{P} \cdot \mathbf{M} \cdot P_{\text{local}}$$
  La GPU divide per $w=1$ ottenendo lo spazio canonico NDC $[-1, 1]$, che la viewport trasforma infine nei pixel dello schermo $[0, 1280] \times [0, 720]$."*

### Domanda 10: Clipping e algoritmo di Cohen-Sutherland
* **Cosa chiede:** Ruolo del clipping, outcode a 4 bit (sopra, sotto, destra, sinistra), criteri di accettazione/rifiuto e taglio parametrico.
* **Come colleghi il tuo progetto:**
  > *"Nella pipeline OpenGL il clipping hardware avviene nello spazio omogeneo di clip prima della divisione prospettica, eliminando automaticamente i frammenti dei laser o delle particelle che escono dal volume di vista $[-w, w]^3$. A livello logico, in `LaserManager.cpp` e `HazardManager.cpp` implemento un clipping software analogo: calcolo la bounding box della finestra $[0, 1280] \times [0, 720]$ e, se la posizione supera i limiti (outcode non nullo), dealloco immediatamente il proiettile dal vettore per risparmiare memoria e cicli CPU."*

### Domanda 11: Curve parametriche e condizioni di continuità
* **Cosa chiede:** Curva regolare, tangente $C'(t)$, velocità scalare, lunghezza d'arco, continuità $G^0, C^0, G^1, C^1$.
* **Come colleghi il tuo progetto:**
  > *"Nel mio progetto ho implementato curve parametriche sia per le geometrie sia per le traiettorie cinematiche. La traiettoria del nemico `Alien Sentinel Drone` in `HermiteObject.cpp` è una curva chiusa parametrica di Lissajous:
  $$C(t) = (x_0 + A\cos(\omega t),\, y_0 + B\sin(2\omega t))$$
  Il vettore velocità tangente è dato dalla derivata prima $C'(t) = (-A\omega\sin(\omega t),\, 2B\omega\cos(2\omega t))$. Poiché le derivate non si annullano simultaneamente, la curva è regolare e di classe $C^1$, garantendo che il drone si muova con assoluta fluidità senza cambi bruschi di velocità o spigoli."*

### Domanda 12: Curve interpolanti di Hermite
* **Cosa chiede:** Polinomio cubico $P_3(t)$, imposizione delle 4 condizioni agli estremi, funzioni base $h_{00}, h_{01}, h_{10}, h_{11}$, continuità $C^1$ nei nodi.
* **Come colleghi il tuo progetto:**
  > *"Per soddisfare il Requisito 3.2 ho integrato due modelli poligonali costruiti con spline cubiche di Hermite: l'Alien Drone e il Warp Gate. Una curva cubica di Hermite è definita da due punti $P_0, P_1$ e due tangenti $D_0, D_1$ tramite le quattro funzioni base:
  $$C(t) = h_{00}(t)P_0 + h_{01}(t)P_1 + h_{10}(t)D_0 + h_{11}(t)D_1$$
  dove $h_{00}(t) = 2t^3 - 3t^2 + 1$, $h_{01}(t) = -2t^3 + 3t^2$, $h_{10}(t) = t^3 - 2t^2 + t$ e $h_{11}(t) = t^3 - t^2$. Per raccordare più tratti garantendo la continuità parametrica $C^1$, ogni tratto condivide con quello adiacente non solo il punto di controllo comune ($C_{i-1}(t_i) = C_i(t_i)$), ma anche il medesimo vettore tangente derivata ($C'_{i-1}(t_i) = C'_i(t_i)$). Questo garantisce un profilo perfettamente liscio per le corna del drone e per l'anello del portale."*

### Domanda 13: Rasterizzazione di segmenti e circonferenze
* **Cosa chiede:** DDA vs Bresenham per segmenti (variabile di decisione $d$ intera); Midpoint per circonferenze ($d_0 = 1 - R$ e simmetria a 8 ottanti).
* **Come colleghi il tuo progetto:**
  > *"Mentre il DDA richiede calcoli in virgola mobile per aggiornare le coordinate, l'algoritmo Midpoint di Bresenham opera con sola aritmetica intera. Per la circonferenza, sfrutta la funzione implicita $F(x,y) = x^2 + y^2 - R^2 = 0$ e valuta il punto medio tra il pixel Est e Sud-Est partendo da $d_0 = 1 - R$. Calcola solo un ottante ($45^\circ$) e ricava gli altri 7 per simmetria riflessiva. Nel mio codice, in `ShapeGenerator::createCircle`, genero la geometria circolare suddividendola analiticamente in $N$ settori angolari, lasciando alla rasterizzazione hardware della GPU l'accensione dei pixel tramite interpolazione dei triangoli."*

### Domanda 14: Rasterizzazione dei triangoli
* **Cosa chiede:** Algoritmo delle edge functions $E_0, E_1, E_2$, test al centro del pixel $(i+0.5, j+0.5)$, coordinate baricentriche per interpolare colore e attributi.
* **Come colleghi il tuo progetto:**
  > *"Tutte le mesh del mio gioco (quadrettatura navicella, cerchi, asteroidi, monete) vengono decomposte in triangoli con orientamento antiorario dei vertici. La GPU visita il bounding box del triangolo e calcola le tre edge functions lineari:
  $$E_i(x, y) = (x - x_0)(y_1 - y_0) - (y - y_0)(x_1 - x_0)$$
  valutate al centro del pixel $(i + 0.5, j + 0.5)$. Se tutte e tre sono $\le 0$, il centro del pixel cade dentro la primitiva. A quel punto la GPU calcola i pesi baricentrici $\alpha, \beta, \gamma$ e interpola colore e coordinate UV che vengono passati in ingresso al Fragment Shader."*

### Domanda 15: Aliasing e anti-aliasing
* **Cosa chiede:** Campionamento a singolo punto, copertura parziale del pixel, SSAA, MSAA, FXAA.
* **Come colleghi il tuo progetto:**
  > *"L'aliasing si verifica perché un test binario al centro del pixel non misura la copertura reale del frammento, creando bordi a scalettatura ('jaggies'). Per mitigare questo fenomeno nel mio progetto ho adottato tre strategie:
  1. Su texture e cupolino (`texture.frag`), utilizzo la funzione GLSL `smoothstep(1.0, 0.82, dist)` che crea una rampa di transizione continua dell'alpha sui bordi del cerchio, simulando un anti-aliasing analitico a livello di sub-pixel.
  2. Nello shader procedurale dello sfondo `background.frag`, le stelle sono attenuate con `smoothstep(0.12, 0.0, d)` per non avere spigoli netti.
  3. A livello di contesto GLFW è possibile abilitare il Multi-Sample Anti-Aliasing (MSAA a 4 campioni con `glfwWindowHint(GLFW_SAMPLES, 4)`), che campiona la copertura su 4 sub-pixel per ogni bordo geometrico."*

### Domanda 16: Struttura di un'applicazione OpenGL
* **Cosa chiede:** Inizializzazione, ciclo di rendering, rilascio risorse, ruoli di GLFW, GLAD e GLM.
* **Come colleghi il tuo progetto:**
  > *"La struttura del mio file `main.cpp` rispecchia fedelmente l'architettura standard:
  1. **GLFW:** Crea la finestra del sistema operativo, alloca il contesto OpenGL 3.3 Core Profile e gestisce la coda eventi e il double buffering con `glfwSwapBuffers`.
  2. **GLAD:** Inizializzato subito dopo aver reso corrente il contesto con `gladLoadGLLoader`, carica gli indirizzi a runtime delle funzioni OpenGL moderne dal driver grafico.
  3. **GLM:** Libreria matematica header-only che fornisce i tipi vettoriali (`glm::vec2, vec3, vec4`) e matriciali (`glm::mat4`), nonché le funzioni di trasformazione affine (`translate, rotate, scale, ortho`).
  Il ciclo di rendering legge gli input, calcola il delta time clampato, aggiorna la fisica, pulisce il buffer colore con `glClear(GL_COLOR_BUFFER_BIT)`, invia le draw call e scambia i buffer."*

### Domanda 17: VAO, VBO, EBO: geometria e istanza
* **Cosa chiede:** Ruoli di VBO (dati vertici), VAO (stato configurazione attributi), EBO (indici), differenza tra geometria e istanza.
* **Come colleghi il tuo progetto:**
  > *"La classe `Geometry.cpp` gestisce il VBO (che contiene l'array grezzo di posizioni e colori caricati con `glBufferData`) e il VAO (che memorizza la configurazione registrata con `glVertexAttribPointer` e abilitata con `glEnableVertexAttribArray`). Il VAO non contiene vertici fisici, ma la 'ricetta' con cui la GPU deve interpretare la memoria del VBO. Per quanto riguarda la distinzione tra Geometria e Istanza, in `AsteroidManager.cpp` esiste una sola `Geometry` condivisa (un unico VAO e VBO in VRAM), mentre ciascun asteroide nella scena rappresenta una diversa `Instance`, caratterizzata dalla propria posizione, rotazione, scala e quindi da una distinta matrice `uniform mat4 model` calcolata a ogni frame."*

### Domanda 18: Vertex shader, fragment shader e uniform
* **Cosa chiede:** Ruoli dei due stadi GLSL, differenze tra attribute (in), varying (out/in) e uniform; passaggio matrici da CPU.
* **Come colleghi il tuo progetto:**
  > *"Un Vertex Shader (es. `basic.vert`) elabora ogni singolo vertice in parallelo: riceve come attributi `in` posizione e colore dal VAO, moltiplica la posizione per le matrici Model e Projection e invia all'esterno la variabile `out vec4 vColor`. Il rasterizzatore interpola `vColor` e lo passa come `in vec4 vColor` al Fragment Shader (`basic.frag`), che restituisce il colore finale del pixel `FragColor`. Le variabili `uniform` (come `projection`, `model`, `uTime`, `uColorTint`) sono invece parametri costanti globali per tutti i vertici e frammenti di una data draw call, e vengono inviate dalla CPU alla GPU tramite chiamate come `glUniformMatrix4fv` o `glUniform1f` della classe `Shader.cpp`."*

### Domanda 19: Input, eventi e organizzazione in classi
* **Cosa chiede:** Polling vs callback con GLFW, separazione delle responsabilità tra logica, geometria, shader, rendering e interfaccia.
* **Come colleghi il tuo progetto:**
  > *"Il progetto separa nettamente le responsabilità:
  - `Input.cpp` e GLFW gestiscono gli eventi discreti tramite callback (`keyCallback` per restart o uscita) e gli eventi continui tramite polling (`glfwGetKey` per la propulsione della navicella a ogni frame).
  - `GameWorld.cpp` coordina la logica di gioco, lo stato FSM e le collisioni.
  - `Geometry.cpp` incapsula i buffer hardware (VBO/VAO).
  - `Shader.cpp` compila e gestisce i programmi GLSL.
  - `ShapeGenerator.cpp` calcola le coordinate matematiche delle mesh su CPU.
  - `ImGui` gestisce l'interfaccia utente (Mission Control HUD) in modo non invasivo, controllando `!io.WantCaptureMouse` per non bloccare l'input di gioco durante l'interazione con gli slider."*

### Domanda 20: RGB, alpha, blending e trasparenza
* **Cosa chiede:** Modello RGB, canale alpha come opacità, equazione di blending $C_{out} = F_s C_s + F_d C_d$, `glBlendFunc`, ordine di rendering dal fondo alla camera.
* **Come colleghi il tuo progetto:**
  > *"Nel mio progetto i colori sono definiti nel modello RGBA normalizzato $[0, 1]^4$. In `main.cpp` ho abilitato il blending con `glEnable(GL_BLEND)` e impostato la funzione standard:
  $$\text{glBlendFunc}(\text{GL\_SRC\_ALPHA}, \text{GL\_ONE\_MINUS\_SRC\_ALPHA})$$
  che corrisponde all'equazione algebrica:
  $$C_{\text{out}} = \alpha_s \cdot C_s + (1 - \alpha_s) \cdot C_d$$
  dove $C_s$ è il colore sorgente del frammento con opacità $\alpha_s$, e $C_d$ è il colore già presente nel framebuffer. Per garantire la correttezza visiva della trasparenza ed evitare artefatti, l'ordine di rendering in `GameWorld::render` è rigorosamente **Back-to-Front**:
  1. Prima lo Sfondo procedurale opaco;
  2. Poi gli oggetti solidi (portale, asteroidi, monete, nemico);
  3. Infine la navicella e tutti gli elementi traslucidi (fiamme del reattore, luci stroboscopiche LED e il sistema particellare di fumo e scintille con alpha decadente)."*

---

# 7. GUIDA STRATEGICA ALL'ESPOSIZIONE DELLE 10 SLIDE E ALLA DEMO LIVE

Per il testo completo parola per parola da pronunciare durante il colloquio e per le risposte alle domande a trabocchetto su ogni singola slide, consulta il documento complementare:
👉 [`guida_presentazione_canva.md`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/doc/guide/guida_presentazione_canva.md)

### 7.1 Mappa Sinottica delle 10 Slide Definitive (5-7 Minuti Totali)

| Slide | Titolo & Contenuto Chiave | Concetto CG & Formula da Citare | Focus da Enfatizzare |
| :--- | :--- | :--- | :--- |
| **1** | **Copertina & Titolo** (*Astro-Miner 2D*) | OpenGL 3.3 Core Profile, pipeline programmabile | Zero pipeline fissa, portabilità nativa macOS/Win |
| **2** | **Obiettivi Didattici & Specifiche** | Modellazione gerarchica, Hermite $C^1$, VRAM static | Conformità 100% ai requisiti d'esame |
| **3** | **Architettura Software & Game Loop** | Separation of Concerns (SoC), delta time clamping $\Delta t$ | Zero variabili globali, RAII, disaccoppiamento |
| **4** | **Oggetto Composto (Requisito 3.1)** | $M_{\text{comp}} = M_{\text{player}} \cdot T \cdot R \cdot S$, specularità $S(-1,1,1)$ | 8 componenti logici (11 nodi), nessun vertice duplicato |
| **5** | **Curve di Hermite (Requisito 3.2)** | $p(t) = \sum h_{ij}(t) P_j$, raccordo $C^1$ ($C = C, C' = C'$) | Alien Drone (113 vtx) su Lissajous e Warp Gate (144 vtx) |
| **6** | **Buffer Statici & Istanziazione (Req. 4)** | `GL_STATIC_DRAW`, $v_{\text{clip}} = P \cdot M \cdot v_{\text{loc}}$ | 1 solo VAO statico per 9 asteroidi: zero bus transfer |
| **7** | **Shader Sfondo vs Shader Oggetti (Req. 5)** | Starfield $h(p) = \text{fract}(\sin(p \cdot K) \cdot 43758.54)$, $P_{\text{ortho}}$ | Sfondo procedurale matematico in NDC puro $[-1,1]$ |
| **8** | **Gameplay, Fisica & Collisioni** | $\|c_1 - c_2\|^2 \le (r_1+r_2)^2$, attrito $v(t+\Delta t) = v(1-\gamma\Delta t)$ | Nessuna radice quadrata, 28 schegge particellari |
| **9** | **Portabilità Totale & HUD ImGui** | GLFW/GLAD/GLM bundled, `!io.WantCaptureMouse` | Compilazione out-of-the-box, slider velocità nave |
| **10** | **Chiusura & Avvio Demo Live** | Integrazione completa rendering + fisica + input | Transizione fluida e sicura alla prova al computer |

---

### 7.2 Scaletta Temporale per l'Esposizione Orale

* **Minuti 0:00 – 1:00 (Slide 1–3) — Inquadramento e Architettura:**
  Saluta con sicurezza, dichiara le tecnologie utilizzate (C++17, OpenGL 3.3 Core Profile) ed esponi la struttura ad alto livello del motore grafico: pattern RAII, disaccoppiamento dei layer (Core, Graphics, Scene, FSM) e delta-time clamping ($\Delta t \le 0.05\text{s}$) per evitare tunneling e jittering.
* **Minuti 1:00 – 2:30 (Slide 4–5) — Geometria, Grafo Gerarchico e Hermite:**
  Spiega l'albero matriciale della navicella sottolineando l'eleganza della specchiatura alare sinistra tramite $S(-1, 1, 1)$, che dimezza i vertici memorizzati. Passa subito alla formulazione analitica delle spline cubiche di Hermite, scrivendo o indicando le 4 funzioni base $h_{00}, h_{01}, h_{10}, h_{11}$ e spiegando come la continuità parametrica $C^1$ garantisca la perfetta tangenza sia nelle corna dell'Alien Drone che nella corona del Warp Gate.
* **Minuti 2:30 – 3:45 (Slide 6–7) — Ottimizzazione della Pipeline e Shader GLSL:**
  Illustra il principio cardine del Requisito 4: buffer sharing estremo, con un singolo VAO caricato in VRAM all'avvio e riutilizzato per disegnare tutti gli asteroidi e le monete semplicemente aggiornando la matrice uniforme `model` via `glUniformMatrix4fv`. Descrivi la differenza concettuale tra lo shader universale degli oggetti (proiezione ortografica `glm::ortho`) e lo shader dello sfondo procedurale (operante in NDC $[-1,1]$ puro con parallasse a 3 livelli e funzioni hash pseudo-casuali senza alcuna texture).
* **Minuti 3:45 – 4:45 (Slide 8–9) — Dinamica, Collisioni, HUD e Portabilità:**
  Mostra l'ottimizzazione del calcolo delle collisioni con la distanza euclidea al quadrato, la dinamica particellare con decadimento viscoso e alpha blending, l'HUD in Dear ImGui protetto da `io.WantCaptureMouse` e la portabilità garantita dalle librerie ausiliarie incluse come sorgenti.
* **Minuti 4:45 – Fine (Slide 10 & Demo Live):**
  Chiudi ringraziando per l'attenzione e avvia la demo interattiva per mostrare visivamente quanto esposto.

---

### 7.3 I 6 Passi Obbligatori della Demo Live

1. **Avvio:** Lancia `./bin/Interactive2DWorld` da terminale (o da Visual Studio).
2. **Guida & Manovra:** Usa le frecce per muovere la nave: fai notare l'inerzia dolce e l'allungamento progressivo delle fiamme arancioni degli scarichi.
3. **Monete:** Raccogli una moneta d'oro: evidenzia l'oscillazione pseudo-3D ortogonale ($s_x = \|\cos(\omega t)\|$) e il burst di scintille particellari dorate.
4. **Laser & Asteroidi:** Premi `Spazio` per sparare: distruggi un asteroide e mostra la frantumazione in 28 schegge con attrito viscoso e dissolvenza alpha.
5. **Alien Drone & Eye-Blink:** Mostra l'Alien Drone rosso che pattuglia la mappa sulla traiettoria a 8 di Lissajous; volaci sopra e fai notare chiaramente che **l'occhio dorato si chiude e ammicca (eye-blink)** ad ogni contatto!
6. **Vittoria:** Raccogli le 5 monete, vola dentro il Warp Gate luminoso e mostra la schermata centrata di **"HURRÀ!"** con la fontana di coriandoli festosi!

---
*Documento redatto per la preparazione dell'Esame di Computer Graphics – UniBo.*
