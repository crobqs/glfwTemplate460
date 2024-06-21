#version 460 core

in vec3 Color;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D Tex1;
uniform sampler2D Tex2;
uniform float MixRatio;

void main() {
    FragColor = mix(texture(Tex1, TexCoord), texture(Tex2, TexCoord), MixRatio);
}