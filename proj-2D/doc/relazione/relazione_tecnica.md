# Relazione Tecnica – Elaborato 1
# Progetto Midterm: "Interactive 2D World"

**Insegnamento:** Computer Graphics (Università di Bologna)  
**Corso di Laurea:** Laurea in Informatica / Ingegneria Informatica – A.A. 2025/2026  
**Candidata:** Arianna Palmucci  
**Titolo del Progetto:** *Astro-Miner 2D: Cosmic Coin Rush*  
**Tecnologie Adottate:** C++17, OpenGL 3.3 Core Profile, GLSL 330, GLFW, GLAD, GLM, Dear ImGui, stb_image  

---

## 1. Introduzione e Obiettivo del Progetto

Il presente elaborato documenta la progettazione e la realizzazione di un'applicazione grafica 2D interattiva, sviluppata nel rispetto rigoroso delle specifiche del primo progetto midterm per il corso di Computer Graphics.  
L'applicazione, denominata **Astro-Miner 2D: Cosmic Coin Rush**, è concepita come un videogioco arcade spaziale ad alta reattività in cui il giocatore pilota una navicella mineraria avanzata all'interno di un settore orbitale instabile.

### Dinamica di Gioco su Due Fasi:
1. **Fase 1 (Raccolta Mineraria):** Il giocatore naviga lo spazio schivando asteroidi e dardi al plasma, raccogliendo esattamente **5 Monete Galattiche** animate con rotazione 3D fittizia.
2. **Fase 2 (Attivazione e Salto Dimensionale):** Al prelievo della quinta moneta, il **Cosmic Warp Gate** (modellato con curve di Hermite) si attiva con bagliori dorati e azzurri; il giocatore deve volare fisicamente all'interno del Cosmic Warp Gate per chiudere la partita e scatenare l'evento di vittoria **HURRÀ!**, incassando 500 punti bonus e generando il livello successivo.

### Elementi di Minaccia e Dinamismo:
* **Alien Sentinel Drone (Drone Nemico Alieno):** Modellato con curve di Hermite, fluttua secondo una traiettoria a 8 ed è un **ostacolo letale**: passare sopra al drone nemico sottrae 1 vita per ciascuna collisione. Avendo a disposizione 3 vite, dopo 3 contatti scatta il **Game Over** con banner centrato a schermo. A ogni passaggio, l'occhio giallo alieno sbatte le palpebre con un fluido ammiccamento.
* **Cannone Laser e Frantumazione Rocce:** La barra spaziatrice aziona doppi cannoni al plasma che polverizzano gli asteroidi intercettati in 28 schegge fisiche animate con attrito viscoso e decadimento alpha.
* **Incursione Raggi Laterali:** Dardi cremisi ad alta velocità preceduti da linee di avvertimento lampeggianti sui bordi dello schermo.

---

## 2. Architettura Software e Organizzazione del Codice

L'architettura adotta una rigida **separazione delle responsabilità (Separation of Concerns)**, suddividendo il codice in moduli indipendenti e riutilizzabili:

```text
proj-2D/
├── include/ & src/
│   ├── Core:
│   │   ├── Input.h / .cpp           --> Gestione eventi di input da periferica (tastiera/mouse)
│   │   └── Transform.h              --> Incapsulamento matrici di trasformazione 2D e gerarchie
│   ├── Graphics & Shaders:
│   │   ├── ShaderMaker.h / .cpp     --> Compilazione e linking dei programmi GLSL
│   │   ├── Shader.h / .cpp          --> Gestione program ID e uniform (mat4, vec4, float)
│   │   ├── MeshData.h               --> Struttura CPU per vertici, colori e primitiveType
│   │   ├── Geometry.h / .cpp        --> Incapsulamento risorse GPU (VAO, VBO posizioni e colori)
│   │   ├── ShapeGenerator.h / .cpp  --> Fabbrica di primitive geometriche (CPU)
│   │   ├── PlayerShapeLoader.h      --> Parser e triangolatore per file OBJ/MTL
│   │   ├── Texture.h / .cpp         --> Caricamento texture 2D tramite stb_image.h
│   │   └── BackgroundRenderer.h/.cpp--> Render pass dedicato per lo sfondo procedurale su quad NDC
│   ├── Entities & Gameplay:
│   │   ├── Player.h / .cpp          --> Navicella composta (8 parti), texture pilota e spinta
│   │   ├── Collectible.h / .cpp     --> Gestione delle 5 monete d'oro con animazione 3D fittizia
│   │   ├── AsteroidManager.h / .cpp --> Istanze multiple condivise e rilevamento impatti
│   │   ├── HermiteObject.h / .cpp   --> Gestore dei modelli OBJ a spline di Hermite (drone e gate)
│   │   ├── LaserManager.h / .cpp    --> Proiettili laser del giocatore
│   │   ├── HazardManager.h / .cpp   --> Raggi di pericolo dai lati con allerta visiva
│   │   └── ParticleSystem.h / .cpp  --> Simulazione fisica di schegge, scintille e coriandoli
│   ├── Collision & Math:
│   │   └── Collider2D.h             --> Algoritmi di intersezione Cerchio-Cerchio e AABB
│   └── Orchestration & UI:
│       ├── GameWorld.h / .cpp       --> Macchina a stati finiti (Playing, Hurrà, GameOver)
│       └── main.cpp                 --> Game loop, inizializzazione GLFW/GLAD e Dear ImGui
```

### Game Loop e Delta Time
A ogni iterazione del ciclo principale (`while (!glfwWindowShouldClose)`), il Delta Time $\Delta t = \min(t_{current} - t_{last}, 0.05)$ viene integrato per aggiornare la fisica e le animazioni, rendendo l'esperienza indipendente dalla frequenza di refresh del monitor (60 Hz, 120 Hz, 144 Hz).

---

## 3. Modellazione Geometrica e Curve di Hermite

### 3.1 Assemblaggio Gerarchico dell'Oggetto Composto (Navicella a 8 Parti)
La navicella del giocatore (\texttt{Player.cpp}) è un oggetto composito gerarchico a **8 componenti geometriche distinte**:
1. **Muso Anteriore:** Triangolo affusolato con gradiente ciano-turchese.
2. **Fusoliera Centrale:** Trapezio isoscele rastremato ($24 \times 48$ px) a sfumatura metallica blu scuro/titanio.
3. **Cupola Cabina:** Ellisse discretizzata su 32 settori in cristallo ciano semitrasparente.
4. **Foto Pilota con Ritaglio Ellittico:** Proiezione della texture reale del pilota sopra la cabina tramite shader dedicato (`texture.vert` e `texture.frag`), mascherata con equazione ellittica morbida.
5. **Riflesso Geometrico 2D del Vetro:** Sotto-ellisse eccentrica bianca sfumata (decal geometrica semitrasparente) disegnata sopra la texture per conferire brillantezza ottica al vetro senza modelli di illuminotecnica 3D.
6. **Ali a Delta Simmetriche Speculari:** Coppia di ali a freccia. L'ala sinistra è generata specchiando la geometria dell'ala destra mediante matrice di scala negativa sull'asse locale: $S(-1, 1, 1)$.
7. **Cannoni Laser Alari:** Bocche da fuoco rettangolari innestate sulle ali.
8. **Doppi Propulsori Reattivi e Fiamme al Plasma:** Ugelli posteriori con fiamme a doppio strato (nucleo azzurro-bianco + alone esterno arancione).
9. **Indicatori LED Stroboscopici di Posizione:** Piccoli cerchi alari 2D lampeggianti alternati (verde a dritta, rosso a sinistra).

La matrice di ciascun componente è calcolata tramite la composizione:
$$M_{world} = M_{player} \cdot T(t_{rel}) \cdot R(\theta_{rel}) \cdot S(s_{rel})$$

---

### 3.2 Modellazione mediante Curve Cubiche di Hermite (Requisito 3.2)
Dato un segmento $t \in [0, 1]$ con estremi $P_0, P_1$ e tangenti $R_0, R_1$:
$$p(t) = H_0(t) P_0 + H_1(t) P_1 + H_2(t) R_0 + H_3(t) R_1$$
dove:
$$\begin{cases}
H_0(t) = 2t^3 - 3t^2 + 1 \\
H_1(t) = -2t^3 + 3t^2 \\
H_2(t) = t^3 - 2t^2 + t \\
H_3(t) = t^3 - t^2
\end{cases}$$

### Q&A Teorico: Come si garantisce la continuità delle curve di Hermite? (Continuità Tangenziale $C^1$)
* **Definizione Analitica:** La continuità $C^0$ impone che i punti estremi di due segmenti contigui coincidano: $p_k(1) = p_{k+1}(0) = P_{k+1}$.  
  La continuità tangenziale $C^1$ richiede che anche i vettori derivata prima $p'(t)$ coincidano nei nodi di giunzione:
  $$p'_k(1) = p'_{k+1}(0) \iff R_{k+1}^- = R_{k+1}^+$$
  Derivando la forma di Hermite:
  $$p'(t) = (6t^2 - 6t)P_0 + (-6t^2 + 6t)P_1 + (3t^2 - 4t + 1)R_0 + (3t^2 - 2t)R_1$$
  si osserva che $p'(0) = R_0$ e $p'(1) = R_1$. Dunque, allineando e uguagliando i vettori tangenti nei punti di giunzione si garantisce una curva liscia e priva di cuspidi.
* **Allineamento nel Modellatore di Laboratorio:** Sia il drone alieno che l'anello Warp Gate sono stati modellati interattivamente nell'applicazione di laboratorio (Hermite Modeler), che vincola i vettori tangenti dei nodi di giunzione a essere collineari e di pari intensità ($C^1$ continuity). La mesh poligonale chiusa è stata campionata ed esportata fedelmente nei file Wavefront OBJ/MTL (`alien_drone.obj` e `warp_gate.obj`).

### 3.3 Conformità Didattica: Esclusioni Esplicite da Specifica di Corso (Midterm 2D)
Nel pieno rispetto dei vincoli e dei confini didattici dell'Elaborato 1 (Interactive 2D World):
* **Curve di Bézier, B-spline e NURBS:** Nel progetto sono impiegate unicamente **spline cubiche di Hermite a continuità tangenziale $C^1$**. Non sono presenti curve di Bézier (polinomi di Bernstein, algoritmo di de Casteljau), B-spline (vettori dei nodi, algoritmo di De Boor) o NURBS (funzioni razionali), argomenti riservati a parti successive dell'insegnamento.
* **Materiali e Modelli di Illuminazione (Phong, Blinn-Phong, Lambert):** Trattandosi di un mondo bidimensionale nativo, **non è implementato alcun modello di illuminazione** (nessun calcolo di normali per vertice/frammento, nessuna componente ambientale/diffusa/speculare, nessuna sorgente di luce direzionale o puntiforme). Il rendering si basa su colori per vertice (interpolazione lineare) e modulazione uniforme. I file Wavefront MTL svolgono il solo compito di tavolozze colore flat RGB statiche.
* **Ray Tracing e Path Tracing:** Il rendering sfrutta esclusivamente la classica architettura a **rasterizzazione** della pipeline programmabile OpenGL 3.3 Core Profile (VAO, VBO, Vertex Shader e Fragment Shader GLSL). Non è presente alcun algoritmo di tracciamento di raggi o integrazione Monte Carlo.

---

## 4. Trasformazioni Geometriche e Ottimizzazione GPU (Requisito 4)

La pipeline trasforma le coordinate locali in clip space secondo:
$$v_{clip} = P \cdot M \cdot v_{local}$$
dove $P = \text{glm::ortho}(0, \text{SCR\_W}, 0, \text{SCR\_H}, -1, 1)$ e $M = T \cdot R \cdot S$.

### Q&A Architetturale: Perché non riscrivere i vertici a ogni frame? (GL_STATIC_DRAW vs Model Matrix)
* **Allocazione Statica in VRAM (`GL_STATIC_DRAW`):** All'inizializzazione, le geometrie degli oggetti (asteroidi, monete, parti del caccia) vengono caricate nei Vertex Buffer Object (VBO) della GPU una volta per tutte con il flag `GL_STATIC_DRAW`. Il driver grafico alloca tali dati permanentemente nella memoria video ad alta velocità.
* **Eliminazione del Bottleneck CPU-GPU:** Modificare le coordinate dei vertici su CPU e trasferirle ogni frame tramite `glBufferSubData` creerebbe un collo di bottiglia critico sul bus PCI-Express e forzerebbe la sincronizzazione CPU-GPU.
* **Trasformazioni nello Shader con GLM:** La CPU computa un'unica matrice $4 \times 4$ omogenea per ciascun oggetto tramite la libreria GLM ($M = T \cdot R \cdot S$) e la trasmette alla GPU con una singola chiamata `glUniformMatrix4fv` (`uniform mat4 model`).
* **Parallelismo Massivo della GPU:** È il Vertex Shader (`basic.vert`) a moltiplicare ciascun vertice per la matrice $P \cdot M$ in parallelo su centinaia di core GPU simultaneamente. Ciò permette di mantenere oltre 120 FPS stabili con utilizzo CPU prossimo allo zero.

### Istanziazione Multipla Condivisa
La medesima mesh dell'asteroide (`m_asteroidGeo`) viene condivisa da **4–6 istanze simultanee**, iterando nel rendering e aggiornando unicamente la matrice `uniform mat4 model`. La stessa tecnica governa le 5 monete d'oro.

---

## 5. Pipeline di Rendering e Shader GLSL (Requisito 5)

### Q&A Shading: Come funziona lo sfondo cosmico senza texture?
* **Zero Memoria Texture (Procedurale Puro):** Lo sfondo non carica immagini bitmap o skybox precalcolate; è interamente sintetizzato matematicamente pixel per pixel all'interno del fragment shader (`background.frag`) su un quadrilatero fullscreen a coordinate normalizzate NDC $[-1, 1]$.
* **Funzione Hash Pseudo-Casuale ad Alta Frequenza:** I pixel delle stelle sono generati matematicamente applicando una funzione hash deterministica alle coordinate di griglia discretizzate nello spazio dello schermo:
  $$h(p) = \text{fract}\left(\sin(p \cdot (12.9898, 78.233)) \cdot 43758.5453\right)$$
  Isolando i valori con $h(p) > 0.985$ e calcolando la distanza radiale rispetto al centro della cella, si generano sorgenti puntiformi luminose con decadimento morbido verso i bordi.
* **Parallasse Cosmico a Tre Livelli di Profondità:** Il campionamento della funzione hash avviene su **tre layer indipendenti** a frequenze spaziali e velocità di scorrimento differenziate:
  1. *Layer Profondo (Sfondo remoto):* Griglia densa ($120\times$), stelle minute e fitte con scorrimento lentissimo.
  2. *Layer Intermedio (Midfield):* Griglia media ($70\times$), stelle di media magnitudine a velocità moderata.
  3. *Layer in Primo Piano (Foreground):* Griglia ampia ($35\times$), stelle luminose con pulsazione armonica luminosa $\sin(3.5 \cdot uTime + \phi)$.
  Il moto asincrono dei tre strati genera un effetto di parallasse tridimensionale naturale.
* **Nebulosa Fluida Armonica & Domain Warping Psichedelico:** Sovrapposizione di onde sinusoidali bidimensionali a doppia frequenza combinate con **domain warping trigonometrico** ($p' = p + \alpha (\sin(2.5 y + \omega t), \cos(2.5 x - \omega t))$) e coordinate polari $(r, \theta)$ per simulare flussi energetici e spirali cosmiche.
* **Alternanza Palette per Livello & Slider ImGui:** Alternanza discreta di temi cromatici tra livelli dispari (Ciano/Magenta) e pari (Oro/Viola cosmico), modulabili dinamicamente con lo slider `Effetto Psichedelico` nell'HUD.

---

## 6. Animazione e Gestione del Tempo (Requisito 6)

* **Animazione Legata all'Utente:** Fiamme dei propulsori modulate in lunghezza e larghezza in base all'intensità di spinta e oscillate con $\sin(35 \cdot t)$.
* **Animazione Autonoma dello Scenario:** Il Drone nemico descrive una traiettoria piana a 8 di Lissajous ($x(t) = 640 + 340 \cos(0.85 t)$, $y(t) = 380 + 160 \sin(1.70 t)$); scala la propria velocità con il livello ($1.0 + 0.12(L-1)$) e compie improvvisi **scatti predatori non lineari** con rollata d'attacco ed evoluzioni a campana sinusoidale prima di riallinearsi alla traiettoria; gli asteroidi ruotano stazionari sul proprio asse tra $-45^\circ/\text{s}$ e $+45^\circ/\text{s}$; le monete d'oro ruotano in 3D fittizio tramite $s_x(t) = s_0 \cdot |\cos(2.8 t + \phi)|$.
* **Fisica Particellare a Frantumazione:** All'impatto laser con un asteroide, si generano 28 schegge di roccia dotate di velocità radiale, attrito viscoso ($v(t+\Delta t) = v(t) \cdot (1 - 0.75 \Delta t)$) e dissolvenza alpha.

---

## 7. Interazione, Collisioni e Gameplay (Requisito 7)

* **Algoritmi di Collisione:** Intersezione Cerchio-Cerchio ($\|c_1 - c_2\|^2 \le (r_1 + r_2)^2$) per navicella, monete, asteroidi e drone; Cerchio-AABB per i proiettili laser.
* **3 Vite e Letalità dell'Alien Drone:** L'Alien Drone nemico è letale: ogni collisione toglie 1 vita e attiva 1.6s di invulnerabilità con lampeggio rosso; dopo 3 impatti la partita termina con il **Game Over** (banner centrato a schermo con pivot `(0.5, 0.5)`). **Meccanica di Indebolimento**: sparando al Drone Rosso con la barra spaziatrice, la sentinella viene indebolita per 4.5s (velocità ridotta del 65%, scatti bloccati, scala ridotta e occhio stordito) diventando temporaneamente innocua.
* **Scaling Difficoltà e Raggi Pericolosi:** La frequenza dei pericoli e la velocità dei proiettili scalano con il livello. Dal **Livello 5** compaiono dardi laser verticali discendenti dall'alto con allarmi orizzontali lampeggianti.
* **Vittoria tramite Cosmic Warp Gate:** All'acquisizione delle 5 monete, il Cosmic Warp Gate a destra si accende; volandovi fisicamente dentro la partita si conclude con successo scatenando l'**HURRÀ!**, pioggia di coriandoli e passaggio al livello successivo.
* **Controlli con le Frecce Direzionali e Finestra HUD Ridimensionabile:** Movimento omnidirezionale affidato principalmente alle **Frecce Direzionali** (o tasti WASD); slider per la velocità di volo da 120 a 850 px/s (con scorciatoie Shift + Su/Giù e +/-); finestra HUD che all'avvio è aperta con tutte le istruzioni e al **primo click del mouse** si ridimensiona automaticamente in una vista compatta ed elegante; punteggio **cumulativo** persistente per l'intera sessione di gioco finché non si chiude l'app.
* **Spunta Wireframe Didattica (Laboratorio 22 Ottobre):** Controllo interattivo `ImGui::Checkbox("Wireframe", &m_wireframe)` che commuta il rasterizzatore su `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)` per mostrare in tempo reale la maglia triangolata di navicella, asteroidi e spline di Hermite, ripristinando automaticamente `GL_FILL` per l'interfaccia utente.

---

## 8. Tabella di Rispondenza ai Requisiti di Progetto

| Requisito Docente | Stato | File / Moduli di Riferimento | Implementazione & Utilizzo nel Progetto |
| :--- | :---: | :--- | :--- |
| **3.1 Oggetto Composto** | ✅ | `Player.h/.cpp`, `ShapeGenerator.cpp`, `texture.frag` | Navicella gerarchica a 8 componenti: fusoliera, muso a freccia, ali speculari tramite $S(-1,1,1)$, cannoni, fiamme al plasma e texture 2D del pilota su cupolino ellittico. |
| **3.2 Curve di Hermite** | ✅ | `HermiteObject.h/.cpp`, `alien_drone.obj`, `warp_gate.obj` | Due modelli a continuità tangenziale $C^1$ esportati da editor: *Alien Drone* letale a 3 colpi con occhio animato su traiettoria a 8, e *Warp Gate* interattivo come traguardo di vittoria. |
| **4 Trasformazioni & Istanze** | ✅ | `AsteroidManager.h/.cpp`, `Collectible.h/.cpp`, `Transform.h` | Matrici affini $M = T \cdot R \cdot S$ con GLM. Buffer GPU statici (`GL_STATIC_DRAW`): 4--6 asteroidi e 5 monete condividono gli stessi VAO/VBO variando la sola Model Matrix. |
| **5 Due Shader Program GLSL** | ✅ | `background.vert/.frag`, `basic.vert/.frag`, `BackgroundRenderer.cpp` | 1) Sfondo procedurale fullscreen senza texture con hash a 3 layer di parallasse cosmica e nebulosa fluida; 2) Shader oggetti per colori per vertice e modulazione uniforme `uColorTint`. |
| **6 Animazioni & Delta Time** | ✅ | `Player.cpp`, `HermiteObject.cpp`, `Collectible.cpp`, `AsteroidManager.cpp` | Indipendenza da FPS tramite $\Delta t$: propulsori reattivi alla spinta, orbita di Lissajous del drone, rotazione asteroidi e rotazione 3D fittizia delle monete con $s_x(t) = s_0 \|\cos(\omega t)\|$. |
| **7 Interazioni & Collisioni** | ✅ | `Collider2D.h`, `LaserManager.cpp`, `HazardManager.cpp`, `ParticleSystem.cpp` | Navigazione con Frecce Direzionali (o WASD), finestra HUD ridimensionabile al primo click, punteggio cumulativo, slider velocità, sparo laser con frantumazione rocce in 28 schegge fisiche, dardi da schivare, 3 vite e ingresso nel Warp Gate. |
| **8 Organizzazione & Portabilità** | ✅ | `proj-2D/src/`, `include/`, `CMakeLists.txt`, `AstroMiner.app` | Struttura C++17 modulare su 18 file sorgente (SoC). Dipendenze incluse in `external/`. Lanciatore cliccabile ad app per macOS (`AstroMiner.app`) e script batch per Windows. |

---

## 9. Istruzioni per l'Avvio e Compilazione Multipiattaforma

Grazie all'inclusione completa delle librerie in `proj-2D/external/` (GLAD, GLM header-only, Dear ImGui, librerie statiche GLFW per macOS `libglfw3.a` e Windows `glfw3.lib`, stb_image), il progetto **non richiede l'installazione di gestori di pacchetti esterni**.

### Avvio Rapido con Doppio Click (Nessun Terminale Necessario)
* **macOS (Apple Silicon o Intel):** Fare doppio click sull'applicazione **`AstroMiner.app`** oppure sul file **`Avvia_AstroMiner.command`** nella cartella `proj-2D/`. L'applicazione compila automaticamente in background (se necessario) e avvia il gioco a finestra.
* **Windows (10 / 11):** Fare doppio click su **`Avvia_AstroMiner.bat`**.

### Compilazione Manuale da Riga di Comando (CMake)
* **macOS e Linux:**
  ```bash
  cd proj-2D
  cmake -B build
  cmake --build build
  ./build/Interactive2DWorld
  ```
* **Windows (Visual Studio 2022 x64):**
  ```bash
  cd proj-2D
  cmake -B build -G "Visual Studio 17 2022" -A x64
  cmake --build build --config Release
  .\build\Release\Interactive2DWorld.exe
  ```
