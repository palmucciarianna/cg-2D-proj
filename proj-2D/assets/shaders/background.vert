#version 330 core
// Vertex shader passthrough in NDC (Normalized Device Coordinates):
// non si moltiplica per projection * model perché i vertici del quad fullscreen
// sono già definiti direttamente in NDC [-1, 1] (indipendenti dalla risoluzione
// e dalla camera del mondo di gioco).
layout(location = 0) in vec2 aPos;
out vec2 vUV;
void main()
{
    // Rimappa le coordinate da [-1, 1] a [0, 1] per avere UV pulite nel fragment shader
    vUV = aPos * 0.5 + 0.5;
    // Piazza il vertice direttamente in clip space / NDC
    gl_Position = vec4(aPos, 0.0, 1.0);
}