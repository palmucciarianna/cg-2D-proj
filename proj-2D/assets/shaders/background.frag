#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform float uTime;
uniform float uPsychedelic; // Intensità effetto psichedelico (0.0 = calmo, 1.0 = cosmico/vortice)
uniform int uLevel;          // Livello corrente: alterna le palette cromatiche tra pari e dispari

// Funzione pseudo-casuale deterministica: trasforma una cella 2D in un numero in [0, 1)
// per generare stelle procedurali senza texture
float hash(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// Strato stellare procedurale con scintillio trigonometrico (parallasse)
float starLayer(vec2 uv, float scale, float probability, float twinkleSpeed)
{
    // Suddivide lo schermo in una griglia: gridId identifica la cella,
    // gridUv contiene le coordinate locali centrate nella cella [-0.5, 0.5]
    vec2 gridId = floor(uv * scale);
    vec2 gridUv = fract(uv * scale) - 0.5;

    float h = hash(gridId);
    if (h > probability)
    {
        float d = length(gridUv);
        // Twinkle: crea un effetto di luminosità variabile nel tempo, simulando il brillio delle stelle
        float twinkle = 0.5 + 0.5 * sin(uTime * twinkleSpeed + h * 6.2831);
        // [Domanda 15: Anti-aliasing analitico con transizione morbida di smoothstep]
        // Stella anti-aliased grazie alla transizione morbida di smoothstep
        float star = smoothstep(0.12, 0.0, d) * twinkle;
        return star;
    }
    return 0.0;
}

// [Domanda 1: Architettura SIMD/SIMT della GPU: eseguito in parallelo per ogni pixel (1280x720)]
void main()
{
    vec2 uv = vUV;
    float t = uTime * 0.25;

    // 1. Coordinate Polari centrate rispetto al centro dello schermo (r, theta)
    vec2 p = uv - vec2(0.5);
    float r = length(p);
    float theta = atan(p.y, p.x);

    // Onde polari a spirale/raggi rotanti, amplificate dal fattore psichedelico
    float spiral = sin(theta * 5.0 + r * 14.0 - t * 1.8 * (0.5 + uPsychedelic));
    float polarMask = 0.5 + 0.5 * spiral;

    // 2. Domain Warping trigonometrico sulle coordinate UV
    // Applica distorsioni armoniche sin/cos annidate per simulare fluidodinamica spaziale
    vec2 warpedUV = uv;
    float warpStrength = 0.05 + 0.12 * uPsychedelic;
    warpedUV.x += warpStrength * sin(uv.y * 6.5 + t + spiral * 0.4);
    warpedUV.y += warpStrength * cos(uv.x * 5.5 - t * 0.9 + spiral * 0.4);

    // Campionamento delle densità nebulari con coordinate deformate
    float n1 = sin(warpedUV.x * 3.8 + t * 0.7) * cos(warpedUV.y * 3.2 - t * 0.6);
    float n2 = sin(warpedUV.x * 7.0 - t * 1.1 + n1 * 1.8) * cos(warpedUV.y * 5.8 + t * 0.8);
    float nebulaMask = smoothstep(-0.25, 0.85, n1 * 0.6 + n2 * 0.4);

    // 3. Gradiente di fondo profondo
    vec3 spaceDeep = vec3(0.012, 0.018, 0.05);
    vec3 spaceMid  = vec3(0.05, 0.03, 0.10);
    vec3 baseColor = mix(spaceDeep, spaceMid, uv.y);

    // 4. Alternanza di due Palette Tematiche tra livelli consecutivi (Pari vs Dispari)
    vec3 colA1 = vec3(0.08, 0.45, 0.75); // Ciano elettrico
    vec3 colA2 = vec3(0.55, 0.12, 0.60); // Viola cosmico / magenta
    vec3 colB1 = vec3(0.75, 0.38, 0.08); // Oro solare / ambra
    vec3 colB2 = vec3(0.12, 0.62, 0.48); // Smeraldo aurora boreale

    bool isEven = (uLevel % 2 == 0);
    vec3 cPrimary   = isEven ? colB1 : colA1;
    vec3 cSecondary = isEven ? colB2 : colA2;

    // Miscelazione cromatico-armonica tra le due tinte del tema corrente
    float colorMix = 0.5 + 0.5 * sin(warpedUV.x * 2.5 + warpedUV.y * 1.8 + t);
    vec3 nebulaColor = mix(cPrimary, cSecondary, colorMix);

    // Fusione della nebulosa con i raggi polari psichedelici
    float finalNebulaFactor = nebulaMask * (0.35 + 0.35 * uPsychedelic) + polarMask * 0.15 * uPsychedelic;
    vec3 finalColor = baseColor + nebulaColor * finalNebulaFactor;

    // 5. Campi stellari procedurali a 3 strati di parallasse
    float stars = 0.0;
    stars += starLayer(uv, 35.0, 0.92, 2.5);  // Grandi vicine
    stars += starLayer(uv, 75.0, 0.95, 4.0);  // Medie
    stars += starLayer(uv, 130.0, 0.97, 6.0); // Piccole lontane
    finalColor += vec3(stars);

    // 6. Vignettatura radiale ai bordi dello schermo
    float vignette = smoothstep(1.25, 0.45, r * 1.4);
    finalColor *= vignette;

    FragColor = vec4(finalColor, 1.0);
}
