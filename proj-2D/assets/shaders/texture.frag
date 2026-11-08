#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uColorTint = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
    // Ritaglio ellittico morbido per adattarsi alla forma della cabina di pilotaggio
    vec2 uvNorm = (vTexCoord - vec2(0.5, 0.5)) * 2.0;
    float dist = dot(uvNorm, uvNorm);
    if (dist > 1.0) discard; // maschera ovale perfetta

    vec4 texColor = texture(uTexture, vTexCoord);
    float edgeAlpha = smoothstep(1.0, 0.82, dist);
    FragColor = vec4(texColor.rgb, texColor.a * edgeAlpha) * uColorTint;
}
