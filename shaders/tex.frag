#version 460 core

in vec3 Color;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D Tex;

void main() {
    FragColor = texture(Tex, TexCoord) * vec4(Color, 1.0f);
}