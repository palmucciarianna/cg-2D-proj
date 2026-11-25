#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;

// A differenza di background.vert (già in NDC), gli oggetti del mondo di gioco
// vengono trasformati tramite model (posa locale -> mondo) e projection
// (matrice ortografica glm::ortho in coordinate logiche del mondo -> NDC).
uniform mat4 projection;
uniform mat4 model;

out vec4 vColor;

void main()
{
    // Trasformazione classica 2D: porta il vertice in clip space / NDC
    gl_Position = projection * model * vec4(aPosition, 1.0);
    // Passa il colore al fragment shader (verrà interpolato dal rasterizzatore sui triangoli)
    vColor = aColor;
}
