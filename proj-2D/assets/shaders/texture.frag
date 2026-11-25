#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uColorTint = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
    // Centra e normalizza le coordinate UV da [0, 1] a [-1, 1]
    vec2 uvNorm = (vTexCoord - vec2(0.5, 0.5)) * 2.0;
    // Calcola la distanza al quadrato dal centro
    float dist = dot(uvNorm, uvNorm);
    // Maschera ellittica: scarta i pixel fuori dal cerchio unitario UV (ellisse sul quad dell'oblò)
    if (dist > 1.0) discard;

    // Campionamento della texture + bordo morbido con anti-aliasing (smoothstep)
    vec4 texColor = texture(uTexture, vTexCoord);
    float edgeAlpha = smoothstep(1.0, 0.82, dist);
    FragColor = vec4(texColor.rgb, texColor.a * edgeAlpha) * uColorTint;
}