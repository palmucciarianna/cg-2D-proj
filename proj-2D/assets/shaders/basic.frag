#version 330 core

in vec4 vColor;
out vec4 FragColor;

// Tinta dinamica globale: permette flash rossi ai danni, trasparenza (fade-out)
// e lampeggi di invulnerabilità senza dover ricreare la geometria
uniform vec4 uColorTint = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
    // [Domanda 18: Fragment Shader - colore interpolato (varying) * uniform uColorTint]
    // Moltiplica il colore interpolato dal rasterizzatore per la tinta dinamica globale
    FragColor = vColor * uColorTint;
}
