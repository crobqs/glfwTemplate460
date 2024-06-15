#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

void main() {
    gl_Position = vec4(aPos, 0.0f, 1.0f);
    Color = aColor;
}