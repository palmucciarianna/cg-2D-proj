# GUIDA ALLA PRESENTAZIONE ORALE (CANVA)
## Astro-Miner 2D: Cosmic Coin Rush
**Candidata:** Arianna Palmucci  
**Docente:** Prof.ssa Damiana Lazzaro – Università di Bologna  
**Corso:** Computer Graphics (A.A. 2026/2027 – Parziale I)  
**Tecnologie:** C++17 nativo, OpenGL 3.3 Core Profile, GLSL, GLFW, GLAD, GLM, Dear ImGui

---

# STRUTTURA DEL DISCORSO ORALE (10 SLIDE – 5/7 MINUTI)

Questa guida è allineata **pagina per pagina** con la tua presentazione Canva definitiva.  
Per ogni slide trovi:
1. **Contenuto della Slide** (i punti chiave a schermo).
2. **Cosa dire a voce (Script parlato)**: formulato in tono professionale, chiaro, accademico e sicuro.
3. **Domande a trabocchetto della Prof.ssa Lazzaro**: cosa potrebbe chiederti su quella slide e la risposta esatta "salva-esame".

---

## SLIDE 1: Titolo e Apertura
* **Titolo a schermo:** *ASTRO-MINER 2D: COSMIC COIN RUSH*
* **Sottotitolo:** *Interactive 2D World – Elaborato 1 di Computer Graphics (C++17 / OpenGL 3.3 Core Profile)*
* **Dati:** Candidata: Arianna Palmucci | Docente: Prof.ssa Damiana Lazzaro | A.A. 2026/2027

### 🎙️ Cosa dire a voce (35 secondi):
> *"Buongiorno professoressa. Oggi presento Astro-Miner 2D: Cosmic Coin Rush, un mondo interattivo bidimensionale sviluppato interamente in C++17 nativo e OpenGL 3.3 Core Profile.  
> L'obiettivo del progetto è stato realizzare un motore grafico didatticamente rigoroso, fondato sulla moderna pipeline programmabile a shader, che implementa modellazione gerarchica complessa, curve spline cubiche di Hermite a continuità $C^1$, texture mapping UV, shader procedurali avanzati, fisica particellare e un'interfaccia utente reattiva basata su Dear ImGui."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"Perché ha scelto il Core Profile 3.3 e non la pipeline tradizionale o una versione più recente?"*
* **Risposta vincente:** *"Ho scelto il Core Profile 3.3 perché esclude completamente tutte le funzioni a pipeline fissa ormai deprecate dal 2008 (come `glBegin`, `glEnd`, `glMatrixMode`), costringendo a gestire esplicitamente la memoria VRAM tramite VBO e VAO e il calcolo geometrico tramite shader GLSL personalizzati. Inoltre garantisce la massima portabilità multipiattaforma nativa, in particolare su macOS dove OpenGL è fermo al profilo 4.1."*

---

## SLIDE 2: Obiettivi Didattici & Rispondenza alle Specifiche
* **Riquadri a schermo:**
  * 🧩 **Geometrie:** Oggetto composto gerarchico a 8 parti + 2 modelli a spline cubiche di Hermite.
  * ⚡ **Prestazioni:** Istanziazione multipla condivisa su buffer statici VRAM (`GL_STATIC_DRAW`).
  * 🌌 **Rendering:** Due shader GLSL disaccoppiati (sfondo procedurale senza texture + shader oggetti).
  * 🕹️ **Dinamiche:** Loop scalato con Delta Time, particelle fisiche, ostacolo letale e Warp Gate di vittoria.
  * **Badge:** *Criteri Soddisfatti: 100%*

### 🎙️ Cosa dire a voce (40 secondi):
> *"In questa panoramica ho sintetizzato la conformità ai vincoli formali della specifica d'esame:
> Dal punto di vista geometrico, ho modellato una navicella articolata e due entità basate su spline cubiche di Hermite.  
> Sul fronte dell'efficienza della pipeline, ho azzerato il bandwidth sprecato su bus PCIe grazie al Buffer Sharing su memoria grafica statica.  
> Il sottosistema di shading impiega due programmi GLSL distinti: uno per gli oggetti della scena e uno interamente matematico e procedurale per lo spazio cosmico.  
> Infine, tutte le dinamiche fisiche sono parametrizzate sul Delta Time, garantendo un framerate indipendente dalla frequenza di refresh del monitor."*

---

## SLIDE 3: Architettura Software e Game Loop
* **Punti a schermo:**
  * **Core & Input:** Finestra GLFW 3.3, gestione eventi e Delta Time ($\Delta t$).
  * **Graphics Layer:** ShaderMaker, Geometry (VAO/VBO), Texture (`stb_image`).
  * **Scena & Entità:** Classi indipendenti `Player`, `AsteroidManager`, `Collectible`, `HermiteManager`.
  * **Orchestrazione:** FSM in `GameWorld` (`PLAYING` $\to$ `HURRÀ` $\to$ `GAME_OVER`).
  * **Diagramma di flusso a destra:** *Input / Delta Time $\to$ Aggiornamento Logico & Collisioni $\to$ Pass Sfondo $\to$ Pass Scena & UI*.

### 🎙️ Cosa dire a voce (45 secondi):
> *"L'architettura del software segue il principio della Separation of Concerns (SoC) ed è priva di qualsiasi variabile globale esposta, applicando il paradigma RAII per la gestione del ciclo di vita delle risorse hardware.  
> Come illustra il diagramma di flusso del Render Loop: a ogni frame la CPU acquisisce gli input e misura il delta time. `GameWorld` aggiorna la cinematica e risolve le collisioni geometriche.  
> Successivamente la GPU riceve i comandi di rendering secondo un ordine rigoroso Back-to-Front: prima il passaggio dello sfondo procedurale, poi il passaggio degli oggetti solidi del mondo e infine il disegno in Alpha Blending dell'interfaccia utente Dear ImGui."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"Cosa succede se il gioco subisce un lag improvviso o la finestra viene messa in pausa?"*
* **Risposta vincente:** *"Ho introdotto il Delta-Time Clamping: limito il $\Delta t$ effettivo a un massimo di 50 millisecondi ($\Delta t_{\text{effettivo}} = \min(\Delta t, 0.05\,\text{s})$). Questo previene il noto fenomeno di tunneling (bullet-through-paper), impedendo che la navicella o i proiettili attraversino gli asteroidi a causa di un passo temporale troppo grande."*

---

## SLIDE 4: Assemblaggio dell'Oggetto Composto (Requisito 3.1)
* **Punti a schermo:**
  * **Scomposizione in Primitive:** Muso a freccia, fusoliera trapezoidale, ali a delta, doppi cannoni, ugelli di scarico, fiamme reattive e LED stroboscopici.
  * **Simmetria Speculare:** L'ala sinistra riutilizza la mesh dell'ala destra invertendo l'asse X locale tramite $S(-1, 1, 1)$, con scala $x = -1$ (nessun vertice duplicato).
  * **Formula:** $\mathbf{M}_{\text{componente}} = \mathbf{M}_{\text{player}} \cdot \mathbf{T}(t_{\text{rel}}) \cdot \mathbf{R}(\theta_{\text{rel}}) \cdot \mathbf{S}(s_{\text{rel}})$
  * **Albero di Trasformazione:** $\text{Player Root } (\mathbf{M} = \mathbf{T}\mathbf{R}\mathbf{S}) \to \text{Fusoliera, Cabina UV, Ala DX, Ala SX } [\mathbf{S}(-1,1,1)], \text{Fiamme, Luci}$.

### 🎙️ Cosa dire a voce (50 secondi):
> *"Per soddisfare il Requisito 3.1, ho realizzato la navicella mineraria Starfighter componendo 8 parti geometriche distinte connesse in un albero di trasformazione gerarchica.  
> La radice dell'albero rappresenta la posizione e l'orientamento globale del pilota nel mondo: $M_{\text{player}} = T \cdot R \cdot S$.  
> Ciascun componente figlio possiede una trasformazione locale che viene moltiplicata a destra per quella del genitore: $M_{\text{comp}} = M_{\text{player}} \cdot T_{\text{rel}} \cdot R_{\text{rel}} \cdot S_{\text{rel}}$.  
> Un'ottimizzazione algebrica importante riguarda le ali: per disegnare l'ala sinistra non ho duplicato i dati dei vertici su VRAM, ma ho riutilizzato la mesh dell'ala destra anteponendo una matrice di riflessione speculare con fattore di scala orizzontale negativo $S(-1, 1, 1)$."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"Perché l'ordine delle moltiplicazioni matriciali è $T \cdot R \cdot S$ e non $S \cdot R \cdot T$?"*
* **Risposta vincente:** *"In OpenGL e GLM adottiamo vettori colonna e notazione associativa da destra a sinistra. Scrivendo $M \cdot v = (T \cdot R \cdot S) \cdot v$, la matrice di scala $S$ agisce per prima sul vettore rispetto alla sua origine locale; poi la matrice di rotazione $R$ orienta l'oggetto attorno al proprio baricentro; infine la matrice di traslazione $T$ lo sposta alle coordinate globali del mondo. Se invertissimo $T$ ed $R$, l'oggetto ruoterebbe attorno all'origine del mondo $(0,0)$ con un raggio pari alla distanza di traslazione, anziché ruotare su se stesso."*

---

## SLIDE 5: Modellazione con Curve di Hermite (Requisito 3.2)
* **Punti a schermo:**
  * **Formula Vettoriale:** $p(t) = h_{00}(t)P_0 + h_{01}(t)P_1 + h_{10}(t)D_0 + h_{11}(t)D_1, \quad t \in [0, 1]$
  * **Funzioni Base:**
    * $h_{00}(t) = 2t^3 - 3t^2 + 1$
    * $h_{01}(t) = -2t^3 + 3t^2$
    * $h_{10}(t) = t^3 - 2t^2 + t$
    * $h_{11}(t) = t^3 - t^2$
  * **Raccordo $C^1$:** $C_{i-1}(t_i) = C_i(t_i)$ e $C'_{i-1}(t_i) = C'_i(t_i)$
  * 😈 **Alien Drone:** Sagoma cornuta biomeccanica (113 vertici), orbita a 8 di Lissajous, ammiccamento occhio al sorvolo.
  * 🌀 **Warp Gate:** Portale monumentale ad anello cavo (144 vertici) con vortice interno rotante.

### 🎙️ Cosa dire a voce (60 secondi):
> *"Per il Requisito 3.2 ho integrato due geometrie complesse basate su spline cubiche di Hermite.  
> Una curva di Hermite interpola due punti di controllo $P_0, P_1$ e due vettori tangenti $D_0, D_1$ mediante le quattro funzioni base cubiche $h_{00}, h_{01}, h_{10}, h_{11}$, ricavate imponendo le quattro condizioni di vincolo su valore e derivata prima agli estremi dell'intervallo $[0, 1]$.  
> La continuità parametrica $C^1$ tra tratti adiacenti è garantita imponendo nei nodi interni sia la coincidenza delle posizioni, sia l'uguaglianza dei vettori derivata tangente.  
> La prima entità è l'Alien Sentinel Drone, un ostacolo dinamico la cui testa cornuta è modellata a curve Hermite; esso pattuglia lo spazio seguendo una curva armonica chiusa di Lissajous a rapporto di frequenze 1:2 ed è dotato di un'interazione visiva: quando la navicella lo sorvola, contrae l'occhio in un ammiccamento (eye-blink).  
> La seconda entità è il Cosmic Warp Gate, un anello monumentale a curve Hermite con vortice interno rotante, che si attiva al completamento della raccolta monete."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"Qual è la differenza fondamentale tra continuità geometrica $G^1$ e continuità parametrica $C^1$?"*
* **Risposta vincente:** *"La continuità $G^1$ richiede soltanto che nei punti di giunzione i vettori tangenti abbiano la stessa direzione e verso concordi, ovvero $C'_{i}(t) = \lambda C'_{i-1}(t)$ con $\lambda > 0$. La continuità parametrica $C^1$ è una condizione più stringente: impone che anche la norma del vettore derivata sia identica ($\lambda = 1$), garantendo non solo l'assenza di spigoli visivi, ma anche una velocità di percorrenza continua senza accelerazioni istantanee infinite."*

---

## SLIDE 6: Buffer Statici GPU e Istanziazione Multipla (Requisito 4)
* **Punti a schermo:**
  * **Formula di Clip:** $v_{\text{clip}} = \mathbf{P} \cdot \mathbf{M} \cdot v_{\text{local}}$ con proiezione ortografica 2D.
  * **Buffer Sharing:** 1 solo VAO/VBO allocato in memoria con `GL_STATIC_DRAW`, riutilizzato per tutti gli asteroidi (da 4 a 9) aggiornando solo la variabile `uniform mat4 model`.
  * **Diagramma VRAM:** `VRAM: 1 solo VBO/VAO caricato all'avvio` $\to$ `CPU Loop: 9 x glUniformMatrix4fv(model)` (Zero riscrittura vertici $\to$ overhead PCIe azzerato).
  * **Monete d'Oro:** Stessa tecnica con oscillazione verticale armonica e rotazione pseudo-3D modulata da $\cos(\omega t)$.

### 🎙️ Cosa dire a voce (45 secondi):
> *"Il Requisito 4 impone di non ricaricare i dati geometrici sulla scheda video a ogni frame.  
> Per ottimizzare le prestazioni, ho implementato il Buffer Sharing: all'avvio viene allocato in VRAM un unico blocco di memoria per l'asteroide e uno per la moneta tramite `glBufferData` in modalità `GL_STATIC_DRAW`.  
> Nel game loop la CPU non invia alcuna coordinata di vertice: scorre le istanze attive, calcola la matrice Model individuale $M_i = T_i \cdot R_i \cdot S_i$, la trasferisce alla GPU tramite `glUniformMatrix4fv` ed emette la draw call `glDrawArrays`.  
> Sulle monete d'oro ho inoltre applicato una simulazione di rotazione pseudo-3D: variando la sola scala orizzontale come $s_x(t) = s_{\text{base}} \cdot |\cos(\omega t)|$, la moneta bidimensionale proiettata ortogonalmente simula all'occhio umano una rotazione solida nello spazio tridimensionale."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"Cosa memorizza esattamente un VAO rispetto a un VBO?"*
* **Risposta vincente:** *"Il VBO è un'area di memoria grezza su VRAM che contiene i byte delle coordinate e dei colori. Il VAO invece non contiene coordinate: è un oggetto di stato che memorizza la configurazione degli attributi, ovvero le chiamate a `glVertexAttribPointer` e `glEnableVertexAttribArray`. Il VAO rappresenta la 'ricetta' con cui la pipeline interpreta la memoria del VBO associato."*

---

## SLIDE 7: Shader Dedicato allo Sfondo vs Shader Oggetti (Requisito 5)
* **Punti a schermo:**
  * 🌌 **Shader Sfondo Procedurale (`background.frag`) – Senza Texture:**
    * Fullscreen quad renderizzato direttamente in coordinate normalizzate NDC $[-1, 1]$.
    * Starfield a 3 livelli di parallasse: sintesi frattale pseudo-casuale con funzione hash $h(p) = \text{fract}(\sin(p \cdot K) \cdot 43758.54)$ e scintillio temporale con $\sin(uTime)$.
    * Nebulosa fluida armonica con domain warping trigonometrico in toni ciano e magenta.
  * 🎨 **Shader Oggetti (`basic.frag` / `basic.vert`):**
    * Riceve coordinate e attributi colore interpolati per vertice (Gouraud).
    * Supporta la variabile `uniform vec4 uColorTint` per gestire feedback visivi di danno, flash da laser e invulnerabilità.

### 🎙️ Cosa dire a voce (50 secondi):
> *"Come prescritto dal Requisito 5, l'applicazione impiega due shader program distinti con pipeline specializzate.  
> Lo shader dello sfondo non fa uso di alcuna immagine pre-renderizzata: è generato matematicamente nel fragment shader sfruttando l'architettura massivamente parallela SIMD della scheda video per calcolare in tempo reale per tutti i 921.600 pixel funzioni di hash pseudo-casuale, onde polari e gradienti di nebulosa con domain warping. I vertici del quad di sfondo sono passati direttamente in coordinate normalizzate NDC $[-1, 1]$, azzerando i calcoli di matrici.  
> Lo shader degli oggetti `basic` gestisce invece la proiezione ortografica $glm::ortho$, l'interpolazione del colore di vertice tramite coordinate baricentriche e la modulazione dinamica della tinta tramite la variabile uniform `uColorTint`."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"Come funziona il passaggio di attributi dal vertex shader al fragment shader?"*
* **Risposta vincente:** *"Nel vertex shader dichiariamo la variabile di uscita `out vec4 vColor`. Lo stadio di rasterizzazione a funzione fissa calcola le coordinate baricentriche $(\alpha, \beta, \gamma)$ all'interno di ogni triangolo e interpola linearmente il valore del colore per ciascun frammento generato, consegnandolo al fragment shader come `in vec4 vColor` prima della scrittura nel framebuffer."*

---

## SLIDE 8: Gameplay Arcade, Fisica Particellare e Collisioni
* **Punti a schermo:**
  * **Collisioni Geometriche 2D:** Test analitico Cerchio–Cerchio $\|c_1 - c_2\|^2 \le (r_1 + r_2)^2$ e Cerchio–AABB tramite clamping.
  * **Fisica dei Frammenti di Roccia:** Il laser distrugge gli asteroidi in schegge animate con velocità radiale, attrito viscoso $v(t + \Delta t) = v(t)(1 - \gamma \Delta t)$ e dissolvenza alpha.
  * **Flusso di Vittoria & HURRÀ:** Raccolta 5 monete $\to$ Attivazione Warp Gate $\to$ Ingresso nel portale $\to$ Pioggia di coriandoli particellari e passaggio al livello successivo.
  * **Pop-up Centrati:** Schermate di Game Over e Hurrà perfettamente centrate su schermo con pivot $(0.5, 0.5)$.

### 🎙️ Cosa dire a voce (45 secondi):
> *"La libreria di collisione è stata sviluppata internamente privilegiando l'efficienza: il test cerchio-cerchio confronta la distanza euclidea al quadrato con il quadrato della somma dei raggi, eliminando completamente il calcolo della costosa radice quadrata: $\|c_1 - c_2\|^2 \le (r_1 + r_2)^2$.  
> Alla distruzione di un asteroide o alla raccolta di una moneta, il `ParticleSystem` genera decine di particelle animate da cinematica fisica con attrito viscoso spaziale e decadimento lineare dell'opacità, disegnate con blending trasparente.  
> Il flusso di gioco richiede di raccogliere tutte le 5 monete per energizzare il Warp Gate: attraversandolo, il sistema attiva lo stato trionfale di vittoria HURRÀ con una fontana di coriandoli celebrativi."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"Come ha configurato il blending per le particelle trasparenti?"*
* **Risposta vincente:** *"Ho abilitato il blending con `glEnable(GL_BLEND)` e configurato i fattori sorgente e destinazione con `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`, che implementa l'equazione $C_{\text{out}} = \alpha_s C_s + (1 - \alpha_s) C_d$. Inoltre, per garantire la correttezza visiva, disegno le particelle semitrasparenti rigorosamente dopo tutti gli oggetti opaci della scena."*

---

## SLIDE 9: Portabilità Totale (Zero Dipendenze Esterne)
* **Punti a schermo:**
  * 🍏 **macOS:** Apple Clang nativo (Apple Silicon M1–M4 e Intel) via CMake.
  * 🪟 **Windows:** Soluzione Visual Studio 2022 a 64-bit (`.sln` / `.vcxproj`).
  * 🐧 **Linux:** Supporto GCC / Clang standard.
  * **HUD Interattivo Dear ImGui:** Regolazione velocità navicella da slider e da scorciatoia tastiera (Shift + Frecce), indicatore vite a cuori, contatore monete e monitoraggio FPS/$\Delta t$ in tempo reale.

### 🎙️ Cosa dire a voce (35 secondi):
> *"Il progetto è stato progettato con un'attenzione particolare all'ingegneria del software e alla portabilità: tutte le librerie ausiliarie (GLFW, GLAD, GLM, ImGui, stb_image) sono incluse come codice sorgente statico all'interno del repository, consentendo la compilazione immediata 'out-of-the-box' su macOS, Linux e Windows senza installare pacchetti di sistema.  
> L'interfaccia utente in Dear ImGui è protetta dal controllo `!io.WantCaptureMouse`, permettendo di regolare la velocità della nave o monitorare le prestazioni a 60/120 FPS senza che i click del mouse interferiscano con i controlli di guida."*

---

## SLIDE 10: Grazie per l'Attenzione & Demo Live
* **Punti a schermo:**
  * *Grazie per l'attenzione*
  * Screenshot panoramico del gameplay attivo (navicella, asteroidi, monete, drone rosso cornuto e portale blu).
  * Footer accademico pulito.

### 🎙️ Cosa dire a voce (30 secondi):
> *"Questo conclude l'inquadramento teorico e architetturale del progetto.  
> Avvio ora con piacere la dimostrazione dal vivo per illustrare in tempo reale la fluidità del controllo a frecce, la rotazione delle monete d'oro, il laser al plasma, l'ammiccamento interattivo dell'Alien Drone e la sequenza di completamento del Warp Gate.  
> Sono a sua completa disposizione per qualsiasi approfondimento sul codice o sulla teoria."*

---

# GUIDA PRATICA ALLA DEMO LIVE (Cosa fare al computer)

1. **Avvio:** Apri il terminale ed esegui `./bin/Interactive2DWorld` (o avvialo da Visual Studio).
2. **Guida:** Premi le **frecce direzionali** (o WASD): mostra che la navicella accelera dolcemente e che le fiamme arancioni dei reattori si allungano realisticamente.
3. **Monete:** Sorvola una moneta d'oro: evidenzia la rotazione pseudo-3D e mostra le scintille dorate particellari al momento della raccolta.
4. **Laser & Asteroidi:** Premi la **barra spaziatrice** per sparare un laser al plasma contro un asteroide: mostra l'esplosione fisica dei 32 frammenti con attrito viscoso.
5. **Alien Drone:** Sorvola l'Alien Sentinel Drone rosso: fai notare la traiettoria a 8 di Lissajous e mostra chiaramente che, non appena la navicella ci passa sopra, **l'occhio dorato si chiude e ammicca (eye-blink)**!
6. **Vittoria:** Raccogli tutte e 5 le monete, fai notare che il Warp Gate in alto a destra si illumina, e volaci dentro per attivare la schermata centrate di **"HURRÀ!"** con la fontana di coriandoli festosi!
