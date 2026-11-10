# Guida all'Avvio Multipiattaforma e Architettura di CMakeLists.txt
## Progetto *Astro-Miner 2D: Cosmic Coin Rush* (Computer Graphics - UniBo)

Questo documento fornisce:
1. **La guida operativa per compilare e avviare il gioco** su **macOS**, **Windows** e **Linux**, sia tramite interfaccia grafica a doppio click sia tramite terminale da riga di comando.
2. **L'analisi dettagliata e didattica riga per riga di `CMakeLists.txt`**, spiegando le scelte architetturali, la gestione delle dipendenze autonome e il processo di build cross-platform.

---

# PARTE 1: Come Lanciare il Progetto in Base al Sistema Operativo

Il progetto è progettato con una filosofia **"zero dipendenze esterne"**: tutte le librerie necessarie (GLAD, GLM, Dear ImGui, stb_image e le librerie statiche GLFW per Mac e Windows a 64 bit) sono già pre-compilate o incluse come codice sorgente nella cartella [`external/`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/external).  
Non è necessario installare alcun package manager esterno (Homebrew, vcpkg, Conan).

---

## 1.1 macOS (Apple Silicon M1/M2/M3/M4 e Intel x86_64)

### Metodo A: Avvio Diretto ad Icona (Consigliato per la dimostrazione)
Nella cartella `proj-2D/` è presente un **App Bundle nativo macOS**:
1. Apri la cartella `proj-2D` nel **Finder**.
2. Fai doppio click su **`AstroMiner.app`**.
3. Il gioco si apre istantaneamente a tutto schermo/finestra. Se il binario non era ancora stato compilato, lo script interno di `AstroMiner.app` invoca CMake in background e lo avvia senza mostrare schermate di terminale.

> *Alternativa cliccabile con log:* Puoi fare doppio click su **`Avvia_AstroMiner.command`**. Questo aprirà una finestra del Terminale mostrando i log di inizializzazione OpenGL e avvierà il gioco. Se macOS segnala che il file non è eseguibile, digita una sola volta nel terminale: `chmod +x Avvia_AstroMiner.command`.

### Metodo B: Compilazione Manuale da Riga di Comando (Terminale)
Prerequisiti: Xcode Command Line Tools (installabili con `xcode-select --install`) e `cmake`.

```bash
# 1. Posizionati nella directory del progetto
cd proj-2D

# 2. Configura il progetto generando la cartella di build
cmake -B build

# 3. Compila l'eseguibile C++17
cmake --build build

# 4. Avvia l'applicazione
./build/Interactive2DWorld
```

---

## 1.2 Windows (Windows 10 / Windows 11)

### Metodo A: Avvio Diretto con Doppio Click
1. Apri la cartella `proj-2D` in Esplora File.
2. Fai doppio click sul file **`Avvia_AstroMiner.bat`**.
3. Lo script batch rileva l'ambiente Visual Studio, compila automaticamente il progetto in modalità Release e avvia `Interactive2DWorld.exe`.

### Metodo B: Compilazione con Visual Studio 2022 (Consigliato per la consegna d'esame)
Il regolamento d'esame richiede che il progetto sia configurabile con Visual Studio:
1. Apri il prompt dei comandi (**Developer Command Prompt for VS 2022** o PowerShell) e posizionati in `proj-2D`:
   ```cmd
   cd proj-2D
   cmake -B build -G "Visual Studio 17 2022" -A x64
   ```
2. Questo comando genera il file di soluzione Microsoft Visual Studio: `build/Interactive2DWorld.sln`.
3. Fai doppio click su **`Interactive2DWorld.sln`** per aprirlo in Visual Studio.
4. Imposta la configurazione su **Release** (oppure **Debug**) e piattaforma **x64**.
5. Clicca con il tasto destro su **Interactive2DWorld** in *Esplora soluzioni* e seleziona **"Imposta come progetto di avvio"**.
6. Premi **F5** (Avvia debug) oppure **Ctrl+F5** (Avvia senza eseguire debug).

### Metodo C: Compilazione Rapida da Riga di Comando (PowerShell / CMD)
```cmd
cd proj-2D
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
.\build\Release\Interactive2DWorld.exe
```

---

## 1.3 Linux (Ubuntu, Debian, Fedora, Arch)

Su Linux, le librerie statiche binarie di GLFW variano in base alla versione del server grafico (X11 vs Wayland). `CMakeLists.txt` rileva automaticamente se GLFW è installata sul sistema o la compila tramite fallback.

### Prerequisiti (Dipendenze di sistema standard per X11/OpenGL):
```bash
# Su Debian / Ubuntu / Mint:
sudo apt-get update
sudo apt-get install build-essential cmake libglfw3-dev libgl1-mesa-dev

# Su Fedora / RedHat:
sudo dnf install gcc-c++ cmake glfw-devel mesa-libGL-devel
```

### Compilazione ed Esecuzione:
```bash
cd proj-2D
cmake -B build
cmake --build build
./build/Interactive2DWorld
```

---

# PARTE 2: Spiegazione Dettagliata di `CMakeLists.txt` Riga per Riga

Il file [`CMakeLists.txt`](file:///Users/ariannapalmucci/Desktop/cg-project/proj-2D/CMakeLists.txt) è il copione di generazione universale per CMake. È suddiviso in due sezioni logiche: **configurazione delle librerie** e **creazione del target di gioco**. Di seguito ne viene analizzata ogni singola istruzione.

```cmake
cmake_minimum_required(VERSION 3.16)
project(Interactive2DWorld VERSION 1.0 LANGUAGES C CXX)
```
* **Riga 1 (`cmake_minimum_required`):** Dichiara la versione minima di CMake richiesta (3.16). La versione 3.16 introduce il supporto completo per i target `INTERFACE` moderni, il comando `copy_directory` con generator expressions e la compatibilità nativa con i compilatori Clang su Apple Silicon e MSVC 2022 su Windows.
* **Riga 2 (`project`):** Definisce il nome del progetto (`Interactive2DWorld`), la versione formale (1.0) e i linguaggi di programmazione richiesti: **C** (necessario per compilare `glad.c`) e **CXX** (C++ moderno per il resto della logica).

---

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```
* **Righe 4-5:** Impongono al compilatore lo standard **C++17** in modo obbligatorio (`REQUIRED ON`). Il C++17 è necessario nel nostro codice per funzionalità avanzate come `std::make_unique`, *structured bindings*, *constexpr if* e per la compatibilità con le versioni moderne di GLM.

---

### Gestione del Package OpenGL di Sistema

```cmake
find_package(OpenGL REQUIRED)
```
* **Riga 13:** Cerca i file di intestazione e le librerie di base OpenGL fornite dal sistema operativo ospitante (`OpenGL32.lib` su Windows, `libGL.so` su Linux, framework di sistema su Mac). La parola chiave `REQUIRED` interrompe la configurazione con un errore chiaro se la macchina è priva di supporto OpenGL.

---

### Gestione Autonoma di GLM (OpenGL Mathematics)

```cmake
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glm/glm/glm.hpp")
    add_library(glm_bundled INTERFACE)
    target_include_directories(glm_bundled INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/external/glm")
    set(GLM_TARGET glm_bundled)
    message(STATUS "Usando GLM incluso nel progetto (external/glm)")
else()
    find_package(glm REQUIRED)
    set(GLM_TARGET glm::glm)
endif()
```
* **Righe 16-24:** GLM è una libreria puramente *header-only* (costituita da soli file `.hpp` contenenti template per vettori `glm::vec2, vec3, vec4` e matrici `glm::mat4`).
* Lo script verifica se GLM è presente nella cartella locale `external/glm`. In caso affermativo, crea una libreria virtuale di tipo `INTERFACE` (`glm_bundled`), che indica a CMake di aggiungere la directory degli include a chiunque la colleghi. Se non fosse presente, ricorre a `find_package(glm)` come fallback.

---

### Gestione Multipiattaforma di GLFW (Creazione Finestra e Input)

```cmake
if(APPLE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib/macos/libglfw3.a")
    add_library(glfw_bundled STATIC IMPORTED)
    set_target_properties(glfw_bundled PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib/macos/libglfw3.a"
    )
    set(GLFW_TARGET glfw_bundled)
elseif(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib/windows/glfw3.lib")
    add_library(glfw_bundled STATIC IMPORTED)
    set_target_properties(glfw_bundled PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib/windows/glfw3.lib"
    )
    set(GLFW_TARGET glfw_bundled)
else()
    find_package(glfw3 QUIET)
    ...
endif()
```
* **Righe 27-58:** Questa è una delle parti più strategiche per la portabilità:
  - **Su macOS (`APPLE`):** Viene importata la libreria statica già compilata ad architettura universale (x86_64 e arm64 Apple Silicon) `libglfw3.a` situata in `external/glfw/lib/macos/`.
  - **Su Windows (`WIN32`):** Viene importata la libreria statica a 64 bit per Visual Studio `glfw3.lib` situata in `external/glfw/lib/windows/`.
  - In questo modo, sia su macOS che su Windows **non serve installare GLFW separatamente**.
  - **Su Linux (ramo `else`):** Si cerca la libreria `glfw3` di sistema con `find_package` o cercando i percorsi in `/usr/include` e `/usr/lib`.

---

### Compilazione di GLAD (OpenGL Loader-Generator)

```cmake
add_library(glad STATIC
    external/glad/src/glad.c
)
target_include_directories(glad PUBLIC
    external/glad/include
)
```
* **Righe 61-66:** GLAD è il caricatore che interroga il driver della scheda grafica a runtime per ottenere gli indirizzi di memoria di tutte le funzioni di OpenGL 3.3 Core Profile (`glGenVertexArrays`, `glCompileShader`, `glDrawArrays`, ecc.).
* Viene compilato come libreria statica a partire dal sorgente `external/glad/src/glad.c`, esponendo pubblicamente la cartella `external/glad/include` in modo che chi collega `glad` possa includere `<glad/glad.h>`.

---

### Compilazione di Dear ImGui (Interfaccia Grafica Utente)

```cmake
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
    external/glad/include
)
target_link_libraries(imgui PUBLIC ${GLFW_TARGET} glad)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(imgui PRIVATE -Wno-nontrivial-memcall)
endif()
```
* **Righe 69-85:** Compila la libreria `Dear ImGui` includendo sia i file core (`imgui.cpp`, `imgui_draw.cpp`, `imgui_widgets.cpp`), sia i due backend ufficiali scelti per il progetto:
  - `imgui_impl_glfw.cpp`: per intercettare mouse, tastiera e ridimensionamento finestra tramite GLFW.
  - `imgui_impl_opengl3.cpp`: per renderizzare la UI direttamente nella pipeline OpenGL 3.3 con shader dedicati.
* L'opzione `-Wno-nontrivial-memcall` per compilatori Clang disattiva un warning innocuo di memset interno a ImGui, garantendo un output di compilazione pulito al 100%.

---

### Definizione dell'Eseguibile Principale e Collegamento Librerie

```cmake
file(GLOB_RECURSE SRC_FILES
    "src/*.cpp"
)

add_executable(Interactive2DWorld ${SRC_FILES})

target_include_directories(Interactive2DWorld PRIVATE
    include
    external
    external/glad/include
    external/imgui
    external/imgui/backends
)

target_link_libraries(Interactive2DWorld PRIVATE
    glad
    imgui
    ${GLFW_TARGET}
    ${GLM_TARGET}
    ${OPENGL_LIBRARIES}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(Interactive2DWorld PRIVATE -Wno-nontrivial-memcall)
endif()
```
* **Righe 90-94:** Raccoglie tutti i file `.cpp` presenti nella directory `src/` (`main.cpp`, `GameWorld.cpp`, `Player.cpp`, `Geometry.cpp`, ecc.) e dichiara la creazione dell'eseguibile target `Interactive2DWorld`.
* **Righe 96-102:** Configura i percorsi delle cartelle `include/` del nostro progetto e delle librerie esterne.
* **Righe 104-110:** Esegue il linking (collegamento binario) di `glad`, `imgui`, del target `GLFW`, di `GLM` e della libreria `OpenGL`.
* **Righe 112-114:** Disattiva per i compilatori Clang su macOS il warning di memset interno a ImGui per mantenere l'output di compilazione pulito.

---

### Framework di Sistema Specifici per OS

```cmake
if(APPLE)
    target_link_libraries(Interactive2DWorld PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework QuartzCore"
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(Interactive2DWorld PRIVATE
        ${CMAKE_DL_LIBS}
        pthread
        m
    )
endif()
```
* **Righe 112-126:**
  - **Su macOS:** Il driver grafico e il sistema a finestre richiedono i framework di sistema proprietari di Apple:
    - `Cocoa`: gestione eventi nativi della GUI e della barra delle applicazioni Mac.
    - `IOKit`: accesso all'hardware di input (mouse, tastiera).
    - `CoreVideo`: sincronizzazione del display e V-Sync per evitare glitch grafici.
    - `QuartzCore`: gestione del layer di composizione a finestra.
  - **Su Linux:** Collega `pthread` (multi-threading per i timer di sistema), `m` (funzioni matematiche C standard) e `dl` (caricamento dinamico delle librerie con `dlopen`).

---

### Comando di Post-Build: Copia Automatica degli Asset

```cmake
add_custom_command(TARGET Interactive2DWorld POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Interactive2DWorld>/assets
)
```
* **Righe 129-132:** Uno dei comandi più importanti per il corretto funzionamento del gioco:
  - All'avvio, il codice C++ carica gli shader (`assets/shaders/*.frag`), i modelli 3D (`assets/models/*.obj`) e le texture (`assets/textures/*.bmp`).
  - Spesso su Windows (con Visual Studio) l'eseguibile viene generato in sottocartelle come `build/Release/` o `build/Debug/`.
  - Questo comando post-compilazione dice a CMake: *"Ogni volta che finisci di compilare l'eseguibile con successo, copia automaticamente l'intera cartella `assets/` all'interno della directory esatta in cui risiede il file binario (`$<TARGET_FILE_DIR>`)"*.
  - **Risultato:** Il gioco trova sempre i modelli Hermite, la foto del pilota e gli shader GLSL al primo colpo, a prescindere dal sistema operativo e dalla cartella da cui viene lanciato.
