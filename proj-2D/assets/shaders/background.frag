#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform float uTime;

// Funzione di hash pseudo-casuale per generare stelle senza texture
float hash(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// Genera uno strato di stelle con scintillio
float starLayer(vec2 uv, float scale, float probability, float twinkleSpeed)
{
    vec2 gridId = floor(uv * scale);
    vec2 gridUv = fract(uv * scale) - 0.5;

    float h = hash(gridId);
    if (h > probability)
    {
        float d = length(gridUv);
        float twinkle = 0.5 + 0.5 * sin(uTime * twinkleSpeed + h * 6.2831);
        float star = smoothstep(0.12, 0.0, d) * twinkle;
        return star;
    }
    return 0.0;
}

void main()
{
    // Coordinate normalizzate centrate
    vec2 uv = vUV;

    // 1. Gradiente di fondo: dallo spazio profondo (blu notte quasi nero) a un violetto tenue
    vec3 spaceDeep = vec3(0.015, 0.02, 0.06);
    vec3 spaceMid  = vec3(0.06, 0.04, 0.12);
    vec3 baseColor = mix(spaceDeep, spaceMid, uv.y);

    // 2. Nebulosa cosmica procedurale (onde armoniche sin/cos nel tempo)
    float t = uTime * 0.15;
    float n1 = sin(uv.x * 3.5 + t) * cos(uv.y * 3.0 - t * 0.8);
    float n2 = sin(uv.x * 6.0 - t * 1.2 + n1 * 1.5) * cos(uv.y * 5.0 + t);
    float nebulaMask = smoothstep(-0.2, 0.8, n1 * 0.6 + n2 * 0.4);

    vec3 nebulaCyan    = vec3(0.08, 0.35, 0.55);
    vec3 nebulaMagenta = vec3(0.45, 0.10, 0.45);
    vec3 nebulaColor   = mix(nebulaCyan, nebulaMagenta, 0.5 + 0.5 * sin(uv.x * 2.0 + t));

    vec3 finalColor = baseColor + nebulaColor * nebulaMask * 0.45;

    // 3. Campi stellari su 3 livelli di parallasse e densità
    float stars = 0.0;
    stars += starLayer(uv, 35.0, 0.92, 2.5); // Stelle grandi
    stars += starLayer(uv, 70.0, 0.95, 4.0); // Stelle medie
    stars += starLayer(uv, 120.0, 0.97, 6.0); // Stelle lontane

    finalColor += vec3(stars);

    // 4. Lieve bagliore ai bordi (vignettatura spaziale)
    float vignette = smoothstep(1.2, 0.4, length(uv - 0.5) * 1.4);
    finalColor *= vignette;

    FragColor = vec4(finalColor, 1.0);
}
