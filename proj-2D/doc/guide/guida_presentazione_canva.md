# Guida e Contenuti per la Presentazione Canva
## Progetto: *Astro-Miner 2D: Cosmic Coin Rush*
### Esame di Computer Graphics – Università di Bologna (UniBo)
**Candidata:** Arianna Palmucci  
**Docente:** Prof.ssa Serena Morigi  

---

## Suggerimenti di Stile per il Template Canva
* **Tema / Palette:** Sfondo scuro cosmico (Deep Navy `#0a0e1a` o Space Black `#050811`), con accenti ciano/neon (`#00f0ff`), oro dorato (`#ffd700`) e magenta/rosso allerta (`#ff3366`).
* **Tipografia consigliata su Canva:**
  * **Titoli:** *Orbitron*, *Space Grotesk* o *Montserrat ExtraBold* (aspetto tecnologico, moderno e pulito).
  * **Testi corpo:** *Inter*, *Roboto* o *Open Sans* (massima leggibilità a distanza).
* **Regola d'oro per le slide:** Poche scritte essenziali, grafici ed elenchi puntati ad alto impatto, lasciando che siano gli screenshot del gioco e la tua voce a trasmettere la padronanza tecnica.

---

# Struttura delle Slide (12 Slide)

---

### Slide 1: Titolo e Apertura
* **Layout Canva:** Sfondo cosmico scuro con stelle sfumate. Titolo grande e luminoso al centro.
* **Titolo Principale:** *Astro-Miner 2D: Cosmic Coin Rush*
* **Sottotitolo:** *Mondo 2D Interattivo in C++17 e OpenGL 3.3 Core Profile*
* **Dati:**
  * Candidata: Arianna Palmucci
  * Corso: Computer Graphics (A.A. 2025/2026) – Università di Bologna
* **Visual da inserire:** Screenshot panoramico del gioco con navicella, portale e asteroidi.
* **Cosa dire a voce:**
  > *"Buongiorno professoressa. Oggi presento Astro-Miner 2D: Cosmic Coin Rush, un mondo interattivo bidimensionale sviluppato interamente in C++17 nativo e OpenGL 3.3 Core Profile. Il progetto implementa l'architettura della moderna pipeline programmabile, includendo modellazione gerarchica complessa, curve spline di Hermite, texture mapping, shader procedurali, fisica particellare e un'interfaccia utente interattiva basata su Dear ImGui."*

---

### Slide 2: Obiettivi e Filosofia del Progetto
* **Layout Canva:** 3 box orizzontali affiancati con icone a tema (Chip, Matrice, Joystick).
* **Titolo Slide:** *Obiettivi Architetturali e Specifiche di Progetto*
* **Punti Chiave:**
  * **Zero dipendenze esterne:** Tutte le librerie (GLFW, GLAD, GLM, ImGui, stb_image) pre-configurate o statiche.
  * **Separation of Concerns (SoC):** Disaccoppiamento netto tra algebra geometrica, pipeline shader, simulazione fisica e rendering.
  * **Zero Variabili Globali:** Incapsulamento orientato agli oggetti puro con pattern RAII e gestione sicura delle risorse hardware.
* **Visual da inserire:** Schema a blocchi architetturale sintetico (main $\rightarrow$ GameWorld $\rightarrow$ Manager).
* **Cosa dire a voce:**
  > *"L'obiettivo primario non è stato soltanto creare un'esperienza di gioco fluida e divertente, ma soprattutto costruire un motore grafico didatticamente ineccepibile. Ho eliminato qualsiasi variabile globale esposta, strutturato il codice secondo il paradigma RAII per prevenire memory leak sulla GPU e reso il progetto compilabile all'istante su macOS, Windows e Linux con una configurazione CMake autonoma."*

---

### Slide 3: Modellazione Gerarchica della Navicella (Requisito 3.1)
* **Layout Canva:** Due colonne: a sinistra la gerarchia delle matrici, a destra il dettaglio grafico della navicella.
* **Titolo Slide:** *Oggetto Composto Multilivello: Starfighter*
* **Punti Chiave:**
  * **Albero di Trasformazione a 11 nodi:** Fusoliera trapezoidale, muso a punta, cabina, ali, cannoni, ugelli, fiamme e luci LED.
  * **Algebra delle Matrici:** Ordine algebrico rigoroso $M = T \cdot R \cdot S$ in formato column-major.
  * **Simmetria Alare Ottimizzata:** Ala sinistra derivata dalla destra mediante matrice di riflessione speculare con scala negativa:
    $$\mathbf{S}(-1, 1, 1)$$
  * **Texture Mapping Cabina:** Fotografia personalizzata proiettata sopra la cabina con coordinate UV ed interpolazione bilineare.
* **Visual da inserire:** Zoom ravvicinato sulla navicella con evidenziati i componenti (ali, cabina, fiamme).
* **Cosa dire a voce:**
  > *"Per il Requisito 3.1 dell'oggetto composto, ho realizzato la navicella assemblando 11 componenti distinti. La radice del modello mappa la posizione e l'orientamento globale; tutti i componenti figli moltiplicano la propria matrice locale per quella del genitore. L'ala sinistra sfrutta l'algebra lineare specchiando l'ala destra con una scala orizzontale negativa S(-1, 1, 1), dimezzando il codice di modellazione. Sul cupolino è mappata una texture RGBA con coordinate UV caricate tramite stb_image."*

---

### Slide 4: Curve Cubiche di Hermite (Requisito 3.2)
* **Layout Canva:** Grafico delle curve a sinistra, foto dell'Alien Drone e del Warp Gate a destra.
* **Titolo Slide:** *Geometrie Avanzate: Curve Spline di Hermite*
* **Punti Chiave:**
  * **Formulazione Parametrica $C^1$:** Interpolazione cubica basata su punti di controllo $p_0, p_1$ e vettori tangenti $m_0, m_1$:
    $$p(t) = (2t^3 - 3t^2 + 1)p_0 + (t^3 - 2t^2 + t)m_0 + (-2t^3 + 3t^2)p_1 + (t^3 - t^2)m_1$$
  * **1. Alien Sentinel Drone (Ostacolo Dinamico):**
    * Traiettoria orbitale a 8 di Lissajous: $x(t) = x_0 + A\cos(\omega t),\, y(t) = y_0 + B\sin(2\omega t)$.
    * Animazione interattiva di **ammiccamento (eye-blink)** modulando la scala Y dell'occhio quando il giocatore sorvola il drone.
  * **2. Cosmic Warp Gate (Portale Monumentale):**
    * Anello cavo a curve Hermite con vortice rotante; volare dentro chiude il livello con l'HURRÀ!
* **Visual da inserire:** Screenshot del Drone alieno rosso con l'occhio giallo e dell'anello azzurro del Warp Gate.
* **Cosa dire a voce:**
  > *"Per il Requisito 3.2 ho integrato due entità modellate a spline cubiche di Hermite a continuità C1. La prima è l'Alien Sentinel Drone, un nemico che pattuglia lo spazio seguendo una curva armonica a 8 di Lissajous ed è dotato di un occhio interattivo che si chiude quando la navicella ci passa sopra. La seconda è il Cosmic Warp Gate, un portale monumentale con vortice rotante che costituisce il traguardo interattivo della missione."*

---

### Slide 5: Buffer Sharing & Instancing su VRAM (Requisito 4)
* **Layout Canva:** Schema concettuale che mostra 1 solo blocco di VRAM collegato a 6 asteroidi e 5 monete.
* **Titolo Slide:** *Efficienza della Pipeline: Condivisione dei Buffer GPU*
* **Punti Chiave:**
  * **Zero Duplicazione di Memoria:** Un solo VAO e VBO allocato su VRAM in modalità `GL_STATIC_DRAW`.
  * **Disegno Multi-Istanza:** 4–9 asteroidi e 5 monete condividono la stessa identica geometria.
  * **Uniform Model Matrix:** Per ogni istanza la CPU calcola $M_i = T_i \cdot R_i \cdot S_i$ e aggiorna la variabile `uniform mat4 model`, lasciando alla GPU il calcolo dei vertici.
* **Visual da inserire:** Diagramma illustrativo: `VRAM: 1 VAO Asteroid` $\rightarrow$ `CPU Loop: 6 x glUniformMatrix4fv + glDrawArrays`.
* **Cosa dire a voce:**
  > *"Il Requisito 4 impone di non ricaricare i vertici sulla GPU ad ogni frame. Ho implementato la tecnica del Buffer Sharing: in memoria VRAM esiste una sola copia dei vertici dell'asteroide e della moneta, residenti in buffer statici. Nel game-loop, il manager scorre le istanze attive, invia alla scheda video la rispettiva Model Matrix tramite glUniformMatrix4fv ed emette la draw call. Questo approccio azzera i trasferimenti di memoria RAM-VRAM durante il gameplay."*

---

### Slide 6: Dual Shader Architecture & Sfondo Procedurale (Requisito 5)
* **Layout Canva:** Due colonne a confronto con frammenti di codice GLSL e screenshot dello sfondo.
* **Titolo Slide:** *Architettura Shading: Due Programmi Shader GLSL*
* **Punti Chiave:**
  * **1. Shader Sfondo (`background.vert` / `background.frag`):**
    * Fullscreen Quad renderizzato direttamente in coordinate normalizzate NDC $[-1, 1] \times [-1, 1]$.
    * Sfondo procedurale matematico puro senza texture: funzione hash pseudo-casuale per 3 layer di stelle a parallasse e nebulosa fluida animata da `uTime`.
  * **2. Shader Base Oggetti (`basic.vert` / `basic.frag`):**
    * Proiezione ortografica `glm::ortho`, colori interpolati per vertice e uniform `uColorTint` per lampeggi e dissolvenze.
* **Visual da inserire:** Immagine dello sfondo stellato con nebulosa a confronto con lo shader degli oggetti.
* **Cosa dire a voce:**
  > *"Come richiesto dalla specifica (Requisito 5), l'applicazione impiega due shader program distinti con compiti separati. Lo sfondo non usa alcuna immagine pre-renderizzata: è generato matematicamente in tempo reale nel fragment shader calcolando gradienti di nebulosa ed un campo di stelle a 3 strati di profondità. Lo shader degli oggetti gestisce invece la proiezione ortografica e l'interpolazione del colore di Gouraud."*

---

### Slide 7: Animazioni e Delta Time Clamping (Requisito 6)
* **Layout Canva:** 3 schede con equazioni fisiche e grafici temporali.
* **Titolo Slide:** *Fisica e Animazioni Indipendenti dal Framerate*
* **Punti Chiave:**
  * **Indipendenza Hardware tramite $\Delta t$:**
    $$p(t + \Delta t) = p(t) + v \cdot \Delta t$$
  * **Delta-Time Clamping:** $\Delta t_{\text{effettivo}} = \min(\Delta t,\, 0.05\,\text{s})$, impedisce glitch fisici di attraversamento muri in caso di pause.
  * **Animazioni Utente vs Autonome:**
    * *Utente:* fiamme dei propulsori con rampa reattiva esponenziale guidata dalla spinta dei motori.
    * *Autonome:* oscillazione armonica verticale delle monete e rotazione pseudo-3D via $s_x(t) = \cos(\omega t)$.
* **Visual da inserire:** Grafico della funzione coseno e screenshot della moneta assottigliata in rotazione pseudo-3D.
* **Cosa dire a voce:**
  > *"Tutte le animazioni e le equazioni cinematiche sono parametrizzate sul Delta Time, garantendo velocità costante sia a 60 che a 144 Hz. Ho introdotto il Delta-Time Clamping a 50 millisecondi per prevenire il noto effetto 'bullet-through-paper'. Tra le animazioni autonome spicca la rotazione pseudo-3D delle monete: modulando la sola scala orizzontale con una funzione coseno, il cerchio 2D appare all'occhio umano come una moneta tridimensionale che rotea su se stessa."*

---

### Slide 8: Sistema di Collisioni e Meccaniche di Gioco (Requisito 7)
* **Layout Canva:** Illustrazione grafica dei volumi di collisione con cerchi e frecce.
* **Titolo Slide:** *Rilevamento Collisioni e Meccaniche Arcade*
* **Punti Chiave:**
  * **Bounding Volumes Ottimizzati:** Test Cerchio-Cerchio e Cerchio-AABB senza calcolo di radice quadrata:
    $$d^2 \le (r_1 + r_2)^2 \quad \Longleftrightarrow \quad \text{Impatto avvenuto}$$
  * **Regole del Gioco:**
    * Raccogliere **5 monete d'oro** per sbloccare l'accesso al portale dimensionale.
    * **Cannone Laser (Barra Spaziatrice):** Frantuma gli asteroidi liberando la rotta spaziale.
    * **Alien Sentinel Drone:** Se toccato 3 volte distrugge la navicella.
    * **Dardi di plasma laterali:** Missili supersonici (880 px/s) da schivare reattivamente.
* **Visual da inserire:** Screenshot del laser che colpisce un asteroide e dei cerchi di collisione.
* **Cosa dire a voce:**
  > *"La libreria di collisione sviluppata internamente si basa sulla distanza quadratica per azzerare l'impatto computazionale delle radici quadrate. Il gameplay è articolato: il pilota manovra con le frecce direzionali, raccoglie le 5 monete d'oro indicate dalla barra HUD, frantuma gli asteroidi sparando raggi laser ed evita le cariche letali di plasma che attraversano lo schermo dopo un preavviso visivo."*

---

### Slide 9: Motore Particellare Fisico
* **Layout Canva:** Sequenza di screenshot dell'esplosione di un asteroide in frammenti.
* **Titolo Slide:** *Simulazione Particellare e Dinamica dei Fluidi*
* **Punti Chiave:**
  * **Polverizzazione Asteroidi:** 32 frammenti rocciosi proiettati radialmente con velocità casuali.
  * **Attrito Viscoso Spaziale:** Smorzamento progressivo della velocità:
    $$v(t + \Delta t) = v(t) \cdot (1 - \gamma \Delta t)$$
  * **Dissolvenza Alfa e Restringimento:** Scala e canale di trasparenza sfumano a zero durante la vita residua.
  * **Feedback Visivi:** Scintille dorate alla raccolta monete, bagliori ciano alla reliquia e fuochi d'artificio per l'HURRÀ.
* **Visual da inserire:** Screenshot dell'asteroide che esplode in decine di pezzi colorati.
* **Cosa dire a voce:**
  > *"Quando un asteroide viene colpito, il ParticleSystem genera un'esplosione fisica di 32 frammenti. Ogni particella possiede posizione, velocità, orientamento, raggio e tempo di vita. Il moto è smorzato da un coefficiente di attrito viscoso, mentre la trasparenza e la scala decadono linearmente fino alla cancellazione dal vettore di memoria, garantendo un feedback visivo immediato e spettacolare."*

---

### Slide 10: Interfaccia Utente Dear ImGui e Controlli
* **Layout Canva:** Confronto tra HUD esteso all'avvio e HUD compatto post-click.
* **Titolo Slide:** *Mission Control HUD: Interfaccia Utente Intelligente*
* **Punti Chiave:**
  * **Integrazione Dear ImGui:** Pipeline di disegno OpenGL 3.3 con shader dedicati.
  * **Auto-Compressione Intelligente:** L'HUD si presenta esteso all'avvio; al primo click sull'area di gioco si comprime automaticamente nell'angolo superiore.
  * **Protezione Eventi Mouse:** Il controllo su `!io.WantCaptureMouse` impedisce la chiusura accidentale dell'HUD quando si interagisce con lo slider della velocità.
  * **Punteggio Cumulativo Permanente:** Il punteggio di sessione persiste tra i vari restart di livello.
* **Visual da inserire:** Screenshot dell'HUD di gioco aperto con slider velocità, vite a cuori e progresso monete.
* **Cosa dire a voce:**
  > *"L'interfaccia utente è realizzata con Dear ImGui. Ho curato nei dettagli l'esperienza utente: la finestra si apre con la guida completa ai comandi; al primo click sulla scena si riduce automaticamente ad un HUD compatto per non ostruire la visuale. Interagendo con lo slider della velocità navicella o con i pulsanti, la finestra non si chiude perché ho vincolato il ridimensionamento al flag !io.WantCaptureMouse. Il punteggio totale rimane cumulativo durante l'intera sessione di gioco."*

---

### Slide 11: Matrice dei Requisiti e Conformità Ufficiale
* **Layout Canva:** Tabella elegante con spunte verdi di verifica.
* **Titolo Slide:** *Sintesi di Conformità alle Specifiche della Docente*
* **Tabella Sintetica:**

| Criterio Specifica | Requisito Richiesto | Implementato nel Progetto |
| :--- | :--- | :---: |
| **3.1 Primitive Composte** | $\ge 1$ oggetto da $\ge 3$ parti | ✅ Navicella Starfighter a 11 parti gerarchiche |
| **3.2 Curve di Hermite** | $\ge 2$ forme distinte, $\ge 1$ dinamica | ✅ Alien Drone (Lissajous + Eye-Blink) & Warp Gate |
| **4. Trasformazioni/Istanze** | Model matrix CPU $\rightarrow$ GPU; $\ge 3$ istanze condivise | ✅ Asteroidi e Monete su singolo VAO `GL_STATIC_DRAW` |
| **5. Due Shader Program** | $\ge 2$ shader; sfondo procedurale puro | ✅ Shader Sfondo procedurale NDC + Shader Basic |
| **6. Animazioni / $\Delta t$** | $\ge 2$ animazioni indipendenti da FPS | ✅ Fiamme reattive, orbita a 8, rotazione pseudo-3D |
| **7. Interazioni / Collisioni** | Movimento Frecce; raccolta $\ge 5$ oggetti | ✅ Frecce Direzionali, 5 monete d'oro, laser, Warp Gate |
| **8. Organizzazione Codice** | SoC, no globali, C++17, docs complete | ✅ Zero variabili globali, RAII puro, CMake universale |

* **Cosa dire a voce:**
  > *"In questa tabella riassuntiva ho mappato ciascuno dei punti di valutazione formale previsti dalla scheda del progetto. Ogni singolo requisito è stato soddisfatto e documentato dettagliatamente sia nei commenti al codice sia nella relazione tecnica in LaTeX."*

---

### Slide 12: Chiusura e Sessione Demo Live
* **Layout Canva:** Schermata di vittoria centrata "HURRÀ! SEI ENTRATO NEL WARP GATE!" con coriandoli.
* **Titolo Slide:** *Astro-Miner 2D – Live Demonstration*
* **Punti di Sintesi:**
  * Codice sorgente C++17 commentato istruzione per istruzione.
  * Relazione tecnica accademica LaTeX e guida multipiattaforma incluse.
  * Disponibile per la dimostrazione interattiva e per le domande del colloquio.
* **Testo di Chiusura:** *Grazie per l'attenzione!*
* **Cosa dire a voce:**
  > *"Questo conclude la presentazione teorica di Astro-Miner 2D. Avvio ora la dimostrazione dal vivo del gioco per illustrare in tempo reale la fluidità del controllo a frecce, la rotazione pseudo-3D delle monete d'oro, la frantumazione degli asteroidi con il laser, l'ammiccamento dell'Alien Drone e la sequenza di salto nel Cosmic Warp Gate. Sono a completa disposizione per qualsiasi domanda tecnica sul codice."*

---

### Come Usare Questo Contenuto su Canva:
1. Apri **Canva** e cerca il template: **"Presentazione Tecnologica"** o **"Tech / Gaming Pitch Deck"** (formato 16:9).
2. Applica la palette scura cosmica indicata in cima a questo documento.
3. Copia e incolla i titoli e i bullet point sintetici di ciascuna delle 12 slide.
4. Trascina all'interno delle slide gli screenshot del gioco già presenti nel tuo repository (trovi le immagini in alta risoluzione nella documentazione e negli screenshot di test).
5. Tieni stampate le note di accompagnamento (*"Cosa dire a voce"*) durante l'esposizione orale per una presentazione brillante e sicura!
