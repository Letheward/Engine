[vert]
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 in_color;

out vec3 color;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

vec4 Position;

void main() {
    Position = projection * view * model * vec4(pos, 1.0);
    gl_Position = Position.xzyw;
    color = in_color;
}

[frag]
#version 330 core

in  vec3 color;
out vec4 out_color;

void main() {
    out_color = vec4(color, 1.0); 
}
