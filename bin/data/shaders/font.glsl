[vert]
#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 in_uv;

out vec2 uv;

uniform mat2 transform;
uniform vec2 offset;
uniform vec2 position;

void main() {

    vec2 temp = in_uv;
   
    temp.x /= 16;
    temp.y /= 6; 
    temp.x += offset.x;
    temp.y += offset.y;
    
    uv = temp;
    vec2 out_pos = transform * pos + position;
    gl_Position = vec4(out_pos, 0, 1.0);
}

[frag]
#version 330 core

uniform sampler2D texture0;
uniform vec4 color;

in vec2 uv;
out vec4 out_color;

void main() {
    out_color = vec4(color.rgb, texture(texture0, uv).a * color.a);
}
