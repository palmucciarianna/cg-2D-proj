# GUIDA ALLA PRESENTAZIONE ORALE (CANVA)
## Astro-Miner 2D: Cosmic Coin Rush
**Candidata:** Arianna Palmucci  
**Docente:** Prof.ssa Damiana Lazzaro – Università di Bologna  
**Corso:** Computer Graphics (A.A. 2026/2027 – Parziale I)  
**Tecnologie:** C++17 nativo, OpenGL 3.3 Core Profile, GLSL, GLFW, GLAD, GLM, Dear ImGui

---

# STRUTTURA DEL DISCORSO ORALE (10 SLIDE – 7/10 MINUTI)

Questa guida è allineata **pagina per pagina** con la tua presentazione Canva definitiva (senza demo giocata dal vivo durante le slide).  
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

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~35 sec):
> *"Buongiorno professoressa. Oggi presento **Astro-Miner 2D: Cosmic Coin Rush**, un mondo 2D interattivo sviluppato in **C++17** e **OpenGL 3.3 Core Profile**.*  
> *È un arcade spaziale in cui la navicella deve raccogliere 5 monete cosmiche evitando asteroidi, raggi di plasma e un drone nemico, fino ad aprire e attraversare il Warp Gate finale.*  
> *Dal punto di vista grafico, il progetto implementa la pipeline programmabile GLSL, modellazione gerarchica, curve di Hermite, buffer sharing e uno shader procedurale per lo sfondo."*

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

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~45 sec):
> *"In questa slide riassumo come il progetto soddisfa tutti i requisiti dell'elaborato:*  
> *Sul piano **geometrico**, ho modellato una navicella composita ad albero gerarchico e due entità basate su curve spline cubiche di Hermite.*  
> *Sul piano delle **prestazioni**, ho applicato il Buffer Sharing su buffer statici in VRAM (`GL_STATIC_DRAW`) per non ricaricare i vertici a ogni frame.*  
> *Sul piano dello **shading**, uso shader GLSL distinti: uno per gli oggetti 2D, uno per la texture ellittica della cabina e uno interamente procedurale per lo sfondo.*  
> *Infine, tutta la simulazione è legata al **Delta Time**, così fisica e velocità restano identiche su qualsiasi monitor."*

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

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~55 sec):
> *"L'architettura segue la **Separation of Concerns**, senza variabili globali esposte e con gestione **RAII** delle risorse GPU (`Geometry`, `Shader`, `Texture` rilasciano i buffer nei distruttori).*  
> *`main.cpp` inizializza GLFW, GLAD, l'alpha blending e Dear ImGui, poi esegue il game loop parlando solo con **`GameWorld`**, che fa da **Facade** e gestisce una **FSM** a 3 stati: `PLAYING`, `LEVEL_CLEARED_HURRA` e `GAME_OVER`.*  
> *Ogni frame segue 4 fasi: calcolo del `deltaTime` con **clamping a 0.05 s** (per evitare glitch o attraversamento degli ostacoli in caso di lag), aggiornamento della fisica e delle collisioni, rendering **Back-to-Front** (sfondo -> oggetti solidi -> navicella -> particelle trasparenti) e infine disegno dell'HUD Dear ImGui prima dello `glfwSwapBuffers`."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"Cosa succede se il gioco subisce un lag improvviso o la finestra viene messa in pausa?"*
* **Risposta vincente:** *"Ho introdotto il Delta-Time Clamping: limito il $\Delta t$ effettivo a un massimo di 50 millisecondi ($\Delta t_{\text{effettivo}} = \min(\Delta t, 0.05\,\text{s})$). Questo previene il noto fenomeno di tunneling (bullet-through-paper), impedendo che la navicella o i proiettili attraversino gli asteroidi a causa di un passo temporale troppo grande."*

---

## SLIDE 4: Assemblaggio dell'Oggetto Composto (Requisito 3.1)
* **Punti a schermo:**
  * **Scomposizione in Primitive:** Fusoliera trapezoidale, muso a freccia, cabina con ritratto UV ellittico (`texture.frag` con `discard` + `smoothstep`), ali a delta, doppi cannoni, ugelli di scarico, fiamme reattive e LED stroboscopici.
  * **Simmetria Speculare:** L'ala sinistra (e il cannone sinistro) riutilizza la mesh destra invertendo l'asse X locale tramite $S(-1, 1, 1)$, con scala $x = -1$ (nessun vertice duplicato).
  * **Formula:** $\mathbf{M}_{\text{componente}} = \mathbf{M}_{\text{player}} \cdot \mathbf{T}(t_{\text{rel}}) \cdot \mathbf{R}(\theta_{\text{rel}}) \cdot \mathbf{S}(s_{\text{rel}})$
  * **Albero di Trasformazione:** $\text{Player Root } (\mathbf{M} = \mathbf{T}\mathbf{R}\mathbf{S}) \to \text{Fusoliera, Cabina UV, Ala DX, Ala SX } [\mathbf{S}(-1,1,1)], \text{Fiamme, Luci}$.

### 🎙️ Cosa dire a voce (50 secondi):
> *"Per soddisfare il Requisito 3.1, ho realizzato la navicella mineraria Starfighter componendo 8 parti geometriche distinte connesse in un albero di trasformazione gerarchica.  
> La radice dell'albero rappresenta la posizione e l'orientamento globale del pilota nel mondo: $M_{\text{player}} = T \cdot R \cdot S$.  
> Ciascun componente figlio possiede una trasformazione locale che viene moltiplicata a destra per quella del genitore: $M_{\text{comp}} = M_{\text{player}} \cdot T_{\text{rel}} \cdot R_{\text{rel}} \cdot S_{\text{rel}}$.  
> Un'ottimizzazione algebrica importante riguarda le ali: per disegnare l'ala sinistra non ho duplicato i dati dei vertici su VRAM, ma ho riutilizzato la mesh dell'ala destra anteponendo una matrice di riflessione speculare con fattore di scala orizzontale negativo $S(-1, 1, 1)$."*

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~55 sec):
> *"La navicella è un albero di trasformazione gerarchica. La radice è il `Player`, la cui matrice globale nel mondo è $M_{\text{player}} = T \cdot R \cdot S$ (con ordine prima scala, poi rotazione attorno al centro locale, poi traslazione).*  
> *Tutti i componenti figli — fusoliera, muso, cabina con texture ellittica del pilota, ali, cannoni, scarichi, fiamme animate e LED — calcolano la propria matrice come $M_{\text{figlio}} = M_{\text{player}} \cdot M_{\text{locale}}$.*  
> *Un'ottimizzazione importante è la **simmetria speculare**: per l'ala sinistra e il cannone sinistro non ho duplicato i vertici in memoria, ma ho riutilizzato la mesh destra applicando una scala orizzontale negativa $S(-1, 1, 1)$.*  
> *Inoltre, la foto del pilota nel cockpit usa `texture.frag`, che applica una maschera ellittica con `discard` fuori dal cerchio UV unitario e un bordo morbido anti-aliased tramite `smoothstep`."*

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

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~60 sec):
> *"Per le forme curvilinee organiche sono state modellate due entità tramite **spline cubiche di Hermite** a tratti, che interpolano punti di controllo $P_0, P_1$ e tangenti $D_0, D_1$ tramite le quattro funzioni base cubiche $h_{00}, h_{01}, h_{10}, h_{11}$ su $[0, 1]$, garantendo continuità $C^1$ nei punti di raccordo (stessa posizione e stesso vettore derivata).*  
> *La prima entità è l'**Alien Sentinel Drone**, un ostacolo dinamico che pattuglia lo spazio lungo una curva chiusa a 8 di **Lissajous** (rapporto frequenze 1:2): reagisce sia al sorvolo del giocatore — chiudendo l'occhio in un ammiccamento — sia ai colpi laser, che lo indeboliscono temporaneamente.*  
> *La seconda è il **Cosmic Warp Gate**, un portale ad anello cavo con vortice interno rotante che resta dormiente finché non si raccolgono tutte e 5 le monete."*

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

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~50 sec):
> *"Per soddisfare il requisito di efficienza, `ShapeGenerator` prepara i dati in CPU dentro `MeshData` (`positions`, `colors`, `primitiveType`) e `Geometry` li carica una sola volta in VRAM tramite `glGenVertexArrays`, `glGenBuffers` e `glBufferData(..., GL_STATIC_DRAW)`.*  
> *Per gli asteroidi e le monete ho applicato il **Buffer Sharing**: c'è un solo VAO/VBO condiviso per l'asteroide tipo e uno per la moneta. Nel loop di rendering la CPU non ri-trasferisce vertici sul bus PCIe, ma aggiorna solo la `uniform mat4 model` tramite `glUniformMatrix4fv` e chiama `glDrawArrays` per ogni istanza attiva.*  
> *Sulle monete ho anche simulato una rotazione **pseudo-3D** in proiezione ortografica modulando la scala orizzontale con $|\cos(\omega t)|$."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"Cosa memorizza esattamente un VAO rispetto a un VBO?"*
* **Risposta vincente:** *"Il VBO è un'area di memoria grezza su VRAM che contiene i byte delle coordinate e dei colori. Il VAO invece non contiene coordinate: è un oggetto di stato che memorizza la configurazione degli attributi, ovvero le chiamate a `glVertexAttribPointer` e `glEnableVertexAttribArray`. Il VAO rappresenta la 'ricetta' con cui la pipeline interpreta la memoria del VBO associato."*

---

## SLIDE 7: Shader Dedicato allo Sfondo vs Shader Oggetti (Requisito 5)
* **Punti a schermo:**
  * 🌌 **Shader Sfondo Procedurale (`background.frag`) – Senza Texture:**
    * Fullscreen quad renderizzato direttamente in coordinate normalizzate NDC $[-1, 1]$.
    * Starfield a 3 livelli di parallasse: griglia UV (`floor`/`fract`), funzione `hash` deterministica (`fract(p * vec2(123.34, 456.21))` + `dot`), anti-aliasing con `smoothstep` e scintillio temporale con $\sin(uTime)$.
    * Nebulosa fluida armonica con coordinate polari, domain warping trigonometrico, alternanza palette per livello (`uLevel`) e vignettatura radiale.
  * 🎨 **Shader Oggetti (`basic.frag` / `basic.vert`):**
    * Riceve coordinate e attributi colore interpolati per vertice (Gouraud).
    * Supporta la variabile `uniform vec4 uColorTint` per gestire feedback visivi di danno, flash da laser e invulnerabilità.

### 🎙️ Cosa dire a voce (50 secondi):
> *"Come prescritto dal Requisito 5, l'applicazione impiega due shader program distinti con pipeline specializzate.  
> Lo shader dello sfondo non fa uso di alcuna immagine pre-renderizzata: è generato matematicamente nel fragment shader sfruttando l'architettura massivamente parallela SIMD della scheda video per calcolare in tempo reale per tutti i 921.600 pixel funzioni di hash pseudo-casuale, onde polari e gradienti di nebulosa con domain warping. I vertici del quad di sfondo sono passati direttamente in coordinate normalizzate NDC $[-1, 1]$, azzerando i calcoli di matrici.  
> Lo shader degli oggetti `basic` gestisce invece la proiezione ortografica $glm::ortho$, l'interpolazione del colore di vertice tramite coordinate baricentriche e la modulazione dinamica della tinta tramite la variabile uniform `uColorTint`."*

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~60 sec):
> *"L'applicazione usa shader program separati, compilati e linkati a runtime da `ShaderMaker`.*  
> *Il più particolare è lo shader dello sfondo (`BackgroundRenderer` + `background.vert/.frag`): disegna un **fullscreen quad** i cui vertici sono già in coordinate normalizzate **NDC $[-1, 1]$**, quindi `background.vert` non moltiplica per `projection * model`.*  
> *Nel fragment shader tutto è generato **matematicamente senza texture**: calcolo coordinate polari e una spirale rotante con **domain warping** trigonometrico per la nebulosa (la cui palette cambia tra livelli pari e dispari tramite `uLevel`), una vignettatura radiale ai bordi e **3 layer di stelle in parallasse** basati su una griglia UV, una funzione `hash` pseudo-casuale deterministica, `smoothstep` per l'anti-aliasing e `sin(uTime)` per lo scintillio.*  
> *Al contrario, `basic.vert` e `texture.vert` usano la proiezione ortografica `glm::ortho(0, 1280, 0, 720, -1, 1)` e supportano `uColorTint` per flash di danno, invulnerabilità e dissolvenze senza toccare i VBO."*

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

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~50 sec):
> *"Tutte le collisioni sono centralizzate in `Collider2D.h`: uso test **Cerchio–Cerchio** confrontando le distanze al quadrato ($\|c_1 - c_2\|^2 \le (r_1 + r_2)^2$, evitando `sqrt`) e test **Cerchio–AABB** tramite clamping per i raggi di plasma perimetrali di `HazardManager`.*  
> *Quando un asteroide esplode o si raccoglie una moneta, `ParticleSystem` genera particelle dinamiche (`GL_DYNAMIC_DRAW` / `updatePositions`) con integrazione di Eulero, attrito viscoso $v \leftarrow v(1 - \gamma\Delta t)$ e **alpha fade** lineare su `uColorTint`, disegnate dopo gli oggetti opaci con `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`.*  
> *Raccolte le 5 monete, il Warp Gate si attiva e attraversarlo porta allo stato `LEVEL_CLEARED_HURRA` con coriandoli particellari prima del livello successivo."*

#### 📝 Versione approfondita (dettaglio completo Slide 8):
> *"La slide 8 descrive tre aspetti fondamentali del gameplay e della simulazione: il sistema di collisioni geometriche, la fisica delle particelle e la gestione del flusso di vittoria.*
>
> ***Partiamo dalle collisioni (`Collider2D.h`).***  
> *Nel progetto ho centralizzato tutti i controlli geometrici nel modulo `Collider2D.h`, utilizzando principalmente due tipi di test analitici:*  
> *1. **Test Cerchio–Cerchio** (usato per navicella, asteroidi, monete, proiettili laser e drone): ogni entità è approssimata da un centro e un raggio. Invece di calcolare la distanza euclidea con la radice quadrata, confronto direttamente la **distanza al quadrato**, calcolata tramite prodotto scalare `glm::dot(diff, diff)`, con il quadrato della somma dei raggi: $\|c_1 - c_2\|^2 \le (r_1 + r_2)^2$. In questo modo elimino l'operazione `sqrt()`, rendendo il controllo molto più veloce all'interno del game loop.*  
> *2. **Test Cerchio–AABB** (Axis-Aligned Bounding Box, cioè un rettangolo allineato agli assi cartesiani): in questo caso utilizzo il **clamping**. Limito le coordinate del centro del cerchio all'intervallo `[min, max]` del rettangolo su entrambi gli assi per trovare il punto del rettangolo più vicino al centro del cerchio; poi verifico, sempre al quadrato e senza radice, se la distanza tra quel punto e il centro è minore o uguale al raggio.*
>
> ***Passiamo alla fisica particellare e al blending (`ParticleSystem`).***  
> *Quando un asteroide viene colpito dal laser, `ParticleSystem` genera **32 frammenti** che si espandono radialmente dal punto d'impatto (e allo stesso modo genera scintille sulle monete, scie dei razzi o **135 coriandoli** alla vittoria).*  
> *- **Ciclo di vita e cinematica:** ogni particella ha posizione, velocità, rotazione, vita residua e colore RGBA (il vettore è pre-allocato con `reserve(256)` per evitare riallocazioni continue). Ad ogni frame, finché `life > 0`, la posizione avanza tramite **integrazione di Eulero** ($p \leftarrow p + v\Delta t$), la rotazione si aggiorna e applico un modello lineare di **attrito viscoso**: $v(t+\Delta t) = v(t)(1 - \gamma\Delta t)$ con $\gamma = 0.75$, così i frammenti rallentano progressivamente in modo naturale prima di scomparire.*  
> *- **Dissolvenza e Buffer Sharing:** in fase di rendering non ricarico nuovi vertici in GPU: uso un'unica geometria base condivisa e calcolo per ogni particella il rapporto `progress = life / maxLife` in $[0, 1]$, con cui riduco sia la scala sia il canale alpha del colore, inviato poi allo shader tramite la uniform `uColorTint`.*  
> *- **Alpha Blending e ordine di disegno:** per rendere corretta la trasparenza ho abilitato `glEnable(GL_BLEND)` con `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`, che implementa l'equazione $C_{\text{out}} = \alpha_s C_s + (1 - \alpha_s) C_d$ tra il colore della particella e quello già presente nel framebuffer. Inoltre, in `GameWorld::render`, le particelle sono disegnate **rigorosamente dopo tutti gli oggetti opaci** (ordine Back-to-Front) per evitare artefatti visivi.*
>
> ***Infine, il flusso di vittoria e i banner centrati (`GameWorld`).***  
> *La logica di progressione è governata dalla macchina a stati finiti di `GameWorld`: raccolte tutte e **5 le monete**, il Warp Gate diventa attivo; attraversandolo, il gioco assegna **+500 punti bonus**, passa allo stato `LEVEL_CLEARED_HURRA` per **2,8 secondi**, lancia i **135 coriandoli particellari** e poi genera automaticamente il livello successivo (`nextLevel()`).*  
> *Ho curato anche la resa grafica delle schermate di **HURRÀ!** e **Game Over** in Dear ImGui, posizionandole al centro dello schermo tramite un **pivot centrale `(0.5, 0.5)`** rispetto a `DisplaySize * 0.5` e centrando dinamicamente ogni riga di testo con `CalcTextSize`, così da avere un'interfaccia sempre bilanciata su qualsiasi risoluzione.*  
> *In questo modo la slide mostra come geometria analitica, fisica discreta, blending OpenGL e logica FSM collaborino insieme dentro il frame loop."*

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
> L'interfaccia utente in Dear ImGui è protetta dal controllo `!io.WantCaptureMouse`, permettendo di regolare la velocità della nave, ricaricare a caldo la texture della cabina o monitorare le prestazioni a 60/120 FPS senza che i click del mouse interferiscano con i controlli di guida."*

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~45 sec):
> *"Tutte le librerie esterne (GLFW, GLAD, GLM, Dear ImGui, `stb_image`) sono incluse nel repository, garantendo compilazione su macOS, Windows e Linux.*  
> *`Input` gestisce le callback GLFW invertendo l'asse Y del cursore rispetto all'origine in basso a sinistra di OpenGL e aggiornando `glViewport` al resize.*  
> *L'HUD in **Dear ImGui** è in modalità immediata con `io.IniFilename = nullptr` (così non crea `imgui.ini` su disco) e auto-compressione protetta da `!io.WantCaptureMouse`: mostra vite, monete, stato del drone, slider per velocità della navicella ed effetto psichedelico, la spunta **Wireframe** (`glPolygonMode`) per ispezionare le mesh e il pulsante di hot-reload della texture cabina."*

### ❓ Possibile domanda a trabocchetto:
* **Professoressa:** *"A cosa serve il pulsante 'Ricarica Texture Cabina' nell'HUD e come evita sprechi di memoria?"*
* **Risposta vincente:** *"Implementa l'hot-reload a runtime della foto del pilota (`assets/textures/player_face.bmp/.png/.jpg`) senza riavviare l'app: prima di caricare il nuovo file con `stbi_load` e `glTexImage2D`, `Texture::load()` dealloca l'handle precedente con `glDeleteTextures`, garantendo l'assenza di memory leak in VRAM."*

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

#### 🗣️ Versione estesa (8–9 min totali, senza demo live – ~25 sec):
> *"In sintesi, Astro-Miner 2D integra in un'unica architettura modulare e RAII i concetti fondamentali della grafica 2D programmabile: trasformazioni gerarchiche, spline di Hermite, buffer sharing statico, shader procedurali in NDC e simulazione fisica indipendente dal framerate.*  
> *Grazie per l'attenzione, sono a disposizione per eventuali domande sul codice o sulla teoria."*

---

# GUIDA PRATICA ALLA DEMO LIVE (Cosa fare al computer)

1. **Avvio:** Apri il terminale ed esegui `./bin/Interactive2DWorld` (o avvialo da Visual Studio).
2. **Guida:** Premi le **frecce direzionali** (o WASD): mostra che la navicella accelera dolcemente e che le fiamme arancioni dei reattori si allungano realisticamente.
3. **Monete:** Sorvola una moneta d'oro: evidenzia la rotazione pseudo-3D e mostra le scintille dorate particellari al momento della raccolta.
4. **Laser & Asteroidi:** Premi la **barra spaziatrice** per sparare un laser al plasma contro un asteroide: mostra l'esplosione fisica dei 32 frammenti con attrito viscoso.
5. **Alien Drone:** Sorvola l'Alien Sentinel Drone rosso: fai notare la traiettoria a 8 di Lissajous e mostra chiaramente che, non appena la navicella ci passa sopra, **l'occhio dorato si chiude e ammicca (eye-blink)**!
6. **Vittoria:** Raccogli tutte e 5 le monete, fai notare che il Warp Gate in alto a destra si illumina, e volaci dentro per attivare la schermata centrate di **"HURRÀ!"** con la fontana di coriandoli festosi!
