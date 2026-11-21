#version 330 core
in vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D uSceneTexture;
uniform float uTime;

// Parametri Shockwave (Onda d'Urto Ottica)
uniform vec2 uShockwaveCenter;   // In coordinate UV normalizzate [0, 1]
uniform float uShockwaveTime;    // Secondi trascorsi dall'onda
uniform float uShockwaveMaxTime; // Durata complessiva dell'effetto (es. 0.65s)
uniform float uShockwaveStrength;// Ampiezza della distorsione rifrattiva

// Effetto CRT & Retro Arcade
uniform bool uEnableCRT;
uniform float uScanlineIntensity;   // Intensità delle righe di scansione [0, 1]
uniform float uChromaticAberration; // Sdoppiamento cromatico RGB sui bordi [0, 0.02]

// Danno e Vignettatura
uniform float uDamageFlash;       // Intensità del bagliore cremisi sui bordi [0, 1]
uniform float uVignetteIntensity; // Intensità vignettatura cinematografica [0, 1]

void main()
{
    vec2 uv = vTexCoords;

    // 1. Shockwave Distortion (Onda d'urto ad anello rifrattiva)
    if (uShockwaveTime > 0.0 && uShockwaveTime < uShockwaveMaxTime)
    {
        vec2 diff = uv - uShockwaveCenter;
        diff.y *= (720.0 / 1280.0); // Correzione aspect ratio per garantire circolarità
        float dist = length(diff);

        float waveRadius = (uShockwaveTime / uShockwaveMaxTime) * 0.95;
        float waveThickness = 0.09;

        if (dist <= waveRadius + waveThickness && dist >= waveRadius - waveThickness)
        {
            float waveProgress = 1.0 - (uShockwaveTime / uShockwaveMaxTime);
            float diffDist = dist - waveRadius;
            float offset = sin(diffDist * 3.14159 / waveThickness) * uShockwaveStrength * waveProgress;
            vec2 dir = (dist > 0.0001) ? normalize(uv - uShockwaveCenter) : vec2(0.0);
            uv += dir * offset;
        }
    }

    // 2. Aberrazione Cromatica (Sdoppiamento canali R e B ai bordi dello schermo)
    vec4 sceneColor;
    if (uChromaticAberration > 0.0001)
    {
        vec2 distFromCenter = uv - vec2(0.5);
        vec2 rOffset = distFromCenter * uChromaticAberration * 1.6;
        vec2 bOffset = distFromCenter * -uChromaticAberration * 1.6;

        float r = texture(uSceneTexture, uv + rOffset).r;
        float g = texture(uSceneTexture, uv).g;
        float b = texture(uSceneTexture, uv + bOffset).b;
        float a = texture(uSceneTexture, uv).a;
        sceneColor = vec4(r, g, b, a);
    }
    else
    {
        sceneColor = texture(uSceneTexture, uv);
    }

    // 3. Effetto CRT Scanlines (Simulazione monitor a tubo catodico da sala giochi)
    if (uEnableCRT)
    {
        float scanline = sin(uv.y * 720.0 * 3.14159);
        scanline = (scanline + 1.0) * 0.5;
        sceneColor.rgb -= scanline * uScanlineIntensity * 0.16;
    }

    // 4. Vignettatura Cosmica (Graduale oscuramento radiale degli angoli)
    float vignette = length(uv - vec2(0.5)) * 1.4142;
    vignette = clamp(vignette, 0.0, 1.0);
    float vignetteFactor = 1.0 - pow(vignette, 2.5) * uVignetteIntensity;
    sceneColor.rgb *= clamp(vignetteFactor, 0.0, 1.0);

    // 5. Flash Visivo di Danno (Allarme visivo cremisi in caso di impatto)
    if (uDamageFlash > 0.001)
    {
        float edgeVignette = smoothstep(0.3, 0.85, length(uv - vec2(0.5)));
        vec3 redDamage = vec3(1.0, 0.08, 0.12);
        sceneColor.rgb = mix(sceneColor.rgb, redDamage, edgeVignette * uDamageFlash * 0.7);
    }

    FragColor = sceneColor;
}
