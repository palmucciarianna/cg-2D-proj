#version 330 core

in vec4 vColor;
out vec4 FragColor;

uniform vec4 uColorTint = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
    FragColor = vColor * uColorTint;
}
