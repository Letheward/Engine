[vert]
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 in_tex_coord;
layout(location = 2) in vec3 in_color;

out vec2 tex_coord;
out vec3 color;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

vec4 Position;

void main() {

    Position = projection * view * model * vec4(pos, 1.0);
    gl_Position = Position.xzyw;
    gl_PointSize = 50 / Position.y;
    tex_coord = in_tex_coord;
    color = in_color;
}

[frag]
#version 330 core

in vec3 color;
in vec2 tex_coord;

layout(location = 0) out vec4 out_color;

uniform sampler2D texture0;

vec3 ambient = 0.6 * vec3(0.8, 0.9, 0.6);
vec3 diffuse = vec3(0.1, 0.1, 0.1);

void main() {
    out_color = /* vec4((ambient + diffuse), 1.0) *  */texture(texture0, tex_coord);
}
