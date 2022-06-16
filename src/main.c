#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>



/* ---- Third Party ---- */
#define GLFW_INCLUDE_NONE
//#define STB_TRUETYPE_IMPLEMENTATION 

#include "glad.h"
#include "glfw3.h"
#include "stb_image.h"
//#include "stb_truetype.h"



/* ---- Modules ---- */

// todo: linux and mac
#ifdef OS_WINDOWS
#include "layer/win32.c"

// On Windows, these stupid global variables force laptops to use dedicated GPU
__declspec(dllexport) int NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

#include "runtime.c"
#include "file.c"
#include "linear_algebra.c"
#include "backend.c"





// temp
Vector2* test_vertices;
u32      test_count;
u32      test_shader;
u32      test_vbo;
u32      test_vao;


// temp
void get_char_xy(u8 c, int* x, int* y) {
    int d = c - ' ';
    *x = d % 16;
    *y = d / 16;
}


void get_mesh_fonts() {
    
    u8* data;
    int w;
    int h;
    
    // switch the x y back (because stb flip it) and get rid of channels, just for convenience, maybe slow
    {
        Texture* t = &asset_textures.styxel;

        w = t->w;
        h = t->h;
        data = malloc(w * h); 

        u64 acc = 0;
        for (int i = h - 1; i >= 0; i--) {
            for (int j = 0; j < w * 4; j += 4) {
                data[acc] = t->data[i * w * 4 + j];
                acc++;
            }
        }
    }

    printf("w %d h %d\n", w, h);
    
    int char_w = 6;
    int char_h = 6;
    int x;
    int y;
    get_char_xy('A', &x, &y);
    
    u32 total_pixel_count = 0;

    for (int i = y * char_h; i < (y + 1) * char_h; i++) {
        for (int j = x * char_w; j < (x + 1) * char_w; j++) {
            printf("%c", data[i * w + j] ? 'O' : ' ');
            if (data[i * w + j]) total_pixel_count++;
        }
        printf("\n");
    }
    
    printf("total pixel count %d\n", total_pixel_count);
    
    Vector2* vertices = malloc(sizeof(Vector2) * total_pixel_count * 6);
    
    // todo: flip back now, so we can merge this with the flip above
    u64 acc = 0;
    for (int i = y * char_h; i < (y + 1) * char_h; i++) {
        for (int j = x * char_w; j < (x + 1) * char_w; j++) {
          
            if (data[i * w + j]) {

                Vector2 p0 = {
                    ((j + 0) % char_w) / (f32) char_w, 
                    1 - ((i + 0) % char_h) / (f32) char_h, 
                };

                Vector2 p3 = {
                    (j % char_w + 1) / (f32) char_w, 
                    1 - (i % char_h + 1) / (f32) char_h, 
                };
                
                Vector2 p1 = {p3.x, p0.y};
                Vector2 p2 = {p0.x, p3.y};
                
                vertices[acc + 0] = p2;
                vertices[acc + 1] = p3;
                vertices[acc + 2] = p1;
                vertices[acc + 3] = p2;
                vertices[acc + 4] = p1;
                vertices[acc + 5] = p0;

                acc += 6;
 
/*/
                print_v2(p0);
                print_v2(p1);
                print_v2(p2);
                print_v2(p3);
/*/
            }
        }
    }

    test_vertices = vertices;
    test_count    = total_pixel_count * 6;

    free(data);

    test_shader = asset_shaders.rect;

    glGenBuffers(     1, &test_vbo);
    glGenVertexArrays(1, &test_vao);

    glUseProgram(test_shader); 
    glBindBuffer(GL_ARRAY_BUFFER, test_vbo);
    glBufferData(GL_ARRAY_BUFFER, test_count * sizeof(Vector2), (f32*) test_vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(test_vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*) 0);
    glEnableVertexAttribArray(0);
}



void draw_char_test() {

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Vector4 color = {1, 1, 1, 1};
    Vector4 color2 = {0, 0, 0, 0.7};
    
    Vector2 scale = {0.05, 0.05};

    Matrix2 m = m2_mul(m2_scale(scale), m2_scale((Vector2) {1 / window_info.aspect, 1}));

    u32 shader = test_shader;
    glUseProgram(shader); 

    glBindBuffer(GL_ARRAY_BUFFER, test_vbo);
    glBindVertexArray(test_vao);

    glUniformMatrix2fv(glGetUniformLocation(shader, "transform"), 1, GL_FALSE, (f32*) &m);
   
    for (int i = 0; i < 6; i++) {
        Vector2 position = {i * 0.05 + 0.004, 0.1 - 0.006};
        glUniform4fv(glGetUniformLocation(shader, "color"), 1, (f32*) &color2);
        glUniform2fv(glGetUniformLocation(shader, "position"), 1, (f32*) &position);
        glDrawArrays(GL_TRIANGLES, 0, test_count * 2);
    }
     
    for (int i = 0; i < 6; i++) {
        Vector2 position = {i * 0.05, 0.1};
        glUniform4fv(glGetUniformLocation(shader, "color"), 1, (f32*) &color);
        glUniform2fv(glGetUniformLocation(shader, "position"), 1, (f32*) &position);
        glDrawArrays(GL_TRIANGLES, 0, test_count * 2);
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}












int main(int c_arg_count, char** c_args) {


    setup(c_arg_count, c_args);

    get_mesh_fonts();


    /*/
    String roboto_ttf = load_file("data/fonts/Roboto-Regular.ttf");
    
    stbtt_fontinfo font;
    stbtt_InitFont(&font, roboto_ttf.data, stbtt_GetFontOffsetForIndex(roboto_ttf.data, 0));

    // for (int j = 0; j < h; j++) {
    //     for (int i = 0; i < w; i++) {
    //         putchar(" .:ioVM@"[bitmap[j * w + i] >> 5]);
    //     }
    //     putchar('\n');
    // }
    
    // find a simple way to draw a quad
    u32 my_fid;
    {
        int w, h;
        u8* bitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, 256), 'A', &w, &h, 0, 0);
        
        // todo: use red channel don't work, why????
        u8* rgba_bitmap = calloc(1, w * h * 4); 
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                int x = j * w + i;
                rgba_bitmap[x * 4] = bitmap[x];
            }
        }
        
        glGenTextures(1, &my_fid);
        glBindTexture(GL_TEXTURE_2D, my_fid);
        glTexImage2D(
            GL_TEXTURE_2D, 
            0, 
            GL_RGBA,
            w, 
            h, 
            0, 
            GL_RGBA, 
            GL_UNSIGNED_BYTE, 
            rgba_bitmap
        );
    }
    /*/


    GeometryPrimitives* gp = &geometry_primitives;

    Model3D object = {
        .base = {
            .position    = {0, 0, 0},
            .scale       = {2, 2, 2},
            .orientation = {1, 0, 0, 0},
        },
        .mesh = &gp->cube,
    };
 
    Model3D object2 = {
        .base = {
            .position    = {-2, -3, -5},
            .scale       = {2, 2, 12},
            .orientation = {1, 0, 0, 0},
        },
        .mesh = &gp->cube,
    };
 
    Model3D object3 = {
        .base = {
            .position    = {6, 0, -10},
            .scale       = {10, 10, 6},
            .orientation = {1, 0, 0, 0},
        },
        .mesh = &gp->cube,
    };
    
    Model3D room[6] = {
        {
            .base = {
                .position    = {0, 0, -10},
                .scale       = {100, 100, 0.1},
                .orientation = {1, 0, 0, 0},
            },
            .mesh = &gp->cube,
        },
        {
            .base = {
                .position    = {0, 0, 10},
                .scale       = {100, 100, 0.1},
                .orientation = {1, 0, 0, 0},
            },
            .mesh = &gp->cube,
        },
        {
            .base = {
                .position    = {-50, 0, 0},
                .scale       = {0.1, 100, 20},
                .orientation = {1, 0, 0, 0},
            },
            .mesh = &gp->cube,
        },
        {
            .base = {
                .position    = {50, 0, 0},
                .scale       = {0.1, 100, 20},
                .orientation = {1, 0, 0, 0},
            },
            .mesh = &gp->cube,
        },
        {
            .base = {
                .position    = {0, 50,  0},
                .scale       = {100, 0.1, 20},
                .orientation = {1, 0, 0, 0},
            },
            .mesh = &gp->cube,
        },
        {
            .base = {
                .position    = {0, -50,  0},
                .scale       = {100, 0.1, 20},
                .orientation = {1, 0, 0, 0},
            },
            .mesh = &gp->cube,
        },
    };



    Timer lerp_clock = {0};
    Timer text_pulse = {0};
    Timer fps_clock  = {.interval = 1};

    f64 time_now = glfwGetTime();


    // main loop
    while (1) {


        // get dt
        f64 dt;
        {
            f64 next = glfwGetTime();
            dt       = next - time_now;
            time_now = next;
        }
        update_FPS_timer(&fps_clock, dt);


        /* ==== Simulate ==== */ 
        {
            process_inputs(dt);
            f64 scaled_dt    = dt * engine_speed_scale;
            lerp_clock.base += scaled_dt;
            text_pulse.base += dt * 6;
            
            f32 lerp_value          = sin_normalize(lerp_clock.base);
            object.base.orientation = nlerp_r3d(R3D_DEFAULT, r3d_from_plane_angle(B3_XY, TAU * 0.25), lerp_value);
            object.base.position    = lerp_v3((Vector3) {0, 0, -5}, (Vector3) {0, 0, -3}, lerp_value);
            
            light = (Vector3) {cos(lerp_clock.base) * 20, sin(lerp_clock.base) * 20, 0};
        }



        /* ==== Render ==== */ 

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        /* ---- 3D ---- */
        
        draw_model(room, 6, &camera);
        draw_model(&object, 1, &camera);
        draw_model(&object2, 1, &camera);
        draw_model(&object3, 1, &camera);

        draw_char_test();



        /* ---- 2D ---- */

        {
            Vector2 pos        = lerp_v2((Vector2) {-0.65, 0.7}, (Vector2) {-0.65, 0.72}, sin_normalize(text_pulse.base));
            Vector2 pos2       = v2_add(pos, (Vector2) {0, -0.2});
            Vector2 scale      = {0.1, 0.1}; 
            Vector2 offset     = {0.005, -0.007};
            Vector4 color      = lerp_v4((Vector4) {0.5, 0.7, 0.95, 1}, (Vector4) {0.2, 0.8, 0.45, 1}, sin_normalize(text_pulse.base));
            Vector4 color_back = {0, 0, 0, 0.7}; 

            draw_string_shadowed(pos , offset, scale, color, color_back, string("WASD to move, QE to roll"));
            draw_string_shadowed(pos2, offset, scale, color, color_back, string("ESC to exit"));
        }
        
        if (window_info.show_debug_info) {

            String fps;
            String draw_mode;
            {
                Timer* t = &fps_clock;
                Camera* c = &camera;
                f64 v = t->counter / t->base / t->interval;
                
                char* mode = "(Unknown)";
                switch (c->draw_mode) {
                    case GL_POINTS:         mode = "GL_POINTS";         break;
                    case GL_LINES:          mode = "GL_LINES";          break;
                    case GL_LINE_LOOP:      mode = "GL_LINE_LOOP";      break;
                    case GL_LINE_STRIP:     mode = "GL_LINE_STRIP";     break;
                    case GL_TRIANGLES:      mode = "GL_TRIANGLES";      break;
                    case GL_TRIANGLE_STRIP: mode = "GL_TRIANGLE_STRIP"; break;
                    case GL_TRIANGLE_FAN:   mode = "GL_TRIANGLE_FAN";   break;
                }
    
                fps       = temp_print("Frametime: %fms  FPS: %f", 1000 / v, v);
                draw_mode = temp_print("Mesh Draw Mode: %s", mode);

                if (window_info.is_first_frame) fps.count = 0;
            }
            
            Vector2 scale      = {0.03, 0.03};
            Vector2 offset     = {scale.x * 0.1, scale.y * -0.1};
            Vector4 color      = V4_UNIT;
            Vector4 color_back = {0, 0, 0, 0.7};
            
            draw_string_shadowed((Vector2) {-0.95, 0.9},  offset, scale, color, color_back, fps);
            draw_string_shadowed((Vector2) {-0.95, 0.85}, offset, scale, color, color_back, temp_print("Engine   Speed: %f", engine_speed_scale));
            draw_string_shadowed((Vector2) {-0.95, 0.8},  offset, scale, color, color_back, temp_print("Movement Speed: %f", movement_speed_scale));
            draw_string_shadowed((Vector2) {-0.95, 0.75}, offset, scale, color, color_back, draw_mode);

            draw_axis_arrow((Vector3) {0.05, 0.05, 0.05}, &camera);

        } else {

            // cursor
            draw_circle((Vector2) {0, 0}, (Vector2) {0.014, 0.014}, (Vector4) {0, 0, 0, 0.2});
            draw_circle((Vector2) {0, 0}, (Vector2) {0.012, 0.012}, (Vector4) {1, 1, 1, 0.8});
        }


        // end frame
        temp_reset();
        
        if (glfwWindowShouldClose(window_info.handle)) break;
        glfwSwapBuffers(window_info.handle);
        glfwPollEvents();
    }
    
    save_position();
    glfwTerminate(); 

    return 0;
}

