[vert]
#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

out vec3 pos;
out vec2 uv;
out vec3 normal;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

vec4 Position;

void main() {

    pos    = vec3(model * vec4(in_pos, 1));
    uv     = in_uv;
    normal = in_normal;

    Position    = projection * view * model * vec4(in_pos, 1.0);
    gl_Position = Position.xzyw;

    // gl_PointSize = 50 / Position.y;
}

[frag]
#version 330 core

in vec3 pos;
in vec2 uv;
in vec3 normal;

layout(location = 0) out vec4 out_color;

uniform sampler2D texture0;
uniform vec3      light_pos;

void main() {

    vec3  ambient   = 0.2 * vec3(1, 1, 1);
    vec3  diffuse   = vec3(0.7, 0.6, 0.3); 
    
    vec3  diff      = light_pos - pos;
    vec3  direction = normalize(diff);
    float att       = sqrt(1 / length(diff));
    float scale     = max(dot(normal, direction), 0) * att * 10;

    out_color = texture(texture0, uv) * vec4((ambient + diffuse * scale), 1.0);
}

