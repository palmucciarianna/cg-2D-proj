# Astro-Miner 2D: Cosmic Coin Rush
**Interactive 2D World – Arcade Space Adventure**  
*C++ / OpenGL 3.3 Core Profile / GLSL / GLFW / GLAD / GLM / Dear ImGui*

---

## 1. Descrizione del Progetto

**Astro-Miner 2D: Cosmic Coin Rush** è un'applicazione grafica 2D interattiva sviluppata in **C++ e OpenGL moderno (Core Profile)**.  
Il giocatore pilota una navicella mineraria avanzata attraverso un settore spaziale ricco di pericoli, campi di asteroidi e sentinelle aliene. L'obiettivo principale è raccogliere **5 Monete d'Oro Cosmiche** fluttuanti per attivare il monumentale **Warp Gate** (modellato con curve cubiche di Hermite) e accedere ai livelli successivi, celebrati con l'evento **"HURRÀ!"**.

Il progetto implementa un sistema di **particelle fisiche** (esplosione delle rocce in decine di frammenti al laser), **spari letali da schivare dai bordi dello schermo**, shader procedurali senza texture e una gestione rigorosa della gerarchia delle trasformazioni geometriche.

---

## 2. Comandi di Gioco

| Tasto | Azione |
| :--- | :--- |
| **W / A / S / D** oppure **Frecce Direzionali** | Movimento della navicella in tutte le direzioni con propulsione reattiva |
| **Spazio** | Sparo Laser minerario per distruggere gli asteroidi in mille pezzi |
| **Shift + Freccia Su** oppure **`+`** / **`]`** | Incrementa la velocità massima della navicella (aggiorna lo slider HUD) |
| **Shift + Freccia Giù** oppure **`-`** / **`[`** | Diminuisce la velocità della navicella |
| **Mouse (Interfaccia ImGui)** | Spunta Wireframe (mesh reticolari), regolazione parametri (Velocità Navicella, Slider **Effetto Psichedelico Sfondo**), reset partita, monitoraggio FPS e vite |
| **ESC** | Chiusura applicazione |

---

## 3. Avvio Rapido e Guida alla Compilazione Cross-Platform

Il progetto è **100% autosufficiente**: tutte le librerie di terze parti necessarie (**GLM, GLFW, GLAD, Dear ImGui, stb_image**) sono incluse direttamente nella cartella `external/`, garantendo la compilazione senza dover configurare gestori di pacchetti esterni.

### 🟢 Avvio Diretto con Doppio Click (Nessun Terminale Necessario)
* **macOS:** Fare doppio click sull'applicazione **`AstroMiner.app`** oppure sul file eseguibile **`Avvia_AstroMiner.command`** nella cartella `proj-2D/`. L'applicazione compila (se necessario) e avvia il gioco a finestra.
* **Windows (10 / 11):** Fare doppio click sul file **`Avvia_AstroMiner.bat`**.

---

### 💻 Compilazione Manuale da Riga di Comando (CMake)

### A) macOS (Apple Silicon & Intel)
Prerequisiti: `cmake` e le Command Line Tools di Xcode (`clang++`).
```bash
cd proj-2D
cmake -B build
cmake --build build
./build/Interactive2DWorld
```

### B) Windows (Visual Studio 2022)
Per generare la soluzione `.sln`:
```cmd
cd proj-2D
cmake -B build -G "Visual Studio 17 2022" -A x64
```
Apri il file generato `build/Interactive2DWorld.sln` con Visual Studio e premi **F5** (o Avvia senza eseguire debug). In alternativa da riga di comando:
```cmd
cmake --build build --config Release
.\build\Release\Interactive2DWorld.exe
```

### C) Linux (Ubuntu/Debian/Arch)
```bash
cd proj-2D
cmake -B build
cmake --build build
./build/Interactive2DWorld
```

*(La compilazione copia automaticamente gli asset di shader e modelli poligonali `.obj` nella cartella di destinazione del binario).*

---

## 4. Struttura del Progetto e Cartelle

```text
proj-2D/
├── CMakeLists.txt              # Configurazione build universale (Mac/Win/Linux)
├── AstroMiner.app              # Applicazione nativa macOS (doppio click)
├── Avvia_AstroMiner.command    # Script di avvio per macOS
├── Avvia_AstroMiner.bat        # Script di avvio per Windows
├── assets/
│   ├── models/                 # Modelli poligonali OBJ/MTL (alien_drone, warp_gate)
│   ├── screenshots/            # Screenshot di gioco ad alta risoluzione
│   ├── shaders/                # background.frag/.vert, basic.frag/.vert, texture.frag/.vert
│   └── textures/               # Texture del pilota
├── external/                   # Dipendenze bundled (glad, imgui, glm, glfw, stb_image)
├── include/                    # Header file C++ (.h)
└── src/                        # Implementazioni C++ (.cpp) commentate riga per riga
```

---

## 6. Documentazione e Link alla Presentazione

* 📁 **Cartella di Consegna Ufficiale (UniBo OneDrive):** [computer-graphics-2D-proj-AriannaPalmucci](https://liveunibo-my.sharepoint.com/:f:/g/personal/arianna_palmucci_studio_unibo_it/IgDtSYn5HvOoRoAC68fzldEYAbE0YNStANN4Z3mLvcDBN14?e=bU2ap4)
* 📖 **Guida Avvio Cross-Platform e CMake:** [`doc/guide/guida_avvio_os_e_cmake.md`](doc/guide/guida_avvio_os_e_cmake.md)
