[vert]
#version 330 core

layout(location = 0) in vec2 pos;

uniform mat2 transform;
uniform vec2 position;

void main() {
    vec2 out_pos = transform * pos + position;
    gl_Position = vec4(out_pos, 0, 1.0);
}

[frag]
#version 330 core

layout(location = 0) out vec4 out_color;
uniform vec4 color;

void main() {
    out_color = color;
}
