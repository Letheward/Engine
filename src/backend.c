/* ==== Global Info ==== */

typedef struct {

    GLFWwindow* handle;
    char* title;

    int width;
    int height;
    f32 aspect;

    // prev position for toggle fullscreen
    struct {
        int x;
        int y;
        int w;
        int h;
    } pos;

    struct {
        f64 x;
        f64 y;
    } cursor;

    // options and states
    u8 vsync;
    u8 cursor_visible;
    u8 fullscreen;
    u8 is_first_frame;
    u8 show_debug_info;

} WindowInfo;




/* ==== Time ==== */

typedef struct {
    f64 base;
    f64 counter;
    f64 interval;
} Timer;




/* ==== Resource ==== */

typedef struct {

    u8* data;
    int w;
    int h;
    int channel;

    u32 id;

} Texture;


// global name binding

typedef struct {
   
    u32 cube;
    u32 rect;
    u32 axis;
    
    u32 font;
    u32 light;

} Asset_Shaders;

typedef struct {
    Texture test;
    Texture sun;
    Texture stairway;
    Texture styxel;
    Texture styxel_8x8;
    Texture sb_16x16;
} Asset_Textures;




/* ==== Entity ==== */

typedef struct {
    f32*    vertex_data;
    u32*    indices;
    u32     vertex_data_count;
    u32     index_count;
    Texture texture; // optional
    struct {
        u32 shader;
        u32 vertex_array;
        u32 vertices;
        u32 indices; 
        u32 texture; // todo: redundant now, as a Texture has it
    } id;
} Mesh;

typedef struct {
    Vector3 position;
    Vector3 scale;
    Rotor3D orientation;
} Entity3D;

typedef struct {
    Entity3D base;
    Mesh*    mesh;
} Model3D;

// Experiment: Gravity
typedef struct {
    Entity3D base;
    Vector3  velocity;
    f32      mass;
    Mesh*    mesh;
} CelestialBody;


typedef struct {
    
    Vector3 position;
    Rotor3D orientation;
    
    // if camera is FPS style, these store the rotation in radians, todo: add a flag to indicate different style?
    f32     yz;
    f32     zx;
    f32     xy;
    
    int     draw_mode;   // temp, maybe remove this
    
    // FOV
    int     FOV;         // we may want f32, this is just for convenience
    f32     near;
    f32     far;

    Matrix4 view;        // output of position & rotation
    Matrix4 projection;  // output of FOV, near, far

} Camera;

// global name binding
typedef struct {
    Mesh axis_arrow;
    Mesh ring;
    Mesh circle;
    Mesh rectangle;
    Mesh font_rectangle;
    Mesh cube;
    Mesh sphere;
    Mesh tetrahedron;
} GeometryPrimitives;






/* ==== Global Data ==== */

// stupid global variables to force laptop to use dedicated GPU
// on linux these don't work and will break compilation
__declspec(dllexport) int NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

GeometryPrimitives geometry_primitives;
Asset_Shaders      asset_shaders;
Asset_Textures     asset_textures;

f32 engine_speed_scale   = 1.0;
f32 movement_speed_scale = 1.0;

WindowInfo window_info = {
    .width           = 800,
    .height          = 600,
    .vsync           = 1,
    .show_debug_info = 0,
};

Camera camera = {
    
    .position = {0, -10, 0},
    .orientation = R3D_DEFAULT,
    
    .yz = 0,
    .zx = 0,
    .xy = 0,

    .FOV = 70,
    .near = 0.100000,
    .far = 10000.000000,
    .draw_mode = 4,
};





/* ==== Utilities ==== */

void clamp_s32(s32* x, s32 low, s32 high) {
    if (*x < low ) *x = low;
    if (*x > high) *x = high;
}

void clamp_f32(f32* x, f32 low, f32 high) {
    if (*x < low ) *x = low;
    if (*x > high) *x = high;
}





/* ==== Waves ==== */

// todo: maybe slower than state machine?
f32 triangle_wave(f32 in) {
    const f32 f = 4 / TAU;
    return f * asinf(sinf(in));
}

f32 sin_normalize(f32 in) {
    return (1 + sinf(in)) / 2;
}



/* ==== GL: Debug ==== */

// legacy debug functions, use print_OpenGL_debug_message() to get better result
void GL_clear_errors() {
    while (glGetError() != GL_NO_ERROR);
}

// legacy debug functions, use print_OpenGL_debug_message() to get better result
void GL_check_errors() {
    switch (glGetError()) {
        case GL_NO_ERROR:                                                                                   break;
        case GL_INVALID_ENUM:                  logprint("[OpenGL] [Error] Invalid enum\n");                   break;
        case GL_INVALID_VALUE:                 logprint("[OpenGL] [Error] Invalid value\n");                  break;
        case GL_INVALID_OPERATION:             logprint("[OpenGL] [Error] Invalid operation\n");              break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: logprint("[OpenGL] [Error] Invalid frame buffer operation\n"); break;
        case GL_OUT_OF_MEMORY:                 logprint("[OpenGL] [Error] Out of memory\n");                  break;
        case GL_STACK_UNDERFLOW:               logprint("[OpenGL] [Error] Stack underflow\n");                break;
        case GL_STACK_OVERFLOW:                logprint("[OpenGL] [Error] Stack overflow\n");                 break;
        default: break;
    }
}

void GL_print_debug_message(
    GLenum        source,
    GLenum        type,
    GLuint        id,
    GLenum        severity,
    GLsizei       length,
    const GLchar* message,
    const void*   user_param
) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION: logprint("[OpenGL] [Note] %s\n"    , message); break;
        case GL_DEBUG_SEVERITY_LOW:          logprint("[OpenGL] [Error: L] %s\n", message); exit(1); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       logprint("[OpenGL] [Error: M] %s\n", message); exit(1); break;
        case GL_DEBUG_SEVERITY_HIGH:         logprint("[OpenGL] [Error: H] %s\n", message); exit(1); break;
    }
}




/* ==== Renderer: Utilities ==== */

void update_FPS_timer(Timer* t, f64 dt) {
    t->base    += dt;
    t->counter += t->interval;
    if (t->base >= t->interval) {
        t->base    = 0;
        t->counter = 0;
    }
}

void update_camera_projection(Camera* cam) {
    cam->projection = m4_perspective(cam->FOV * TAU / 360, window_info.aspect, cam->near, cam->far);
}

void resize_framebuffer(GLFWwindow* window, int width, int height) {
    WindowInfo* w = &window_info;
    glViewport(0, 0, width, height);
    w->width  = width;
    w->height = height;
    w->aspect = (f32) width / (f32) height;
    update_camera_projection(&camera);
}

void toggle_vsync() {
    WindowInfo* w = &window_info;
    w->vsync = !w->vsync;
    glfwSwapInterval(w->vsync);
}

void toggle_cursor() {
    WindowInfo* w = &window_info;
    w->cursor_visible = !w->cursor_visible;
    glfwSetInputMode(w->handle, GLFW_CURSOR, w->cursor_visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void toggle_debug_info() {
    WindowInfo* w = &window_info;
    w->show_debug_info = !w->show_debug_info;
}


// borderless fullscreen for now
void toggle_fullscreen() {

    // hardcoded
    WindowInfo* w = &window_info;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor(); // hardcoded for now, we may want the window fullscreen on another monitor that user drag the window to
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if (w->fullscreen == 0) {
        glfwGetWindowPos(w->handle, &w->pos.x, &w->pos.y);
        glfwGetWindowSize(w->handle, &w->pos.w, &w->pos.h);
        glfwSetWindowMonitor(w->handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        glfwSwapInterval(w->vsync); // for some reason, toggle fullscreen resets swap interval
        w->fullscreen = 1;
    } else {
        glfwSetWindowMonitor(
            w->handle,
            NULL,
            w->pos.x,
            w->pos.y,
            w->pos.w,
            w->pos.h,
            mode->refreshRate
        );
        w->fullscreen = 0;
    }
}

void change_draw_mode(int d) {
    Camera* c = &camera;
    c->draw_mode += d;
    clamp_s32(&c->draw_mode, GL_POINTS, GL_TRIANGLE_FAN);
    switch (c->draw_mode) {
        case GL_POINTS:         logprint("[OpenGL] Draw Mode: GL_POINTS\n"        ); break;
        case GL_LINES:          logprint("[OpenGL] Draw Mode: GL_LINES\n"         ); break;
        case GL_LINE_LOOP:      logprint("[OpenGL] Draw Mode: GL_LINE_LOOP\n"     ); break;
        case GL_LINE_STRIP:     logprint("[OpenGL] Draw Mode: GL_LINE_STRIP\n"    ); break;
        case GL_TRIANGLES:      logprint("[OpenGL] Draw Mode: GL_TRIANGLES\n"     ); break;
        case GL_TRIANGLE_STRIP: logprint("[OpenGL] Draw Mode: GL_TRIANGLE_STRIP\n"); break;
        case GL_TRIANGLE_FAN:   logprint("[OpenGL] Draw Mode: GL_TRIANGLE_FAN\n"  ); break;
    }
}

void change_engine_speed(f32 scale) {
    engine_speed_scale *= scale;
    logprint("[Engine] Speed Scale: %f\n", engine_speed_scale);
}

void change_movement_speed(f32 scale) {
    movement_speed_scale *= scale;
    logprint("[Engine] Movement Speed Scale: %f\n", movement_speed_scale);
}






/* ==== Renderer ==== */

Matrix4 entity_to_m4(Entity3D e) {
    return m4_mul(
        m4_translate(e.position),
        m4_mul(r3d_to_m4(e.orientation), m4_scale(e.scale))
    );
}

void draw_axis_arrow(Vector3 scale, Camera* cam) {

    Mesh* mesh = &geometry_primitives.axis_arrow;

    Vector3 position = v3_add(cam->position, v3_rotate(V3_Y, cam->orientation));

    glUseProgram(mesh->id.shader); 
    glBindVertexArray(mesh->id.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
 
    glUseProgram(mesh->id.shader); 
    glBindVertexArray(mesh->id.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
    
    Matrix4 m = m4_mul(m4_translate(position), m4_scale(scale));
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "projection"), 1, GL_FALSE, (f32*) &cam->projection);
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "view"), 1, GL_FALSE, (f32*) &cam->view);
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "model"), 1, GL_FALSE, (f32*) &m);
    
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2);
   
    glDrawElements(GL_LINES, mesh->index_count, GL_UNSIGNED_INT, NULL);
    
    glEnable(GL_DEPTH_TEST);
    glLineWidth(1);
}

// todo: how should we do offset?
void draw_ring(Vector2 scale, f32 line_width, Vector4 color) {
    
    Mesh* mesh = &geometry_primitives.ring;

    Matrix2 m = m2_scale((Vector2) {1 / window_info.aspect, 1});
    m = m2_mul(m2_scale(scale), m);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(mesh->id.shader); 
    glBindVertexArray(mesh->id.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
 
    glUniform4fv(glGetUniformLocation(mesh->id.shader, "color"), 1, (f32*) &color);

    glUniformMatrix2fv(glGetUniformLocation(mesh->id.shader, "transform"), 1, GL_FALSE, (f32*) &m);
    
    glLineWidth(line_width);
    glDrawElements(GL_LINES, mesh->index_count, GL_UNSIGNED_INT, NULL);
    glLineWidth(1);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void draw_circle(Vector2 scale, Vector4 color) {
    
    Mesh* mesh = &geometry_primitives.circle;

    Matrix2 m = m2_scale((Vector2) {1 / window_info.aspect, 1});
    m = m2_mul(m2_scale(scale), m);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(mesh->id.shader); 
    glBindVertexArray(mesh->id.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
 
    glUniform4fv(glGetUniformLocation(mesh->id.shader, "color"), 1, (f32*) &color);
    glUniformMatrix2fv(glGetUniformLocation(mesh->id.shader, "transform"), 1, GL_FALSE, (f32*) &m);
    
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, NULL);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}



// todo: what's the better way to do offset?
void draw_rect(Matrix2* m, int count, Vector4 color) {
    
    Mesh* mesh = &geometry_primitives.rectangle;

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);

    glUseProgram(mesh->id.shader); 
    glBindVertexArray(mesh->id.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
 
    glUniform4fv(glGetUniformLocation(mesh->id.shader, "color"), 1, (f32*) &color);

    for (int i = 0; i < count; i++) {
        glUniformMatrix2fv(glGetUniformLocation(mesh->id.shader, "transform"), 1, GL_FALSE, (f32*) &m[i]);
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, NULL);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

// todo: what's the better way to do offset?
// todo: texture leak bug?
void draw_string(Vector2 position, Vector2 scale, Vector4 color, String s) {
    
    Mesh* mesh = &geometry_primitives.font_rectangle;
    
    Matrix2 m = m2_scale((Vector2) {1 / window_info.aspect, 1});
    m = m2_mul(m2_scale(scale), m);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 
    glUseProgram(mesh->id.shader); 
    glBindVertexArray(mesh->id.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
 
    glBindTexture(GL_TEXTURE_2D, asset_textures.sb_16x16.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0);
 
    glUniform1i(glGetUniformLocation(mesh->id.shader, "texture0"), 0);
    glUniformMatrix2fv(glGetUniformLocation(mesh->id.shader, "transform"), 1, GL_FALSE, (f32*) &m);
    glUniform4fv(glGetUniformLocation(mesh->id.shader, "color"), 1, (f32*) &color);
   
    for (int i = 0; i < s.count; i++) {
    
        u8 c = s.data[i];

        int o = 16 * 1;
        int row = (c - o) % 16;
        int col = 6 - (c - o) / 16;
        
        Vector2 offset = {row / 16.0, col / 6.0};
        Vector2 pos_offset = {position.x + scale.x * i / window_info.aspect, position.y};

        glUniform2fv(glGetUniformLocation(mesh->id.shader, "position"), 1, (f32*) &pos_offset);
        glUniform2fv(glGetUniformLocation(mesh->id.shader, "offset"), 1, (f32*) &offset);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

// todo: figure out offset scaling
void draw_string_shadowed(Vector2 position, Vector2 offset, Vector2 scale, Vector4 fg_color, Vector4 bg_color, String s) {
    draw_string(v2_add(position, offset), scale, bg_color, s);
    draw_string(position,                 scale, fg_color, s);
}




// todo: how to handle other shaders?
void draw_model(Model3D* model, int count, Camera* cam) {
    
    Mesh* mesh = model->mesh; 

    glUseProgram(mesh->id.shader); 
    glBindVertexArray(mesh->id.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
    
    glBindTexture(GL_TEXTURE_2D, mesh->id.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(mesh->id.shader, "texture0"), 0);
    
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "projection"), 1, GL_FALSE, (f32*) &cam->projection);
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "view"), 1, GL_FALSE, (f32*) &cam->view);
    
    for (int i = 0; i < count; i++) {
        Matrix4 m = entity_to_m4(model[i].base);
        glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "model"), 1, GL_FALSE, (f32*) &m);
        glDrawElements(cam->draw_mode, mesh->index_count, GL_UNSIGNED_INT, NULL);
    }
}


// todo: how to handle other shaders?
void draw_body(CelestialBody* model, int count, Camera* cam) {
    
    Mesh* mesh = model->mesh; 

    glUseProgram(mesh->id.shader); 
    glBindVertexArray(mesh->id.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
    
    glBindTexture(GL_TEXTURE_2D, mesh->id.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(mesh->id.shader, "texture0"), 0);
    
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "projection"), 1, GL_FALSE, (f32*) &cam->projection);
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "view"), 1, GL_FALSE, (f32*) &cam->view);
    
    for (int i = 0; i < count; i++) {
        Matrix4 m = entity_to_m4(model[i].base);
        glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "model"), 1, GL_FALSE, (f32*) &m);
        glDrawElements(cam->draw_mode, mesh->index_count, GL_UNSIGNED_INT, NULL);
    }
}







/* ==== Resource Loading ==== */

#define make_mesh_from_stack_data(mesh, v, i, va, Vertex_Type, shader, texture) make_mesh(mesh, (f32*) v, i, va, length_of(v), length_of(i), length_of(va), sizeof(Vertex_Type), shader, texture, 1)
void make_mesh(
    Mesh* mesh, 
    f32*  vertices, 
    u32*  indices, 
    u32*  vertex_structure,
    u32   vertex_count, 
    u32   index_count,
    u32   vertex_structure_count,
    u64   vertex_size, 
    u32   shader,
    u32   texture,
    u8    is_stack_data
) {
    
    if (is_stack_data) {
        mesh->vertex_data = malloc(vertex_size * vertex_count);
        mesh->indices     = malloc(sizeof(u32) * index_count);
    } else {
        mesh->vertex_data = vertices;
        mesh->indices     = indices;
    }
   
    mesh->vertex_data_count = vertex_count * vertex_size / sizeof(f32);
    mesh->index_count       = index_count;  
    
    if (is_stack_data) {
        memcpy(mesh->vertex_data, vertices, vertex_size * vertex_count); 
        memcpy(mesh->indices,     indices,  sizeof(u32) * index_count); 
    }
    
    mesh->id.shader  = shader;
    mesh->id.texture = texture;

    glGenVertexArrays(1, &mesh->id.vertex_array);
    glGenBuffers(     1, &mesh->id.vertices);
    glGenBuffers(     1, &mesh->id.indices);

    glUseProgram(mesh->id.shader);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertex_data_count * sizeof(f32), mesh->vertex_data, GL_DYNAMIC_DRAW);

    glBindVertexArray(mesh->id.vertex_array);
    
    u32 step = 0;
    for (u32 i = 0; i < vertex_structure_count; i++) {
        glVertexAttribPointer(i, vertex_structure[i], GL_FLOAT, GL_FALSE, vertex_size, (void*) (step * sizeof(f32)));
        glEnableVertexAttribArray(i);
        step += vertex_structure[i];
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count * sizeof(u32), mesh->indices, GL_DYNAMIC_DRAW);
}

void make_geometry_primitives() {


    /* ---- Geometry Primitives ---- */
    /* 
        rect indices

                       y
        3----2         |
        |    |     --------x
        0----1         |

        cube indices

              7---------6
             /         /|            z  y
            4---------5 |            | /
            |         | |            |/
            | 3       | 2      ------------- x
            |         |/            /|
            0---------1            / |
    */
    
    // Axis arrow
    {
        typedef struct {
            Vector3 pos;
            Vector3 color;
        } Vertex;
        
        u32 va[] = {3, 3};

        Vertex v[] = {
            {{0, 0, 0}, {1, 1, 1}},
            {{1, 0, 0}, {1, 0, 0}},
            {{0, 1, 0}, {0, 1, 0}},
            {{0, 0, 1}, {0, 0, 1}},
        };

        u32 i[] = {
            0, 1,
            0, 2,
            0, 3,
        };

        make_mesh_from_stack_data(&geometry_primitives.axis_arrow, v, i, va, Vertex, asset_shaders.axis, 0);
    }
    
    // Ring
    {
        typedef struct {
            Vector2 pos;
        } Vertex;
        
        u32 vertex_structure[] = {2};

        const u32 edges   = 36;
        
        u32 vertex_count = edges;
        u32 index_count  = edges * 2;
        Vertex* vertices = malloc(sizeof(Vertex) * vertex_count); 
        u32*    indices  = malloc(sizeof(u32)    * index_count); 

        for (int i = 0; i < vertex_count; i++) {
            f32 rad  = TAU * i / (f32) edges;
            vertices[i].pos = (Vector2) {cosf(rad), sinf(rad)};
        }

        for (int i = 0; i < edges - 1; i++) {
            indices[i * 2 + 0] = i;
            indices[i * 2 + 1] = i + 1;
        }
        indices[70] = 35;
        indices[71] = 0;

        make_mesh(
            &geometry_primitives.ring,
            (f32*) vertices, indices,     vertex_structure,
            vertex_count,    index_count, length_of(vertex_structure), 
            sizeof(Vertex), asset_shaders.rect, 0, 0
        );
    }

    // Circle 2D
    {
        typedef struct {
            Vector2 pos;
        } Vertex;
        
        u32 vertex_structure[] = {2};
        
        const u32 edges = 36;

        u32 vertex_count = edges + 1;
        u32 index_count  = edges * 3;

        Vertex* vertices = malloc(sizeof(Vertex) * vertex_count); 
        u32*    indices  = malloc(sizeof(u32)    * index_count);

        for (int i = 1; i < vertex_count; i++) {
            f32 rad = TAU * i / (f32) edges;
            vertices[i].pos = (Vector2) {cosf(rad), sinf(rad)};
        }
        
        for (int j = 0; j < edges; j++) {
            indices[j * 3 + 0] = 0;
            indices[j * 3 + 1] = j + 1;
            indices[j * 3 + 2] = j + 2;
        }
        
        vertices[0].pos = (Vector2) {0, 0};
        indices[edges * 3 - 1] = 1;

        make_mesh(
            &geometry_primitives.circle,
            (f32*) vertices, indices,     vertex_structure,
            vertex_count,    index_count, length_of(vertex_structure), 
            sizeof(Vertex), asset_shaders.rect, 0, 0
        );
    }

    // Rectangle
    {
        typedef struct {
            Vector2 pos;
        } Vertex;

        u32 va[] = {2};

        const f32 s = 0.5; // will get a 1 * 1 rect, center at 0, 0, 0
        
        Vertex v[] = {
            {{-s, -s}},
            {{ s, -s}},
            {{ s,  s}},
            {{-s,  s}},
        };

        u32 i[] = {
            0, 1, 2,
            0, 2, 3
        };

        make_mesh_from_stack_data(&geometry_primitives.rectangle, v, i, va, Vertex, asset_shaders.rect, 0);
    }
    
    // Mono Font
    {
        typedef struct {
            Vector2 pos;
            Vector2 uv;
        } Vertex;

        u32 va[] = {2, 2};
        
        const f32 s = 0.5; // will get a 1 * 1 rect, center at 0, 0, 0

        Vertex v[] = {
            {{-s, -s}, {0, 0}},
            {{ s, -s}, {1, 0}},
            {{ s,  s}, {1, 1}},
            {{-s,  s}, {0, 1}},
        };

        u32 i[] = {
            0, 1, 2,
            0, 2, 3
        };

        make_mesh_from_stack_data(&geometry_primitives.font_rectangle, v, i, va, Vertex, asset_shaders.font, 0);
    }
   
    // Cube 
    {
        typedef struct {
            Vector3 pos;
            Vector2 uv;
            Vector3 color;
        } Vertex;

        u32 va[] = {3, 2, 3};
        
        const f32 s = 0.5; // will get a 1 * 1 * 1 cube, center at 0, 0, 0
        
        Vertex v[] = {
            {{-s, -s, -s}, {0, 0}, {1, 0, 0}}, 
            {{ s, -s, -s}, {1, 0}, {0, 1, 0}},
            {{ s,  s, -s}, {1, 1}, {0, 0, 1}},
            {{-s,  s, -s}, {0, 1}, {1, 1, 1}},
            {{-s, -s,  s}, {0, 0}, {0, 1, 1}},
            {{ s, -s,  s}, {1, 0}, {1, 0, 1}},
            {{ s,  s,  s}, {1, 1}, {1, 1, 0}},
            {{-s,  s,  s}, {0, 1}, {0, 0, 0}},
        };

        u32 i[] = {
            3, 2, 1, 3, 1, 0,
            4, 5, 6, 4, 6, 7,
            0, 1, 5, 0, 5, 4,
            1, 2, 6, 1, 6, 5,
            2, 3, 7, 2, 7, 6,
            3, 0, 4, 3, 4, 7,
        };

        make_mesh_from_stack_data(&geometry_primitives.cube, v, i, va, Vertex, asset_shaders.cube, asset_textures.stairway.id);
    }
    
    // Tetrahedron
    {
        typedef struct {
            Vector3 pos;
            Vector2 uv;
            Vector3 color;
        } Vertex;

        u32 va[] = {3, 2, 3};
        
        const f32 sv = 0.866025; // sqrt(0.75), sqrt(size^2 + (size/2)^2) = sqrt(0.75) * size
        const f32 s  = 1; 
        
        Vertex v[] = {
            {{-s * sv, -s * 0.5, -s * 0.75}, {1, 0}, {1, 1, 1}},
            {{ s * sv, -s * 0.5, -s * 0.75}, {0, 1}, {1, 1, 1}},
            {{       0,       s, -s * 0.75}, {1, 1}, {1, 1, 1}},
            {{       0,       0,  s * 0.75}, {0, 0}, {1, 1, 1}},
        };

        u32 i[] = {
            0, 1, 2,
            0, 1, 3,
            1, 2, 3,
            2, 0, 3,
        };
        
        make_mesh_from_stack_data(&geometry_primitives.tetrahedron, v, i, va, Vertex, asset_shaders.cube, asset_textures.test.id);
    }
    
    // Sphere 
    {
        typedef struct {
            Vector3 pos;
            Vector2 uv;
            Vector3 color;
        } Vertex;
        
        u32 vertex_structure[] = {3, 2, 3};
        
        const u32 edges = 36;
       
        u32 vertex_count = (edges / 2 - 1) * edges + 2;
        u32 index_count  = 2 * edges * 3 + (edges / 2 - 2) * edges * 3 * 2;

        Vertex* vertices = malloc(sizeof(Vertex) * vertex_count); 
        u32*    indices  = malloc(sizeof(u32)    * index_count);
        
        // fill vertices
        for (u32 i = 1; i < edges / 2; i++) {
            
            f32 r1 = TAU * (0.25 - i / (f32) edges);
            
            for (u32 j = 0; j < edges; j++) {
                
                f32 r2 = TAU * j / (f32) edges;
                
                f32 cr1 = cosf(r1);
                f32 sr1 = sinf(r1);
                f32 cr2 = cosf(r2);
                f32 sr2 = sinf(r2);
                
                // final sphere angle output 
                f32 cs  = cr2 * cr1;
                f32 ss  = sr2 * cr1;
                
                // uv
                f32 cuv = cs * 0.5 + 0.5;
                f32 suv = ss * 0.5 + 0.5;

                u32 pos = 1 + (i - 1) * edges + j;
                vertices[pos] = (Vertex) {{cs, ss, sr1}, {cuv, suv}, {1, 1, 1}};
            }
        }

        vertices[0               ] = (Vertex) {{0, 0,  1}, {0.5, 0.5}, {1, 1, 1}};
        vertices[vertex_count - 1] = (Vertex) {{0, 0, -1}, {0.5, 0.5}, {1, 1, 1}};
       

        u32 acc = 0;
        
        // top cap
        for (u32 i = 1; i < edges; i++) {
            u32 i0 = 0;
            u32 i1 = i;
            u32 i2 = i + 1;
            indices[acc + 0] = i0;
            indices[acc + 1] = i1;
            indices[acc + 2] = i2;
            acc += 3;
        }
        {
            // loop back
            u32 i0 = 0;
            u32 i1 = edges;
            u32 i2 = 1;
            indices[acc + 0] = i0;
            indices[acc + 1] = i1;
            indices[acc + 2] = i2;
            acc += 3;
        }
       

        // middle strips
        for (u32 i = 1; i < edges / 2 - 1; i++) {
            u32 up   = (i - 1) * edges + 1;
            u32 down = (i) * edges + 1;
            for (u32 j = 0; j < edges - 1; j++) {
                u32 i0 = up   + j;
                u32 i1 = down + j;
                u32 i2 = up   + j + 1;
                u32 i3 = down + j + 1;
                indices[acc + 0] = i0;
                indices[acc + 1] = i1;
                indices[acc + 2] = i3;

                indices[acc + 3] = i0;
                indices[acc + 4] = i3;
                indices[acc + 5] = i2;
                acc += 6;
            }
            {
                // loop back
                u32 j = edges - 1;

                u32 i0 = up   + j;
                u32 i1 = down + j;
                u32 i2 = up;
                u32 i3 = down;
                indices[acc + 0] = i0;
                indices[acc + 1] = i1;
                indices[acc + 2] = i3;

                indices[acc + 3] = i0;
                indices[acc + 4] = i3;
                indices[acc + 5] = i2;
                acc += 6;
            }
        }
        
        // bottom cap
        for (u32 i = edges * (edges / 2 - 2) + 1; i < vertex_count - 2; i++) {
            u32 i0 = vertex_count - 1;
            u32 i1 = i;
            u32 i2 = i + 1;
            indices[acc + 0] = i0;
            indices[acc + 1] = i2;
            indices[acc + 2] = i1;
            acc += 3;
        }
        {
            // loop back
            u32 i = edges * (edges / 2 - 2) + 1;
            u32 i0 = vertex_count - 1;
            u32 i1 = vertex_count - 2;
            u32 i2 = i;
            indices[acc + 0] = i0;
            indices[acc + 1] = i2;
            indices[acc + 2] = i1;
            acc += 3;
        }
        
        make_mesh(
            &geometry_primitives.sphere,
            (f32*) vertices, indices,     vertex_structure,
            vertex_count,    index_count, length_of(vertex_structure), 
            sizeof(Vertex), asset_shaders.cube, asset_textures.stairway.id, 0
        );
    }
}


// todo: only handle RGBA now
Texture load_texture(char* path, int channel) {

    Texture t; 
    
    t.data    = stbi_load(path, &t.w, &t.h, NULL, channel);
    t.channel = channel;
    if (!t.data) error("[Texture] Cannot load %s\n", path);

    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.w, t.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, t.data);
    
    logprint("[Texture] Loaded %s\n", path);

    return t;
}

void unload_texture(Texture* t) {
    stbi_image_free(t->data);
    free(t);
}


u32 compile_shader(char* path) {
           
    typedef struct {
        char* tag;
        u32   type;
    } TypeTag;

    const TypeTag tags[] = {
        {"[vert]", GL_VERTEX_SHADER},
        {"[frag]", GL_FRAGMENT_SHADER},
        {"[geom]", GL_GEOMETRY_SHADER},
        {"[comp]", GL_COMPUTE_SHADER},
        {"[eval]", GL_TESS_EVALUATION_SHADER},
        {"[ctrl]", GL_TESS_CONTROL_SHADER},
    };

    u32 shader = glCreateProgram();

    void* (*old_alloc)(u64) = runtime.alloc;
    runtime.alloc = temp_alloc;
    char* code = load_file_as_c_string(path);
    runtime.alloc = old_alloc;
 
    if (!code) return 0;

    char* ps[6];
    for (int i = 0; i < 6; i++) ps[i] = strstr(code, tags[i].tag); // find the tags
    for (int i = 0; i < 6; i++) {
        if (ps[i]) {
            *ps[i] = '\0'; // "cut" the string (works for now, maybe no need to switch to String)
            ps[i] += 8;    // get string starting point without tag
        }
    }

    for (int i = 0; i < 6; i++) {

        if (ps[i]) {

            u32 id = glCreateShader(tags[i].type);
            int success;
            int length;

            glShaderSource(id, 1, (const char**) &ps[i], NULL);
            glCompileShader(id);
            glGetShaderiv(id, GL_COMPILE_STATUS, &success);
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

            if (!success) {
                char* message = temp_alloc(length);
                glGetShaderInfoLog(id, length, &length, message);
                error("[GLSL] In tag %s of %s: %s", tags[i].tag, path, message);
                return 0;
            }

            glAttachShader(shader, id);
            glDeleteShader(id); // maybe pointless, since it will only be deleted when detached
        }
    }

    glLinkProgram(shader);
    glValidateProgram(shader);

    logprint("[GLSL] Compiled %s\n", path);

    return shader;
}

void save_position() {
    logprint("[Save] Position Saved.\n");
    save_file(data_string(camera), "data/save/game.pos");
}

void load_position(Camera* cam) {

    char* path = "data/save/game.pos"; 
    FILE* f = fopen(path, "rb");
    if (!f) goto fail;  

    u8* data;
    u64 count;

    fseek(f, 0, SEEK_END);
    count = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (!count || count != sizeof(Camera)) goto fail;

    data = temp_alloc(count);
    fread(data, 1, count, f);
    fclose(f);

    *cam = *(Camera*) data;
    logprint("[Save] Position loaded.\n");
    return;

    fail:
    logprint("[Save] [Warning] Cannot Load position, use default.\n");
}






/* ==== Input ==== */

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, 1); break;
            case GLFW_KEY_F1:     toggle_vsync(); break;
            case GLFW_KEY_F2:     toggle_cursor(); break;
            case GLFW_KEY_F3:     toggle_debug_info(); break;
            case GLFW_KEY_F11:    toggle_fullscreen(); break;
            case GLFW_KEY_Z:      change_draw_mode(-1); break;
            case GLFW_KEY_X:      change_draw_mode(1); break;
            case GLFW_KEY_DOWN:   change_engine_speed(0.5); break;
            case GLFW_KEY_UP:     change_engine_speed(2); break;
            case GLFW_KEY_LEFT:   change_movement_speed(0.5); break;
            case GLFW_KEY_RIGHT:  change_movement_speed(2); break;
        }
    }
}

void scroll_callback(GLFWwindow* window, f64 dx, f64 dy) {
    camera.FOV += (int) dy;
    clamp_s32(&camera.FOV, 1, 179);
    update_camera_projection(&camera);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

    if (action == GLFW_PRESS) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_MIDDLE: {
                camera.FOV = 70;
                update_camera_projection(&camera);
                break;
            }
        }
    }
}





/* ==== Setup ==== */

void setup(int arg_count, char** args) {


    /* ---- Setup Runtime ---- */
    {
        u64 size = 1024 * 256;
        runtime.temp_buffer.data = calloc(size, sizeof(u8));
        runtime.temp_buffer.size = size;
        runtime.alloc    = malloc;
        runtime.log_file = stdout;
        
        runtime.command_line_args = (Array(String)) {
            .data  = malloc(sizeof(String) * arg_count),
            .count = arg_count,
        };

        for (int i = 0; i < arg_count; i++) {
            String* s = &runtime.command_line_args.data[i];
            s->data  = (u8*) args[i];
            s->count = strlen(args[i]);
        }
    }
   

    /* ---- Init Window and OpenGL ---- */

    {
        WindowInfo* w = &window_info;
        setvbuf(stdout, NULL, _IONBF, 0); // force mitty to print immediately

        glfwInit();

        // use OpenGL core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        // glfwWindowHint(GLFW_SAMPLES, 4); // anti-aliasing

        // set window and load OpenGL functions
        w->handle = glfwCreateWindow(w->width, w->height, "Engine", NULL, NULL);
        glfwMakeContextCurrent(w->handle);
        glfwSetFramebufferSizeCallback(w->handle, resize_framebuffer);
        glfwSetKeyCallback(w->handle, key_callback);
        glfwSwapInterval(w->vsync);
        glfwSetInputMode(w->handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetScrollCallback(w->handle, scroll_callback);
        glfwSetMouseButtonCallback(w->handle, mouse_button_callback);

        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(w->handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        gladLoadGL();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE);

        w->is_first_frame = 1;
        camera.view = M4_IDENTITY;
        resize_framebuffer(w->handle, w->width, w->height);
    }


    /* ---- Setup Debug (comment out to disable) ---- */

    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GL_print_debug_message, NULL);
        logprint(
            "[OpenGL] Vendor:         %s\n"
            "[OpenGL] Renderer:       %s\n"
            "[OpenGL] OpenGL Version: %s\n"
            "[OpenGL] GLSL Version:   %s\n",
            glGetString(GL_VENDOR),
            glGetString(GL_RENDERER),
            glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION)
        );
    }


    /* ---- Load Resources ---- */

    {
        // Load All Textures 
        {
            Asset_Textures* t = &asset_textures;

            stbi_set_flip_vertically_on_load(1);
            t->test       = load_texture("data/bitmaps/test.png", 4);
            t->sun        = load_texture("data/bitmaps/sun.png", 4);
            t->stairway   = load_texture("data/bitmaps/stairway.png", 4);
            t->styxel     = load_texture("data/fonts/styxel_trans.png", 4);
            t->styxel_8x8 = load_texture("data/fonts/styxel_8x8.png", 4);
            t->sb_16x16   = load_texture("data/fonts/sb_16x16_trans.png", 4);
        }

        // Compile All Shaders 
        {
            Asset_Shaders* s = &asset_shaders;

            s->cube = compile_shader("data/shaders/cube.glsl");
            s->rect = compile_shader("data/shaders/rect.glsl");
            s->axis = compile_shader("data/shaders/axis.glsl");
            s->font = compile_shader("data/shaders/font.glsl");

        }

        make_geometry_primitives();
        
        load_position(&camera);
    }


}







/* ---- Experiment: Gravity ---- */

const float G = 0.001;

// janky, get the formula right
Vector3 get_gravity(CelestialBody a, CelestialBody b) {
    Vector3 d = v3_sub(b.base.position, a.base.position);
    float dl  = v3_length(d);
    return v3_scale(v3_normalize(d), (G * a.mass * b.mass / (dl * dl)));
}

void update_velocity(CelestialBody* b, Vector3 force, float dt) {
    b->velocity = v3_add(b->velocity, v3_scale(force, dt / b->mass));
}

void update_position(CelestialBody* b, float dt) {
    b->base.position = v3_add(b->base.position, v3_scale(b->velocity, dt));
}







/* ==== Process ==== */

void process_inputs(f64 dt) {

    WindowInfo* w = &window_info;
    Camera*   cam = &camera;
    f64 mdx, mdy; // delta for mouse position
    {
        f64 mx, my;
        glfwGetCursorPos(window_info.handle, &mx, &my);
        mdx = mx - w->cursor.x;
        mdy = my - w->cursor.y;
        w->cursor.x = mx;
        w->cursor.y = my;
    }

    Vector3 dv = {0};
    // Rotor3D dr = {1, 0, 0, 0}; // 3-way free camera

    float factor = 10 * movement_speed_scale;

    if (glfwGetKey(w->handle, GLFW_KEY_D) == GLFW_PRESS) dv.x  =  dt * factor;
    if (glfwGetKey(w->handle, GLFW_KEY_A) == GLFW_PRESS) dv.x  = -dt * factor;
    if (glfwGetKey(w->handle, GLFW_KEY_W) == GLFW_PRESS) dv.y  =  dt * factor;
    if (glfwGetKey(w->handle, GLFW_KEY_S) == GLFW_PRESS) dv.y  = -dt * factor;
    if (glfwGetKey(w->handle, GLFW_KEY_R) == GLFW_PRESS) dv.z  =  dt * factor;
    if (glfwGetKey(w->handle, GLFW_KEY_F) == GLFW_PRESS) dv.z  = -dt * factor;


    // 3-way free camera
    // if (glfwGetKey(w->handle, GLFW_KEY_Q) == GLFW_PRESS) dr.zx = -TAU * dt * 0.1;
    // if (glfwGetKey(w->handle, GLFW_KEY_E) == GLFW_PRESS) dr.zx =  TAU * dt * 0.1;
    // 3-way free camera
    // dr.yz -= mdy * 0.001;
    // dr.xy -= mdx * 0.001;

    if (w->cursor_visible) return;
    
    // ehh... is there a better solution?
    if (w->is_first_frame) {
        w->is_first_frame = 0;
        update_camera_projection(&camera);
        // dr = (Rotor3D) {1, 0, 0, 0}; // 3-way free camera
    } else {

        if (glfwGetKey(w->handle, GLFW_KEY_Q) == GLFW_PRESS) cam->zx += -TAU * dt * 0.3;
        if (glfwGetKey(w->handle, GLFW_KEY_E) == GLFW_PRESS) cam->zx +=  TAU * dt * 0.3;

        cam->yz -= mdy * 0.003;
        cam->xy -= mdx * 0.003;

        clamp_f32(&cam->yz, -0.22 * TAU, 0.22 * TAU);
        while (cam->xy >  TAU) cam->xy -= TAU;
        while (cam->xy < -TAU) cam->xy += TAU;
    }

    // 3-way free camera, todo: understand this. reverse order here (first delta then prev orientation), 
    // and we can get correct behavior, but why? maybe because we need to reverse it later, so we need opposite order?
    // cam->orientation = r3d_mul(r3d_normalize(dr), cam->orientation); 
    
    // zx -> yz -> xy (reverse is xy -> yz -> zx, so the "reverse order" above seems to be a thing?)
    cam->orientation = r3d_mul(
        r3d_mul(
            r3d_from_plane_angle(B3_ZX, cam->zx), 
            r3d_from_plane_angle(B3_YZ, cam->yz)
        ),
        r3d_from_plane_angle(B3_XY, cam->xy)
    );

    cam->position    = v3_add(cam->position, v3_rotate(dv, cam->orientation));
    cam->view        = m4_mul(r3d_to_m4(r3d_reverse(cam->orientation)), m4_translate(v3_reverse(cam->position))); // reverse pos and orientation to get world transform
}

