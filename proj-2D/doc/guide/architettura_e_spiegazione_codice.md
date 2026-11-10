# Astro-Miner 2D: Cosmic Coin Rush
## Guida Architetturale, Spiegazione del Codice e Preparazione Colloquio Orale

**Insegnamento:** Computer Graphics – Progetto Midterm (A.A. 2025/2026)  
**Corso di Laurea:** Informatica / Ingegneria Informatica – Università di Bologna  
**Candidata:** Arianna Palmucci  
**Piattaforma:** C++17, OpenGL 3.3 Core Profile, GLSL 330, GLFW, GLAD, GLM, Dear ImGui, stb_image  

---

## 1. Matrice di Conformità ai Requisiti di Esame

La seguente tabella attesta la piena rispondenza del progetto a ciascuno dei criteri formali stabiliti nel documento ufficiale della docente (*"Interactive 2D World – Specifica del progetto C++ e OpenGL"*):

| Requisito Ufficiale | Richiesta Specifica Docente | Implementazione in *Astro-Miner 2D* | Stato Conformità |
| :--- | :--- | :--- | :---: |
| **3.1 Assemblaggio Primitive** | $\ge 1$ oggetto complesso da $\ge 3$ parti distinte con trasformazioni gerarchiche non banali. | Navicella gerarchica ad albero ad **8 componenti distinte** (fusoliera a trapezio, muso a freccia, cupolino in cristallo, foto pilota con maschera ellittica, riflesso speculare del vetro, ali a delta specchiate tramite $S(-1,1,1)$, doppi cannoni, ugelli con fiamme dinamiche a doppio strato e LED stroboscopici). | **Piena (8 / 3 parti)** |
| **3.2 Curve di Hermite** | $\ge 2$ forme distinte modellate a spline cubiche di Hermite, di cui almeno 1 interattiva o dinamica. | **2 modelli Hermite distinti a continuità $C^1$** esportati dal modellatore di laboratorio: 1) *Alien Sentinel Drone* dinamico su orbita a 8 di Lissajous e ostacolo letale a 3 colpi con occhio animato; 2) *Cosmic Warp Gate* monumentale ad anello con vortice rotante, traguardo interattivo per chiudere la partita. | **Piena (Entrambi attivi)** |
| **4. Trasformazioni & Istanze** | Model matrix $M = T \cdot R \cdot S$ senza riscrivere vertici a ogni frame; $\ge 3$ istanze condivise su buffer statici. | Geometrie caricate una sola volta in VRAM con `GL_STATIC_DRAW`. **4–6 asteroidi simultanei** e **5 monete simultanee** condividono lo stesso identico VAO/VBO in memoria video, aggiornando solo la variabile `uniform mat4 model`. | **Piena** |
| **5. Due Shader Program** | $\ge 2$ shader GLSL con compiti distinti; sfondo dedicato (gradiente/procedurale) senza texture. | 1) `background.frag/.vert`: sfondo procedurale puro su quad fullscreen NDC senza texture con hash a 3 layer di parallasse cosmica e nebulosa fluida; 2) `basic.frag/.vert`: shader oggetti con colori per vertice e `uColorTint`; 3) `texture.frag/.vert`: shader dedicato con maschera ellittica per il viso del pilota. | **Piena** |
| **6. Animazioni & Delta Time** | $\ge 2$ animazioni (1 utente, 1 autonoma) indipendenti dal framerate tramite Delta Time ($\Delta t$). | Indipendenza da FPS tramite $\Delta t$: fiamme reattive dei reattori modulate dalla spinta dell'utente; orbita a 8 del drone nemico, rotazione angolare asteroidi e rotazione 3D prospettica delle monete tramite $s_x(t) = s_0 \|\cos(\omega t)\|$. | **Piena** |
| **7. Interazione & Collisioni** | Controllo con frecce/WASD; raccolta $\ge 5$ oggetti con indicatore; meccanica aggiuntiva. | Movimento con **Frecce Direzionali** ($\leftarrow, \uparrow, \rightarrow, \downarrow$); raccolta di **5 monete** con barra di avanzamento; cannone laser per polverizzare asteroidi in 28 schegge fisiche con attrito viscoso e alpha fade; dardi di plasma da schivare; 3 vite; Warp Gate; punteggio cumulativo. | **Piena** |
| **8. Organizzazione Codice** | SoC: separare geometrie, scena, shader, renderer, input; **evitare variabili globali superflue**. | Architettura C++17 modulare su 18 file sorgente disaccoppiati. **Zero variabili globali esposte** (incapsulamento ad oggetti puro conforme a RAII). | **Piena** |

---

## 2. Audit Tecnico: Pulizia, Efficienza e Buone Pratiche

1. **Assenza di Variabili Globali:**
   Tutte le variabili di stato risiedono come membri privati delle rispettive classi (`Player`, `GameWorld`, `AsteroidManager`, ecc.). I callback GLFW in `Input.cpp` usano variabili statiche incapsulate nell'unità di compilazione, senza simboli globali esposti negli header (`extern`).
2. **Buffer GPU Statici (`GL_STATIC_DRAW`):**
   Nessuna funzione del game loop invoca `glBufferSubData` o rialloca memoria GPU per gli oggetti di scena. I dati geometrici locali rimangono immutabili nella VRAM; qualsiasi movimento, orientamento o scalatura è gestito via CPU come singola matrice $4 \times 4$ omogenea (GLM) e applicato in parallelo dai core del Vertex Shader.
3. **Punteggio Cumulativo Permanente:**
   Il punteggio del giocatore (`m_score`) non si azzera al cambio livello o al respawn, ma continua ad accumularsi per l'intera durata della sessione fino alla chiusura della finestra.
4. **UI Intelligente Non Invasiva:**
   La finestra di controllo Dear ImGui è estesa all'avvio per mostrare le istruzioni, ma non appena l'utente clicca sull'area di gioco per iniziare a pilotare la navicella, si ridimensiona automaticamente in un HUD compatto nell'angolo superiore, senza chiudersi quando si interagisce con lo slider della velocità.

---

## 3. Mappa Architetturale: Flusso dei Dati e Dipendenze

```text
                             ┌──────────────────┐
                             │     main.cpp     │
                             └─────────┬────────┘
                                       │ (Game Loop: dt, pollEvents, swapBuffers)
                                       ▼
                             ┌──────────────────┐
                             │    GameWorld     │
                             └─────────┬────────┘
         (Orchestratore Centrale e Macchina a Stati: PLAYING, HURRA, GAMEOVER)
         ┌───────────────┬────────────────┬────────────────┬──────────────┐
         ▼               ▼                ▼                ▼              ▼
   ┌───────────┐   ┌───────────┐    ┌───────────┐    ┌───────────┐  ┌───────────┐
   │Background │   │  Player   │    │Collectible│    │ Asteroid  │  │  Hermite  │
   │ Renderer  │   │(Navicella)│    │(5 Monete) │    │  Manager  │  │(Drone/Gate│
   └─────┬─────┘   └─────┬─────┘    └─────┬─────┘    └─────┬─────┘  └─────┬─────┘
         │               │                │                │              │
         ▼               ▼                ▼                ▼              ▼
   ┌───────────┐   ┌───────────┐    ┌───────────┐    ┌───────────┐  ┌───────────┐
   │ShaderMaker│   │ Geometry  │    │Laser/Hazard│   │ Particles │  │Collider2D │
   │  & GLSL   │   │ (VAO/VBO) │    │  Managers │    │(28 Schegge│  │(Cerchio/  │
   └───────────┘   └───────────┘    └───────────┘    └───────────┘  │   AABB)   │
                                                                    └───────────┘
```

---

## 4. Spiegazione Dettagliata delle Classi e dei Moduli

### 4.1 `main.cpp` – Entry Point e Game Loop
* **Ruolo:** Inizializza le librerie esterne, crea il contesto grafico e orchestra il ciclo continuo dei frame.
* **Componenti Chiave:**
  - `glfwInit()` e `glfwCreateWindow(1280, 720, ...)`: configurano la finestra e il contesto OpenGL 3.3 Core Profile.
  - `gladLoadGLLoader(...)`: carica i puntatori a tutte le funzioni OpenGL moderne.
  - `glfwSwapInterval(1)`: attiva il V-Sync, sincronizzando i frame alla frequenza del display per eliminare il tearing.
  - **Game Loop (`while (!glfwWindowShouldClose)`):**
    1. Calcolo del Delta Time: $\Delta t = \min(t_{\text{current}} - t_{\text{last}},\, 0.05)$, per rendere la fisica indipendente dal monitor.
    2. Polling dell'input periferico (`glfwPollEvents()`).
    3. Aggiornamento logico del mondo (`gameWorld.update(deltaTime, window)`).
    4. Pulizia del buffer colore e profondità (`glClear`).
    5. Disegno della scena grafica (`gameWorld.render(currentTime)`).
    6. Disegno dell'interfaccia utente Dear ImGui (`gameWorld.renderUI(deltaTime)`).
    7. Scambio dei buffer front/back (`glfwSwapBuffers(window)`).

---

### 4.2 `GameWorld` (`include/GameWorld.h`, `src/GameWorld.cpp`) – Regia e Macchina a Stati
* **Ruolo:** Contiene e coordina tutte le entità dello scenario, arbitrando gli eventi e gestendo la macchina a stati finiti:
  - `PLAYING`: stato di gioco normale, raccolta monete, distruzione rocce, minaccia dardi e nemici.
  - `LEVEL_CLEARED_HURRA`: stato di vittoria raggiunto volando dentro il Warp Gate; attiva il timer di celebrazione e la pioggia di coriandoli.
  - `GAME_OVER`: navicella distrutta (3 impatti subiti); visualizza il popup di fine partita centrato.
* **Metodi Chiave:**
  - `init()`: istanzia gli shader, avvia i sistemi particellari e spawna le entità per il Livello 1.
  - `update(dt, window)`: controlla input da tastiera, sparo laser con barra spazio, aggiornamento proiettili, particelle, dardi e controllo analitico delle collisioni.
  - `renderUI(dt)`: gestisce la finestra Dear ImGui "Mission Control"; grazie al flag `!io.WantCaptureMouse`, compattarsi solo quando l'utente clicca fuori dalla finestra per giocare, mantenendosi aperta quando si agisce sullo slider della velocità.

---

### 4.3 `Geometry` (`include/Geometry.h`, `src/Geometry.cpp`) – Incapsulamento Hardware VAO/VBO
* **Ruolo:** Rappresenta il wrapper C++ per le risorse di memoria della GPU.
* **Componenti Chiave:**
  - `m_vao`: l'ID del Vertex Array Object che memorizza il formato degli attributi di vertice.
  - `m_vboPositions`, `m_vboColors`, `m_vboTexCoords`: i Vertex Buffer Object che ospitano i dati numerici grezzi.
  - `upload()`: esegue il binding ed effettua `glBufferData(..., GL_STATIC_DRAW)`. La geometria viene caricata in VRAM ad alta velocità e lì risiede immutabile.
  - `draw()`: associa il VAO ed emette la chiamata di disegno `glDrawArrays`.

---

### 4.4 `ShaderMaker` e `Shader` (`include/ShaderMaker.h`, `src/ShaderMaker.cpp`, `include/Shader.h`, `src/Shader.cpp`)
* **Ruolo:** Gestione della compilazione e collegamento dei programmi shader scritti in GLSL 330.
* **Componenti Chiave:**
  - `ShaderMaker::createProgram()`: compila il Vertex Shader e il Fragment Shader, controllando con `glGetShaderiv` eventuali errori di sintassi e stampando messaggi diagnostici.
  - `Shader::setMat4(name, mat)`: imposta le matrici $4 \times 4$ (Model e Projection) tramite `glUniformMatrix4fv`.
  - `Shader::setVec4(name, vec)`: imposta vettori quadridimensionali (es. `uColorTint` per lampeggi visivi di danno o invulnerabilità).

---

### 4.5 `BackgroundRenderer` (`include/BackgroundRenderer.h`, `src/BackgroundRenderer.cpp`) – Sfondo Senza Texture
* **Ruolo:** Esegue il pass di rendering dedicato dello sfondo spaziale procedurale (Requisito 5.1).
* **Funzionamento Algoritmico:**
  - Disegna un quadrilatero fullscreen a coordinate normalizzate NDC $[-1, 1]$.
  - Nel fragment shader (`assets/shaders/background.frag`), le stelle nascono da una funzione hash pseudo-casuale deterministica ad alta frequenza:
    $$h(\mathbf{p}) = \text{fract}\left(\sin(\mathbf{p} \cdot (12.9898, 78.233)) \cdot 43758.5453\right)$$
  - La funzione viene campionata su **tre layer di parallasse indipendenti**:
    1. *Layer Remoto ($120\times$):* stelle fitte, minute e a scorrimento lentissimo.
    2. *Layer Intermedio ($70\times$):* magnitudine media e velocità moderata.
    3. *Layer Primo Piano ($35\times$):* stelle grandi e pulsanti nel tempo ($\sin(3.5 \cdot \text{uTime} + \phi)$).
  - Viene inoltre generata una nebulosa fluida trigonometrica a onde armoniche incrociate che mescola gradazioni ciano e magenta.

---

### 4.6 `ShapeGenerator` (`include/ShapeGenerator.h`, `src/ShapeGenerator.cpp`) – Fabbrica Primitive 2D
* **Ruolo:** Calcola su CPU i vertici e i gradienti di colore delle primitive poligonali di base.
* **Metodi Chiave:**
  - `createCircle(cx, cy, rx, ry, segments)`: costruisce una ventaglio di triangoli (`GL_TRIANGLE_FAN`) a 32 settori discreti, utilizzato per le monete, i LED, il cupolino e gli asteroidi.
  - `createTrapezoid(topW, botW, height)`: genera la mesh a trapezio isoscele della fusoliera.
  - `createTriangle()`: crea triangoli affilati per il muso della navetta e le fiamme di scarico.

---

### 4.7 `Player` (`include/Player.h`, `src/Player.cpp`) – Navicella Composita & Texture Pilota
* **Ruolo:** Modella l'astronave principale a 8 parti gerarchiche (Requisito 3.1) e gestisce la fisica di volo.
* **Componenti Chiave:**
  - **Gerarchia Matrici Affini:**
    La radice è data da $M_{\text{player}} = T(x, y) \cdot R(\theta - 90^\circ)$. Ogni componente si concatena mediante:
    $$M_{\text{componente}} = M_{\text{player}} \cdot T_{\text{rel}} \cdot R_{\text{rel}} \cdot S_{\text{rel}}$$
  - **Simmetria Speculare:** L'ala sinistra è generata riusando la geometria dell'ala destra invertendo l'asse locale $X$ tramite la matrice $S(-1, 1, 1)$.
  - **Texture Viso con Ritaglio Ellittico:** La foto del pilota (`player_face.bmp`) è mappata tramite `texture.frag`: il frammento viene scartato con `discard` se eccede l'equazione dell'ellisse $((u-0.5)/0.5)^2 + ((v-0.5)/0.5)^2 > 1.0$, su cui viene poi disegnato il riflesso lucido del vetro.
  - **Controlli:** Le **Frecce Direzionali** ($\leftarrow, \uparrow, \rightarrow, \downarrow$) calcolano il vettore direzione e aggiornano l'angolo $\theta = \text{atan2}(v_y, v_x)$ e l'intensità della spinta reattiva.

---

### 4.8 `HermiteObject` (`include/HermiteObject.h`, `src/HermiteObject.cpp`) – Curve Cubiche $C^1$
* **Ruolo:** Gestisce i due modelli modellati con spline di Hermite ed esportati in formato Wavefront OBJ (Requisito 3.2).
* **I Due Modelli:**
  1. **Alien Sentinel Drone (`alien_drone.obj`):** sagoma cornuta con 113 vertici triangolati. Si muove lungo la curva chiusa di Lissajous:
     $$x(t) = 640 + 340\cos(0.85t), \quad y(t) = 380 + 160\sin(1.70t)$$
     È un ostacolo letale a 3 colpi: a ogni impatto toglie 1 vita e attiva il timer di ammiccamento dell'occhio giallo (`triggerEyeBlink()`).
  2. **Cosmic Warp Gate (`warp_gate.obj`):** anello monumentale circolare cavo con vortice rotante. Rimane dormiente finché non si raccolgono le 5 monete; una volta attivo, volandovi dentro la partita viene vinta con successo scatenando l'HURRÀ.

---

### 4.9 `AsteroidManager` e `Collectible` – Condivisione Geometria VRAM
* **Ruolo:** Dimostrano l'ottimizzazione GPU (Requisito 4) mediante buffer statici condivisi.
* **Funzionamento Chiave:**
  - La geometria locale di un asteroide o di una moneta viene inviata una sola volta in VRAM con `GL_STATIC_DRAW`.
  - A runtime, il ciclo di rendering itera su tutte le istanze attive, inviando alla GPU esclusivamente la matrice $4 \times 4$ omogenea di ciascun elemento via uniform:
    ```cpp
    for (const auto& a : m_asteroids) {
        if (a.active) {
            glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(a.pos, 0.0f));
            m = glm::rotate(m, glm::radians(a.rotation), glm::vec3(0, 0, 1));
            m = glm::scale(m, glm::vec3(a.scale, a.scale, 1.0f));
            shader.setMat4("model", m);
            m_asteroidGeo.draw(); // Stesso VAO/VBO condiviso
        }
    }
    ```
  - Le 5 monete esibiscono una rotazione 3D fittizia modulando la scala orizzontale come $s_x(t) = s_0 |\cos(2.8 t + \phi_i)|$.

---

### 4.10 `LaserManager` e `HazardManager` – Meccaniche Aggiuntive e Minacce
* **LaserManager:** gestisce i proiettili sparati con la barra spaziatrice. Muove i raggi a $780\text{ px/s}$ e verifica le collisioni contro gli asteroidi, distruggendoli e assegnando $+20$ punti.
* **HazardManager:** a intervalli casuali genera dardi cremisi ad alta velocità che attraversano lo schermo dai bordi, preceduti da una linea di allarme tratteggiata lampeggiante per consentire al giocatore di schivarli.

---

### 4.11 `ParticleSystem` (`include/ParticleSystem.h`, `src/ParticleSystem.cpp`) – Simulazione Fisica
* **Ruolo:** Produce effetti visivi procedurali particellari in tempo reale:
  - **Frantumazione Rocce:** all'impatto laser con un asteroide, quest'ultimo scompare e si generano **28 schegge poligonali di roccia** con velocità radiali e rotazioni casuali, soggette ad attrito viscoso $\mathbf{v}_{t+\Delta t} = \mathbf{v}_t(1 - 0.75\Delta t)$ e dissolvenza lineare progressiva del canale alpha.
  - **Coriandoli di Vittoria:** 135 particelle multicolore sparate a festa a tutto schermo durante l'evento HURRÀ del Warp Gate.

---

### 4.12 `Collider2D.h` – Modulo Matematico di Collisione
* **Ruolo:** Implementa algoritmi analitici ad alta efficienza:
  - **Intersezione Cerchio–Cerchio:** due cerchi con centri $\mathbf{c}_1, \mathbf{c}_2$ e raggi $r_1, r_2$ collidono se $\|\mathbf{c}_1 - \mathbf{c}_2\|^2 \le (r_1 + r_2)^2$. L'assenza della radice quadrata ottimizza le prestazioni su CPU.
  - **Intersezione Cerchio–AABB:** verifica la distanza del centro rispetto al rettangolo allineato agli assi, impiegata per i proiettili laser contro i massi.

---

## 5. Guida alla Discussione Orale: Domande Tipiche e Risposte

### D1: "Perché non riscrivi i vertici della mesh a ogni frame sulla CPU?"
> **Risposta:**  
> Riscrivere le coordinate su CPU e ricaricarle a ogni iterazione (ad esempio tramite `glBufferSubData`) provocherebbe un grave collo di bottiglia dovuto al continuo trasferimento di dati lungo il bus PCI-Express, oltre a forzare frequenti sincronizzazioni tra CPU e GPU.  
> Invece, la geometria locale viene caricata una sola volta nella memoria video (VRAM) tramite `GL_STATIC_DRAW`. La CPU si limita a computare un'unica matrice di modellazione omogenea $4 \times 4$ ($M = T \cdot R \cdot S$) tramite la libreria GLM, trasmettendola alla GPU come variabile `uniform mat4 model`. È il Vertex Shader a moltiplicare parallelamente ciascun vertice per la matrice sui core della scheda video, permettendo rendering stabili a oltre 120 FPS con carico CPU trascurabile.

---

### D2: "Come garantisci la continuità delle curve di Hermite?"
> **Risposta:**  
> Una curva spline composita a tratti è continua di classe $C^0$ se i punti estremi di due segmenti consecutivi coincidono nello spazio: $\mathbf{p}_k(1) = \mathbf{p}_{k+1}(0) = \mathbf{P}_{k+1}$.  
> La continuità tangenziale di classe $C^1$ richiede che anche i vettori derivata prima non presentino discontinuità nei nodi di giunzione. Derivando analiticamente rispetto a $t$ la formulazione cubica di Hermite:
> $$\mathbf{p}'(t) = (6t^2 - 6t)\mathbf{P}_0 + (-6t^2 + 6t)\mathbf{P}_1 + (3t^2 - 4t + 1)\mathbf{R}_0 + (3t^2 - 2t)\mathbf{R}_1$$
> si nota che $\mathbf{p}'(0) = \mathbf{R}_0$ e $\mathbf{p}'(1) = \mathbf{R}_1$. Dunque, la continuità $C^1$ è garantita imponendo che il vettore tangente uscente dal segmento $k$ coincida in modulo, direzione e verso con il vettore tangente entrante nel segmento $k+1$:
> $$\mathbf{p}'_k(1) = \mathbf{p}'_{k+1}(0) \iff \mathbf{R}_{k+1}^- = \mathbf{R}_{k+1}^+$$
> Nel nostro progetto, sia per l'Alien Sentinel Drone che per il Warp Gate, i punti di controllo e le tangenti sono stati modellati ed esportati nell'editor interattivo del laboratorio, che vincola le tangenti contigue a essere collineari e uguali, garantendo una superficie priva di spigoli vivi indesiderati.

---

### D3: "Come funziona lo sfondo spaziale senza usare texture precalcolate?"
> **Risposta:**  
> L'intero spazio profondo è sintetizzato matematicamente pixel per pixel all'interno del fragment shader (`background.frag`), renderizzato su un quadrilatero fullscreen a coordinate normalizzate NDC $[-1, 1]$.  
> Le stelle non provengono da un'immagine, ma nascono da una funzione hash deterministica ad altissima frequenza numerica $h(\mathbf{p}) = \text{fract}(\sin(\mathbf{p} \cdot (12.9898, 78.233)) \cdot 43758.5453)$ valutata sulle coordinate di griglia dello schermo.  
> Per conferire un effetto di profondità e parallasse naturale, il campionamento viene eseguito su **tre layer indipendenti** ($120\times$ lontano e fitto, $70\times$ intermedio, $35\times$ in primo piano con stelle pulsanti nel tempo via `uTime`). A questo si somma una nebulosa fluida trigonometrica a onde sinusoidali continue con sfumature ciano e magenta.

---

### D4: "Come sono realizzate le ali della navicella e la texture della cabina?"
> **Risposta:**  
> L'ala sinistra è generata riusando esattamente la stessa mesh triangolare dell'ala destra: applichiamo una matrice affine locale con fattore di scala negativo sull'asse $X$ locale, ovvero la matrice di riflessione:
> $$S(-1, 1, 1) = \begin{bmatrix} -1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & 1 \end{bmatrix}$$
> Nella cabina di pilotaggio, invece, viene prima renderizzata la base ovoidale in cristallo ciano; subito sopra viene applicata la texture 2D del viso pilota con uno shader dedicato (`texture.frag`) che scarta con `discard` i frammenti all'esterno della forma ellittica, ed infine viene disegnata una sotto-ellisse bianca eccentrica semitrasparente come decal geometrica 2D per riprodurre il riflesso del vetro protettivo (senza calcoli di illuminotecnica 3D).

---

### D5: "Il progetto contiene curve di Bézier, B-spline, NURBS, modelli di illuminazione (Phong/Blinn-Phong), materiali fisici o ray tracing?"
> **Risposta:**  
> **Assolutamente no.** Il progetto rispetta con rigore i confini didattici e gli obiettivi formativi dell'**Elaborato 1 (Interactive 2D World)** del corso:
> 1. **Curve:** Sono impiegate unicamente **spline cubiche di Hermite a continuità $C^1$** (per l'Alien Sentinel Drone e il Warp Gate). Non ci sono curve di Bézier (polinomi di Bernstein, de Casteljau), B-spline (vettori nodali, De Boor) o NURBS, argomenti trattati in altre sezioni del corso.
> 2. **Illuminazione e Materiali:** In un mondo 2D non ha senso computare equazioni di illuminazione 3D. Non è presente alcun modello di illuminazione (Phong, Blinn-Phong, Gouraud, Lambert), non esistono luci puntiformi o direzionali e non si calcolano vettori normali. Il rendering avviene tramite interpolazione lineare dei colori per vertice (flat shading 2D), moltiplicazione per tinte uniformi e shader matematici procedurali per lo sfondo cosmico. I file Wavefront MTL forniscono unicamente il colore flat RGB di base (`Kd`).
> 3. **Ray Tracing e Path Tracing:** Il motore grafico si basa al 100% sulla **rasterizzazione** hardware accelerata di OpenGL 3.3 Core Profile mediante shader GLSL (pipeline programmabile con VAO e VBO). Non viene eseguito alcun tracciamento di raggi o stima della radianza con metodi Monte Carlo.
