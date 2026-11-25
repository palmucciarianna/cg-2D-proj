// ORIGINE: Base dai laboratori della prof (cerchio, farfalla di Fay, cuore, luna, Sierpinski, primitive base) + estensioni per il gioco (moneta, trapezio, diamante) adattate a MeshData.
// ============================================================================
// GUIDA RAPIDA PER L'ORALE - IMPLEMENTAZIONE: ShapeGenerator.cpp
// ----------------------------------------------------------------------------
// 1. A COSA SERVE:
//    - Factory matematica per la creazione di mesh poligonali e curve parametriche su CPU.
//    - Genera cerchi, monete, rettangoli, triangoli, stelle e forme complesse (farfalla, cuore, Sierpinski).
// 2. A CHI È COLLEGATO:
//    - Fornisce strutture MeshData a Player, AsteroidManager, CollectibleManager, HermiteManager.
// 3. FORMULE & PRINCIPI CG CHIAVE:
//    - Coordinate Polari per cerchi e dischi: x = cx + r*cos(theta), y = cy + r*sin(theta) (Domanda 2, 13).
//    - Triangolazione a ventaglio (GL_TRIANGLE_FAN) e a coppie di triangoli (GL_TRIANGLES) (Domanda 14).
//    - Assegnazione dei colori di vertice per l'interpolazione baricentrica di Gouraud (Domanda 7).
// ============================================================================

#include "ShapeGenerator.h"
#include "MathConstants.h"
#include <random>
#include <cmath>

using MathConstants::PI;

// ============================================================================
// Costruzione Triangolo di Base (GL_TRIANGLES)
// ============================================================================
MeshData ShapeGenerator::createTriangle(
    const glm::vec4& color1,
    const glm::vec4& color2,
    const glm::vec4& color3)
{
    MeshData data;

    // Aggiunge i 3 vertici locali centrati rispetto all'origine:
    // Vertice 1: angolo inferiore sinistro (-0.5, -0.5)
    data.positions.push_back(glm::vec3(-0.5f, -0.5f, 0.0f));
    // Vertice 2: angolo inferiore destro (0.5, -0.5)
    data.positions.push_back(glm::vec3(0.5f, -0.5f, 0.0f));
    // Vertice 3: vertice superiore centrale (0.0, 0.5)
    data.positions.push_back(glm::vec3(0.0f, 0.5f, 0.0f));

    // Assegna i rispettivi colori ai vertici per generare un'interpolazione Gouraud liscia.
    data.colors.push_back(color1);
    data.colors.push_back(color2);
    data.colors.push_back(color3);

    // Imposta la modalità di rendering su GL_TRIANGLES (3 vertici per primitiva).
    data.primitiveType = GL_TRIANGLES;
    return data;
}

// ============================================================================
// Costruzione Rettangolo (GL_TRIANGLES - 2 Triangoli)
// ============================================================================
MeshData ShapeGenerator::createRectangle(
    float width,
    float height,
    const glm::vec4& colorTop,
    const glm::vec4& colorBottom)
{
    MeshData data;
    // Calcola la semi-ampiezza e la semi-altezza rispetto al centro.
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    // [Domanda 14: Rasterizzazione dei triangoli (rettangolo decomposto in 2 triangoli GL_TRIANGLES)]
    // Triangolo 1 (inferiore-destro): Bottom-Left, Bottom-Right, Top-Right
    data.positions.push_back(glm::vec3(-hw, -hh, 0.0f));
    data.positions.push_back(glm::vec3( hw, -hh, 0.0f));
    data.positions.push_back(glm::vec3( hw,  hh, 0.0f));
    data.colors.push_back(colorBottom);
    data.colors.push_back(colorBottom);
    data.colors.push_back(colorTop);

    // Triangolo 2 (superiore-sinistro): Bottom-Left, Top-Right, Top-Left
    data.positions.push_back(glm::vec3(-hw, -hh, 0.0f));
    data.positions.push_back(glm::vec3( hw,  hh, 0.0f));
    data.positions.push_back(glm::vec3(-hw,  hh, 0.0f));
    data.colors.push_back(colorBottom);
    data.colors.push_back(colorTop);
    data.colors.push_back(colorTop);

    data.primitiveType = GL_TRIANGLES;
    return data;
}

// ============================================================================
// Costruzione Moneta Dorata con Gradiente Radiale (GL_TRIANGLES)
// ============================================================================
MeshData ShapeGenerator::createCoin(
    float radius,
    int segments,
    const glm::vec4& innerColor,
    const glm::vec4& outerColor)
{
    MeshData data;
    // Centro geometrico della moneta.
    glm::vec3 center(0.0f, 0.0f, 0.0f);

    // Passo angolare tra due spicchi successivi: delta_theta = 2*PI / N
    float step = (2.0f * PI) / static_cast<float>(segments);

    for (int i = 0; i < segments; ++i)
    {
        // Angolo di inizio e di fine dello spicchio triangolare.
        float a1 = static_cast<float>(i) * step;
        float a2 = static_cast<float>(i + 1) * step;

        // Punti sul perimetro circolare calcolati tramite coordinate polari: x = r*cos(a), y = r*sin(a).
        glm::vec3 p1(radius * std::cos(a1), radius * std::sin(a1), 0.0f);
        glm::vec3 p2(radius * std::cos(a2), radius * std::sin(a2), 0.0f);

        // Triangolo dello spicchio: Centro -> P1 -> P2
        data.positions.push_back(center);
        data.positions.push_back(p1);
        data.positions.push_back(p2);

        // [Domanda 7: Coordinate baricentriche e interpolazione Gouraud dei colori di vertice]
        // Colore interno dorato brillante al centro, metallico scuro sul bordo esterno.
        data.colors.push_back(innerColor);
        data.colors.push_back(outerColor);
        data.colors.push_back(outerColor);
    }

    data.primitiveType = GL_TRIANGLES;
    return data;
}

// ============================================================================
// Costruzione Trapezio Isoscele (Fusoliera Navicella)
// ============================================================================
MeshData ShapeGenerator::createTrapezoid(
    float topWidth,
    float bottomWidth,
    float height,
    const glm::vec4& colorTop,
    const glm::vec4& colorBottom)
{
    MeshData data;                   // Struttura CPU di ritorno
    float h = height * 0.5f;         // Semialtezza verticale
    float hwTop = topWidth * 0.5f;   // Semilarghezza lato superiore
    float hwBottom = bottomWidth * 0.5f; // Semilarghezza lato inferiore

    // Triangolo 1: Bottom-Left, Bottom-Right, Top-Right
    data.positions.push_back(glm::vec3(-hwBottom, -h, 0.0f)); // Vertice inferiore sinistro
    data.positions.push_back(glm::vec3( hwBottom, -h, 0.0f)); // Vertice inferiore destro
    data.positions.push_back(glm::vec3( hwTop,     h, 0.0f)); // Vertice superiore destro
    data.colors.push_back(colorBottom); // Colore associato a Bottom-Left
    data.colors.push_back(colorBottom); // Colore associato a Bottom-Right
    data.colors.push_back(colorTop);    // Colore associato a Top-Right

    // Triangolo 2: Bottom-Left, Top-Right, Top-Left
    data.positions.push_back(glm::vec3(-hwBottom, -h, 0.0f)); // Vertice inferiore sinistro
    data.positions.push_back(glm::vec3( hwTop,     h, 0.0f)); // Vertice superiore destro
    data.positions.push_back(glm::vec3(-hwTop,     h, 0.0f)); // Vertice superiore sinistro
    data.colors.push_back(colorBottom); // Colore Bottom-Left
    data.colors.push_back(colorTop);    // Colore Top-Right
    data.colors.push_back(colorTop);    // Colore Top-Left

    data.primitiveType = GL_TRIANGLES; // Primitiva a triangoli indipendenti
    return data;                       // Restituisce i vertici CPU
}

// ============================================================================
// Costruzione Rombo / Diamante (Post-bruciatore Motore)
// ============================================================================
MeshData ShapeGenerator::createDiamond(
    float width,
    float height,
    const glm::vec4& centerColor,
    const glm::vec4& edgeColor)
{
    MeshData data;            // Struttura CPU di ritorno
    float hw = width * 0.5f;  // Semilarghezza orizzontale
    float hh = height * 0.5f; // Semialtezza verticale

    glm::vec3 center(0.0f, 0.0f, 0.0f); // Origine centrale del diamante
    glm::vec3 top(0.0f, hh, 0.0f);      // Vertice superiore
    glm::vec3 bottom(0.0f, -hh, 0.0f);  // Vertice inferiore
    glm::vec3 right(hw, 0.0f, 0.0f);    // Vertice destro
    glm::vec3 left(-hw, 0.0f, 0.0f);    // Vertice sinistro

    // Lambda per aggiungere un triangolo con gradiente dal centro verso la punta esterna.
    auto addTri = [&](const glm::vec3& p1, const glm::vec3& p2) {
        data.positions.push_back(center);      // Primo vertice: centro
        data.positions.push_back(p1);          // Secondo vertice perimetrale
        data.positions.push_back(p2);          // Terzo vertice perimetrale
        data.colors.push_back(centerColor);    // Colore centrale luminoso
        data.colors.push_back(edgeColor);      // Colore esterno di p1
        data.colors.push_back(edgeColor);      // Colore esterno di p2
    };

    // Costruisce i 4 quadranti del rombo in senso orario.
    addTri(top, right);    // Quadrante superiore destro
    addTri(right, bottom); // Quadrante inferiore destro
    addTri(bottom, left);  // Quadrante inferiore sinistro
    addTri(left, top);     // Quadrante superiore sinistro

    data.primitiveType = GL_TRIANGLES; // Primitiva a triangoli indipendenti
    return data;                       // Restituisce la mesh
}

// ============================================================================
// Costruzione Cerchio / Ellisse (GL_TRIANGLE_FAN)
// ============================================================================
MeshData ShapeGenerator::createCircle(
    float cx,
    float cy,
    float radiusX,
    float radiusY,
    int segments,
    const glm::vec4& centerColor,
    const glm::vec4& borderColor)
{
    (void)cx; (void)cy; // La traslazione viene applicata via Model Matrix.
    MeshData data;

    // Riserva lo spazio di memoria esatto: 1 vertice centrale + (segments + 1) vertici perimetrali.
    data.positions.reserve(segments + 2);
    data.colors.reserve(segments + 2);

    // 1. Vertice centrale all'origine locale (0,0).
    data.positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    data.colors.push_back(centerColor);

    // Passo angolare di campionamento.
    const float step = 2.0f * PI / static_cast<float>(segments);

    // [Domanda 2 & Domanda 13: Geometria vettoriale polare per la circonferenza (rasterizzata su GPU)]
    // 2. Vertici perimetrali calcolati con formule trigonometriche: x = rx*cos(t), y = ry*sin(t).
    for (int i = 0; i <= segments; ++i)
    {
        float t = static_cast<float>(i) * step;
        float x = radiusX * std::cos(t);
        float y = radiusY * std::sin(t);

        data.positions.push_back(glm::vec3(x, y, 0.0f));
        data.colors.push_back(borderColor);
    }

    // Modalità GL_TRIANGLE_FAN: il primo vertice è comune a tutti i triangoli generati.
    data.primitiveType = GL_TRIANGLE_FAN;
    return data;
}

// ============================================================================
// Costruzione Curva di Fay della Farfalla (GL_TRIANGLE_FAN)
// ============================================================================
// Equazione polare trascendente scoperta da Temple H. Fay (1989):
// r(t) = e^{cos t} - 2*cos(4t) + sin^5(t / 12)
// x(t) = radiusX * sin(t) * r(t)
// y(t) = radiusY * cos(t) * r(t)
MeshData ShapeGenerator::createButterfly(
    float cx,
    float cy,
    float radiusX,
    float radiusY,
    int segments,
    const glm::vec4& centerColor,
    const glm::vec4& borderColor)
{
    (void)cx; (void)cy;
    MeshData data;

    data.positions.reserve(segments + 2);
    data.colors.reserve(segments + 2);

    // Centro della farfalla.
    data.positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    data.colors.push_back(centerColor);

    const float step = 2.0f * PI / static_cast<float>(segments);

    for (int i = 0; i <= segments; ++i)
    {
        float t = static_cast<float>(i) * step;

        // Valutazione della componente radiale trascendente di Fay.
        float common = std::exp(std::cos(t))
                     - 2.0f * std::cos(4.0f * t)
                     + std::pow(std::sin(t / 12.0f), 5.0f);

        // Conversione in coordinate cartesiane.
        float x = radiusX * std::sin(t) * common;
        float y = radiusY * std::cos(t) * common;

        data.positions.push_back(glm::vec3(x, y, 0.0f));
        data.colors.push_back(borderColor);
    }

    data.primitiveType = GL_TRIANGLE_FAN;
    return data;
}

// ============================================================================
// Costruzione Curva Spicchio di Luna (GL_TRIANGLE_FAN)
// ============================================================================
// Equazione parametrica armonica:
// x(t) = scaleX * 3 * sin(t)
// y(t) = scaleY * (0.5 * cos(2t) - cos(t))
MeshData ShapeGenerator::createMoon(
    float cx,
    float cy,
    float scaleX,
    float scaleY,
    int segments,
    const glm::vec4& color)
{
    (void)cx; (void)cy;
    MeshData data;

    for (int i = 0; i <= segments; ++i)
    {
        float t = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments); // Parametro angolare in [0, 2*PI]

        // Valutazione delle coordinate cartesiane della curvatura lunare.
        float x = 3.0f * std::sin(t);
        float y = 0.5f * std::cos(2.0f * t) - std::cos(t);

        // Applicazione dei fattori di scala sugli assi X e Y.
        x = scaleX * x;
        y = scaleY * y;

        data.positions.push_back(glm::vec3(x, y, 0.0f)); // Inserisce il vertice calcolato
        data.colors.push_back(color);                    // Assegna il colore uniforme
    }

    data.primitiveType = GL_TRIANGLE_FAN; // Disegna a ventaglio dal primo vertice
    return data;                          // Restituisce i vertici CPU
}

// ============================================================================
// Costruzione Curva Cardioide del Cuore (GL_TRIANGLE_FAN)
// ============================================================================
// Equazioni parametriche del cuore:
// x(t) = 16 * sin^3(t)
// y(t) = 13*cos(t) - 5*cos(2t) - 2*cos(3t) - cos(4t)
MeshData ShapeGenerator::createHeart(
    float cx,
    float cy,
    float radiusX,
    float radiusY,
    int segments,
    const glm::vec4& centerColor,
    const glm::vec4& borderColor)
{
    (void)cx; (void)cy;
    MeshData data;

    data.positions.reserve(segments + 2);
    data.colors.reserve(segments + 2);

    // Centro del cuore.
    data.positions.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    data.colors.push_back(centerColor);

    const float step = 2.0f * PI / static_cast<float>(segments);

    for (int i = 0; i <= segments; ++i)
    {
        float t = static_cast<float>(i) * step;
        const float s = std::sin(t);

        // Equazione per la coordinata X.
        float x = radiusX * 16.0f * s * s * s;

        // Equazione per la coordinata Y con i 4 coefficienti armonici di Fourier.
        float y = radiusY * (
            13.0f * std::cos(t)
            - 5.0f * std::cos(2.0f * t)
            - 2.0f * std::cos(3.0f * t)
            - std::cos(4.0f * t)
        );

        data.positions.push_back(glm::vec3(x, y, 0.0f));
        data.colors.push_back(borderColor);
    }

    data.primitiveType = GL_TRIANGLE_FAN;
    return data;
}

// ============================================================================
// Costruzione Frattale di Sierpinski (Chaos Game - GL_POINTS)
// ============================================================================
// Algoritmo del Caos (Barnsley, 1988):
// 1. Si scelgono tre vertici fissi T0, T1, T2 formanti un triangolo.
// 2. Si seleziona casualmente un punto di partenza.
// 3. A ogni iterazione, si sceglie uniformemente a caso un vertice T_k (k in {0,1,2})
//    e si calcola il punto medio: p_{n+1} = (p_n + T_k) / 2.
// 4. L'insieme dei punti generati converge all'attrattore frattale con dimensione di Hausdorff log(3)/log(2) ≈ 1.585.
MeshData ShapeGenerator::createSierpinski(
    const glm::vec3& T0,
    const glm::vec3& T1,
    const glm::vec3& T2,
    int nPoints,
    const glm::vec4& color)
{
    MeshData data;

    // Array dei tre vertici attrattori.
    glm::vec3 vertices[3] = { T0, T1, T2 };

    // Inizializzazione del generatore pseudo-casuale Mersenne Twister a 32 bit.
    std::random_device rd;
    std::mt19937 generator(rd());
    // Distribuzione uniforme discreta per estrarre equiprobabilmente gli indici 0, 1 o 2.
    std::uniform_int_distribution<int> distribution(0, 2);

    // Selezione del punto di partenza casuale tra i tre vertici.
    glm::vec3 currentPoint = vertices[distribution(generator)];

    // Generazione stocastica degli N punti del frattale.
    for (int i = 0; i < nPoints; ++i)
    {
        // Estrazione casuale dell'attrattore target.
        int vertexIndex = distribution(generator);
        glm::vec3 selectedVertex = vertices[vertexIndex];

        // Calcolo della combinazione convessa con peso 0.5 (punto medio geometrico).
        currentPoint = 0.5f * (currentPoint + selectedVertex);

        // Memorizzazione della posizione del punto e del colore associato.
        data.positions.push_back(currentPoint);
        data.colors.push_back(color);
    }

    // Modalità di rendering per primitive puntiformi isolate.
    data.primitiveType = GL_POINTS;
    return data;
}
