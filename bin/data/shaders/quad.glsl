[vert]
#version 330 core

layout(location = 0) in vec2 in_pos;

void main() {
    gl_Position = vec4(in_pos.xy, 0, 0);
}

[frag]
#version 330 core

layout(location = 0) out vec4 out_color;

void main() {
    
    //out_color = vec4(1, 1, 1, texture(texture0, uv).r);
    out_color = vec4(1, 1, 1, 1);
}

