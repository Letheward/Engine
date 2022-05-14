#version 330 core

// todo: these are actually slower (or at least not faster than doing on CPU)
//       find out why

// note: matrices are store in column major order

vec3 v3_rotate(vec3 v, vec4 r) {

    // temp result, a vector and a trivector
    float x   =  v.x * r.x - v.y * r.w + v.z * r.z;
    float y   =  v.y * r.x - v.z * r.y + v.x * r.w;
    float z   =  v.z * r.x - v.x * r.z + v.y * r.y;
    float xyz = -v.x * r.y - v.y * r.z - v.z * r.w;

    // trivector in result will always be 0
    return vec3(
        x * r.x - y * r.w + z * r.z - xyz * r.y,
        y * r.x - z * r.y + x * r.w - xyz * r.z,
        z * r.x - x * r.z + y * r.y - xyz * r.w
    );
}

mat4 r3d_to_m4(vec4 rotor) {
    vec3 v0 = v3_rotate(vec3(1, 0, 0), rotor);
    vec3 v1 = v3_rotate(vec3(0, 1, 0), rotor);
    vec3 v2 = v3_rotate(vec3(0, 0, 1), rotor);
    return mat4(
        v0.x,  v0.y,  v0.z, 0,
        v1.x,  v1.y,  v1.z, 0,
        v2.x,  v2.y,  v2.z, 0,
        0,     0,     0,    1 
    );
}

mat4 m4_translate(vec3 v) {
    return mat4(
        1,   0,   0,   0,
        0,   1,   0,   0,
        0,   0,   1,   0,
        v.x, v.y, v.z, 1
    );
}

mat4 m4_scale(vec3 v) {
    return mat4(
        v.x,   0,   0, 0,
        0,   v.y,   0, 0,
        0,     0, v.z, 0,
        0,     0,   0, 1
    );
}

mat4 entity_to_m4(vec3 pos, vec3 scale, vec4 rotor) {
    return m4_translate(pos) * r3d_to_m4(rotor) * m4_scale(scale);
}