#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;

// Come in basic.vert (e a differenza di background.vert che è già in NDC),
// trasforma l'oggetto 2D dal suo spazio locale allo spazio logico del mondo (model)
// e poi in NDC tramite la matrice ortografica (projection).
uniform mat4 projection;
uniform mat4 model;

out vec2 vTexCoord;

void main()
{
    // Trasformazione classica 2D: projection * model * posizione
    gl_Position = projection * model * vec4(aPosition, 1.0);
    // Passa le coordinate UV al fragment shader per campionare la texture sull'oggetto 2D
    vTexCoord = aTexCoord;
}
