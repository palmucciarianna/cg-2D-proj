# Computer Graphics – Progetti di Laboratorio (UniBo A.A. 2025/2026)

**Studente:** Arianna Palmucci  
**Corso:** Computer Graphics – Università di Bologna (Prof.ssa Damiana Lazzaro)

---

## Mini Descrizione (`proj-2D/`)

**Astro-Miner 2D: Cosmic Coin Rush** è un mondo 2D interattivo e videogioco arcade sviluppato in **C++17** e **OpenGL 3.3 Core Profile**.  
Il giocatore pilota una navicella mineraria composita (modellata gerarchicamente) per raccogliere **5 monete d'oro cosmiche**, schivando o distruggendo asteroidi, dardi di plasma perimetrali e un drone alieno sentinella (modellato con spline di Hermite su orbita di Lissajous), fino ad attivare e attraversare il portale **Warp Gate** finale.

### Struttura del Repository
* **`proj-2D/`**: Progetto Elaborato 1 (Midterm 2D) — codice sorgente (`include/`, `src/`), shader e modelli (`assets/`), dipendenze (`external/`), documentazione (`doc/`).
* **`consegna/`**: Versione pulita per la consegna d'esame con script di test rapido.
* **`proj-3D/`**: Progetto Elaborato 2 (Scena 3D e illuminazione) → [palmucciarianna/cg-3D-proj](https://github.com/palmucciarianna/cg-3D-proj.git)
* **`proj-blender/`**: Modellazione e rendering avanzato → [palmucciarianna/cg-blender-proj](https://github.com/palmucciarianna/cg-blender-proj.git)

---

## Tecnologie Usate e Ruolo nel Progetto

Tutte le librerie di terze parti sono incluse direttamente in [`proj-2D/external/`](proj-2D/external) (**zero dipendenze esterne da installare**):

| Tecnologia | A cosa serve nel progetto |
| :--- | :--- |
| **C++17** | Linguaggio principale; architettura modulare senza variabili globali, gestione risorse GPU tramite **RAII** e **Rule of Five**. |
| **OpenGL 3.3 Core Profile** | Pipeline grafica programmabile moderna (esclude le funzioni legacy `glBegin`/`glEnd`), VAO/VBO statici (`GL_STATIC_DRAW`) e alpha blending (`GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`). |
| **GLSL (`#version 330 core`)** | Shader dedicati su GPU: sfondo procedurale senza texture (`background.vert/.frag`), oggetti 2D (`basic.vert/.frag`) e ritaglio ellittico del ritratto pilota (`texture.vert/.frag`). |
| **GLFW** | Creazione della finestra (`1280x720`), contesto OpenGL, **V-Sync**, **Double Buffering** (`glfwSwapBuffers`), `glfwGetTime()` per il Delta Time e gestione input/eventi (callback + polling). |
| **GLAD** | Caricamento dinamico a runtime dei puntatori alle funzioni OpenGL 3.3 dal driver della GPU (`gladLoadGLLoader`). |
| **GLM** | Algebra lineare 2D/3D: vettori (`vec2`, `vec3`, `vec4`), matrice di proiezione ortografica (`glm::ortho`), matrici di modellazione (`glm::translate`, `rotate`, `scale`) e prodotto scalare (`glm::dot`) per le collisioni. |
| **Dear ImGui** | HUD interattivo in *Immediate Mode* (vite, monete, FPS/$\Delta t$, slider velocità ed effetto psichedelico, toggle *Wireframe*, hot-reload texture cabina e banner centrati), con `io.IniFilename = nullptr`. |
| **stb_image** | Decodifica da disco della foto del pilota (`player_face.bmp/.png/.jpg`) con flip verticale (`stbi_set_flip_vertically_on_load`) prima del caricamento in VRAM (`glTexImage2D`). |
| **CMake** | Sistema di build multipiattaforma per generare ed eseguire il progetto su macOS, Windows e Linux. |

---

## Come Lanciarlo su Ogni Sistema Operativo

### 1. macOS (Apple Silicon & Intel)
* **Doppio click:** apri [`proj-2D/AstroMiner.app`](proj-2D/AstroMiner.app) oppure [`proj-2D/Avvia_AstroMiner.command`](proj-2D/Avvia_AstroMiner.command).
* **Da terminale (CMake + `clang++`):**
  ```bash
  cd proj-2D
  cmake -B build
  cmake --build build
  ./build/Interactive2DWorld
  ```

### 2. Windows (10 / 11 – Visual Studio 2022)
* **Doppio click:** esegui [`proj-2D/Avvia_AstroMiner.bat`](proj-2D/Avvia_AstroMiner.bat).
* **Da terminale (CMake):**
  ```cmd
  cd proj-2D
  cmake -B build -G "Visual Studio 17 2022" -A x64
  cmake --build build --config Release
  .\build\Release\Interactive2DWorld.exe
  ```
  *(Oppure apri `proj-2D/build/Interactive2DWorld.sln` in Visual Studio e premi **F5**.)*

### 3. Linux (Ubuntu / Debian / Arch)
* **Da terminale (CMake + GCC/Clang):**
  ```bash
  cd proj-2D
  cmake -B build
  cmake --build build
  ./build/Interactive2DWorld
  ```

---

Per la documentazione tecnica completa e i dettagli su comandi e requisiti, consulta [`proj-2D/README.md`](proj-2D/README.md) e [`proj-2D/doc/relazione/relazione_sintetica.pdf`](proj-2D/doc/relazione/relazione_sintetica.pdf).
