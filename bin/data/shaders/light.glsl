[vert]
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 0) in vec3 color;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

vec4 Position;

void main() {
    Position = projection * view * model * vec4(pos, 1.0);
    gl_Position = Position.xzyw;
}

[frag]
#version 330 core
out vec4 color;

void main() {
    color = vec4(1.0); 
}
