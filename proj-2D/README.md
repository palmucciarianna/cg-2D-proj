# Astro-Miner 2D: Cosmic Coin Rush
**Progetto Midterm di Computer Graphics (A.A. 2025/2026) – Interactive 2D World**  
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
| **W / A / S / D** oppure **Frecce Direzionali** | Movimento della navicella in tutte le direzioni con propulsione reattiva e scia di particelle di plasma ionico |
| **Spazio** | Sparo Laser minerario per distruggere gli asteroidi e indebolire il Drone Alieno Sentinella |
| **Shift + Freccia Su** oppure **`+`** / **`]`** | Incrementa la velocità massima della navicella (aggiorna lo slider HUD) |
| **Shift + Freccia Giù** oppure **`-`** / **`[`** | Diminuisce la velocità della navicella |
| **Mouse (Interfaccia ImGui)** | Spunta Wireframe, regolazione Slider **Slow-Motion** (0.1x..2.0x con preset Matrix/Normale/Rapido), Slider **Effetto Psichedelico Sfondo**, Spunta **Traiettoria Hermite Drone**, Visualizzazione **Leaderboard Piloti**, reset partita, monitoraggio FPS e vite |
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
Per generare la soluzione `.sln` richiesta per la consegna:
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

### D) Web (WebAssembly / WebGL 2.0 via Emscripten SDK)
Il progetto supporta nativamente la compilazione per browser Web tramite Emscripten:
```bash
# Avvio script automatico dalla root o da proj-2D:
./build_web.sh

# Oppure manualmente con CMake:
emcmake cmake -B build-web -S proj-2D -DCMAKE_BUILD_TYPE=Release
cmake --build build-web --config Release

# Avvio server HTTP locale per testare nel browser:
python3 -m http.server 8080 --directory build-web
# Apri http://localhost:8080/Interactive2DWorld.html
```

*(La compilazione copia o precarica automaticamente gli asset di shader e modelli poligonali `.obj`).*

---

## 4. Conformità ai Requisiti di Esame

1. **Assemblaggio di Primitive (Requisito 3.1)**:
   * La navicella del giocatore (`src/Player.cpp`) è un oggetto composito formato da **8 parti geometriche**: muso anteriore a punta, fusoliera centrale a trapezio, cupola in cristallo con foto pilota e decal geometrica del vetro riflettente, ali a delta con pannellature specchiate tramite $S(-1,1,1)$, doppi cannoni alari, ugelli reattori, doppi getti di fiamma al plasma e indicatori LED stroboscopici di navigazione.
   * La composizione è governata da matrici locali relative alla matrice radice: $M_{componente} = M_{player} \cdot T_{local} \cdot R_{local} \cdot S_{local}$.

2. **Curve Cubiche di Hermite (Requisito 3.2)**:
   * Sono inclusi **due oggetti distinti** a continuità $C^1$ modellati con spline di Hermite ed esportati in formato OBJ/MTL triangolato (`assets/models/`):
     * **Alien Sentinel Drone** (`alien_drone.obj`): Oggetto dinamico che pattuglia lo spazio con moto armonico continuo a 8. È un ostacolo letale: ogni passaggio del giocatore sopra il drone attivo toglie 1 vita e dopo 3 passaggi la navicella viene distrutta (Game Over). L'occhio giallo sbatte le palpebre al contatto! Con l'avanzare dei livelli la sua velocità aumenta e compie **scatti predatori improvvisi ed erratici** con loop di manovra e rollata d'attacco prima di riallinearsi alla traiettoria principale. **Meccanica di Indebolimento Balistico**: colpendolo con il laser, il Drone Rosso **si indebolisce** per 4.5 secondi (velocità ridotta del 65%, scatti aggressivi azzerati, scala ridotta, occhio stordito, tinta visiva pulsante ed emissione di scintille elettrificate): mentre è indebolito diventa **innocuo**, non infligge danni alla navicella ed eroga +35 punti bonus.
     * **Cosmic Warp Gate** (`warp_gate.obj`): Oggetto interattivo ad anello e vortice. Una volta raccolte le 5 monete d'oro, il portale si attiva con luce dorata/ciano: per chiudere la partita e superare il livello il giocatore deve pilotare la navicella dentro il portale!

3. **Trasformazioni Geometriche e Istanze Condivise (Requisito 4)**:
   * Tutti i dati geometrici risiedono in buffer statici GPU (`GL_STATIC_DRAW`). Nessuna coordinata viene riscritta su CPU durante il frame loop.
   * Il gestore asteroidi (`src/AsteroidManager.cpp`) riutilizza **lo stesso identico VAO/VBO** per 4–6 istanze contemporanee con scale, orientamenti e velocità di rotazione indipendenti (rotazione stazionaria su se stessi senza movimenti caotici per garantire massima leggibilità).
   * La stessa tecnica di condivisione mesh è applicata alle 5 monete d'oro.

4. **Shader Dedicato allo Sfondo e Pipeline (Requisito 5)**:
   * **Shader di Sfondo Psichedelico** (`shaders/background.frag`): Calcola proceduralmente (senza uso di texture) uno starfield multi-livello e una nebulosa fluida ipnotica potenziata con **domain warping trigonometrico** ($\sin/\cos$) e coordinate polari ($r, \theta$) per fasci e spirali energetiche. Include **alternanza discreta di due palette cromatiche tra livelli pari e dispari** (Ciano/Magenta galattico vs. Oro/Viola nebulare) e uno **slider interattivo ImGui `Effetto Psichedelico`** per modulare in tempo reale l'intensità della distorsione.
   * **Shader Oggetti** (`shaders/basic.frag`): Shader dedicato con attributi di vertice, matrici ortografiche/model e uniform per il tint dinamico.

5. **Animazioni e Tempo (Requisito 6)**:
   * Animazione legata all'utente: fiamme dei doppi reattori che vibrano e si allungano dinamicamente in base alla spinta (`thrustIntensity`) e al delta time.
   * Animazioni autonome dello scenario: traiettoria sinusoidale del drone alieno con scatti d'attacco a campana sinusoidale, rotazione stazionaria degli asteroidi e rotazione 3D fittizia delle monete con $\cos(\omega t)$.

6. **Interazione, Meccaniche e Collisioni (Requisito 7)**:
   * Movimento fluido con regolazione della velocità da tastiera e slider.
   * Raccolta di **5 Monete Cosmiche** con emissione di scintille dorate e aggiornamento contatore HUD.
   * **Sistema di Difficoltà Progressiva e Pericoli**:
     * **Cannone Laser**: Distrugge gli asteroidi in circa 30 frammenti fisici animati (`ParticleSystem`) e colpisce l'Alien Drone Rosso indebolendolo per 4.5s con scintille ioniche.
     * **Raggi di Pericolo Scalabili**: Dardi al plasma ad alta velocità con segnale di allerta stroboscopico. Al salire dei livelli la frequenza di sparo e la velocità dei proiettili aumentano; **dal Livello 5 in poi si sbloccano laser verticali dall'alto verso il basso**, costringendo a un dodging dinamico su più direttrici.
   * Evento **"HURRÀ!"**: entrando fisicamente nel Warp Gate attivo a destra si scatena la festa con pioggia di coriandoli, bonus +500 punti e generazione del livello successivo.

7. **Organizzazione del Codice (Requisito 8)**:
   * Suddivisione modulare: `Core`, `Graphics`, `Entities`, `Physics`, `UI` ed orchestrazione pulita tramite `GameWorld`.

8. **Conformità e Rigorosa Esclusione di Temi Non Pertinenti al Midterm 2D**:
   * **Curve:** Utilizzate unicamente **spline cubiche di Hermite a continuità $C^1$**. Assenza totale di curve di Bézier, B-spline o NURBS.
   * **Illuminazione e Materiali:** Assenza totale di modelli di illuminazione (Phong, Blinn-Phong, Lambert, Gouraud), normali per vertice/frammento, luci puntiformi o proprietà fisiche di materiale. Rendering 100% 2D tramite vertex colors interpolati, tinte uniformi e shader matematico per lo sfondo. I file MTL forniscono unicamente il colore flat RGB di base.
   * **Ray Tracing / Path Tracing:** Assenza totale. Il rendering sfrutta esclusivamente la **rasterizzazione** programmabile in OpenGL 3.3 Core Profile.

---

## 5. Nuove Funzionalità Avanzate (Branch `v2`)

Sul branch `v2` sono state introdotte evoluzioni grafiche e di gameplay all'avanguardia:

1. **Slow-Motion Simulation Slider (Controllo Temporale Relativistico)**:
   * Slider continuo nel pannello HUD (da `0.1x` a `2.0x`) con pulsanti preset istantanei: `0.25x (Effetto Matrix)`, `1.0x (Normale)`, `2.0x (Rapido)`.
   * Modula la fisica di gioco (`simDt = dt * m_timeScale`) mantenendo il framerate dell'interfaccia ImGui e delle animazioni GUI a 60/120 Hz costanti.

2. **Screen Shake (Camera Trauma Sinusoidale)**:
   * Sistema di trauma della telecamera con decadimento esponenziale armonico `m_shakeOffset = glm::vec2(sin(52t) * amp, cos(44t) * amp) * progress`.
   * Integrato direttamente nella matrice di proiezione ortografica `glm::ortho` per impatti navetta (14px), frangibilità laser degli asteroidi (5px) e salti nel portale Warp (9px).

3. **Neon Glow Aura con Additive Blending (`GL_SRC_ALPHA, GL_ONE`)**:
   * Alone luminoso al plasma attorno al monumentale Warp Gate attivo e alle monete d'oro cosmiche, ottenuto con blending additivo hardware a zero overhead.

4. **Scia Particellare al Plasma dei Propulsori (Thruster Ion Trail)**:
   * Generazione dinamica di particelle ioniche multicolore (ciano, arancio, bianco) dagli ugelli di poppa della navetta, proporzionale all'intensità di spinta e orientata in direzione opposta al vettore prua.

5. **Tracciamento Parametrico della Curva di Hermite del Drone**:
   * Visualizzazione opzionale tramite checkbox della traiettoria a 8 (Lissajous-8) del Drone Sentinella con 120 segmenti `GL_LINE_STRIP` in ciano neon.

6. **Sistema Multi-Utente / Leaderboard Piloti (Classifica Punteggi)**:
   * Tabella ranking con registrazione automatica dei migliori piloti al completamento dei livelli o a fine partita.

7. **Architettura Web Cross-Platform (Emscripten / WebGL 2.0 / WASM)**:
   * Compilazione ed esecuzione nativa nei browser moderni (Chrome, Safari, Firefox, Edge) tramite WebAssembly e WebGL 2.0.
   * Header unificato `OpenGL_Headers.h`, adattamento dinamico GLSL in runtime e template HTML5 reattivo in `web/shell_minimal.html`.

---

## 6. Struttura del Repository e Cartelle

```text
proj-2D/
├── CMakeLists.txt              # Configurazione build universale (Mac/Win/Linux)
├── README.md                   # Documentazione di progetto
├── AstroMiner.app              # Applicazione nativa macOS (doppio click)
├── Avvia_AstroMiner.command    # Script di avvio per macOS
├── Avvia_AstroMiner.bat        # Script di avvio per Windows
├── doc/                        # Documentazione ordinata in minuscolo
│   ├── relazione/              # Relazione tecnica accademica
│   │   ├── relazione_tecnica.pdf  # PDF compilato pronto per la consegna
│   │   ├── relazione_tecnica.tex  # Sorgente LaTeX ufficiale
│   │   └── relazione_tecnica.md   # Versione Markdown
│   └── guide/                  # Guide tecniche e didattiche
│       ├── guida_avvio_os_e_cmake.md         # Istruzioni avvio OS e analisi CMake
│       ├── architettura_e_spiegazione_codice.md # Matrice requisiti e guida all'orale
│       └── guida_presentazione_canva.md       # Traccia e discorsi per le slide
├── presentazione/              # Presentazione multimediale per l'esame
│   └── presentazione.pdf       # Diapositive ufficiali esportate (16:9)
├── assets/
│   ├── models/                 # Modelli Hermite OBJ/MTL (alien_drone, warp_gate)
│   ├── screenshots/            # Screenshot di gioco ad alta risoluzione
│   ├── shaders/                # background.frag/.vert, basic.frag/.vert, texture.frag/.vert
│   └── textures/               # Texture del pilota
├── external/                   # Dipendenze bundled (glad, imgui, glm, glfw, stb_image)
├── include/                    # Header file C++ (.h)
└── src/                        # Implementazioni C++ (.cpp) commentate riga per riga
```

---

## 7. Documentazione e Link alla Presentazione

* 📁 **Cartella di Consegna Ufficiale (UniBo OneDrive):** [computer-graphics-2D-proj-AriannaPalmucci](https://liveunibo-my.sharepoint.com/:f:/g/personal/arianna_palmucci_studio_unibo_it/IgDtSYn5HvOoRoAC68fzldEYAbE0YNStANN4Z3mLvcDBN14?e=bU2ap4)
* 🎨 **Presentazione Online (Canva):** [https://canva.link/qwwbv3ayalvsctp](https://canva.link/qwwbv3ayalvsctp)
* 📊 **Presentazione PDF Inclusa:** [`doc/presentazione/presentazione.pdf`](presentazione/presentazione.pdf) (presentazione ufficiale ad alta risoluzione)
* 📄 **Relazione Tecnica Ufficiale (PDF):** [`doc/relazione/relazione_tecnica.pdf`](doc/relazione/relazione_tecnica.pdf)
* 📜 **Sorgente LaTeX della Relazione:** [`doc/relazione/relazione_tecnica.tex`](doc/relazione/relazione_tecnica.tex)
* 📖 **Guida Avvio Cross-Platform e CMake:** [`doc/guide/guida_avvio_os_e_cmake.md`](doc/guide/guida_avvio_os_e_cmake.md)
* 🏛️ **Architettura, Flusso Dati e Preparazione Orale:** [`doc/guide/architettura_e_spiegazione_codice.md`](doc/guide/architettura_e_spiegazione_codice.md)
* 🗣️ **Traccia Discorso per Canva:** [`doc/guide/guida_presentazione_canva.md`](doc/guide/guida_presentazione_canva.md)
