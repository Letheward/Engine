#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#define GLFW_INCLUDE_NONE
//#define STB_TRUETYPE_IMPLEMENTATION 

#include "glad.h"
#include "glfw3.h"
#include "stb_image.h"
//#include "stb_truetype.h"

#include "clean.c"
#include "linear_algebra.c"
#include "backend.c"



int main(int c_arg_count, char** c_args) {
    
    setup(c_arg_count, c_args);
    
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
            .position    = {0, 0, 4},
            .scale       = {2, 2, 2},
            .orientation = {1, 0, 0, 0},
        },
        .mesh = &gp->sphere,
    };
    
    Model3D room = {
        .base = {
            .position    = {0, 0, 10},
            .scale       = {100, 100, 20},
            .orientation = {1, 0, 0, 0},
        },
        .mesh = &gp->cube,
    };


    /*/
    CelestialBody bodies[256];

    // blackhole
    bodies[0] = (CelestialBody) {
        .base = {
            .position    = {0, 0, 0},
            .scale       = {100, 100, 100}, 
            .orientation = {1, 0, 0, 0},
        },
        .velocity = {0},
        .mass     = 10000000000,   
        .mesh = &gp->sphere,
    };

    for (int i = 1; i < length_of(bodies); i++) {
        float b1 = rand() % 2 ? 1 : -1;
        float b2 = rand() % 2 ? 1 : -1;
        f32 rad = (rand() % 256) / 256.0 * TAU;
        
        bodies[i] = (CelestialBody) {
            .base = {
                .position    = {b1 * (rand() % 512 + 300), b2 * (rand() % 512 + 300), 0 }, //rand() % 1024 - 512},
                .scale       = {10, 10, 10},
                .orientation = {1, 0, 0, 0},
            },
            .velocity = {cosf(rad), sinf(rad), 0}, //rand() / v * b3},
            .mass     = (rand() % 1000) * 100 + 10000,
            .mesh = &gp->sphere,
        };
        f32 l = v3_length(bodies[i].base.position);
        bodies[i].velocity = v3_scale(bodies[i].velocity, l * 0.2);
    }
    /*/



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


        // simulate
        process_inputs(dt);
        f64 scaled_dt    = dt * engine_speed_scale;
        lerp_clock.base += scaled_dt;
        text_pulse.base += dt * 8;
        
        f32 lerp_value          = sin_normalize(lerp_clock.base);
        object.base.orientation = nlerp_r3d(R3D_DEFAULT, r3d_from_plane_angle(B3_XY, TAU * 0.25), lerp_value);
        object.base.position    = lerp_v3((Vector3) {0, 0, 3}, (Vector3) {0, 0, 4}, lerp_value);

        /*/
        for (int i = 0; i < length_of(bodies); i++) {
            for (int j = i + 1; j < length_of(bodies); j++) {
                Vector3 v = get_gravity(bodies[i], bodies[j]);
                update_velocity(&bodies[i], v, scaled_dt);
                update_velocity(&bodies[j], v3_reverse(v), scaled_dt);
            }
        }

        for (int i = 0; i < length_of(bodies); i++) {
            update_position(&bodies[i], scaled_dt);
        }
        /*/


        // render, todo: this is the most expensive part of the loop
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // 3D        
        draw_model(&room, 1, &camera);
        draw_model(&object, 1, &camera);


        // 2D
        draw_string_shadowed(
            (Vector2) {-0.45, 0.7},  
            (Vector2) {0.006, -0.009}, 
            (Vector2) {0.1, 0.1}, 
            lerp_v4((Vector4) {0.5, 0.7, 0.95, 1}, (Vector4) {0.2, 0.8, 0.45, 1}, sin_normalize(text_pulse.base)),
            (Vector4) {0, 0, 0, 0.7}, 
            string("Mono Font Works!")
        );

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
            
            Vector2 text_scale  = {0.03, 0.03};
            Vector4 text_fg     = V4_UNIT;
            Vector4 text_bg     = {0, 0, 0, 0.7};
            Vector2 text_offset = {text_scale.x * 0.1, text_scale.y * -0.1};
            
            draw_string_shadowed((Vector2) {-0.95, 0.9},  text_offset, text_scale, text_fg, text_bg, fps);
            draw_string_shadowed((Vector2) {-0.95, 0.85}, text_offset, text_scale, text_fg, text_bg, temp_print("Engine   Speed: %f", engine_speed_scale));
            draw_string_shadowed((Vector2) {-0.95, 0.8},  text_offset, text_scale, text_fg, text_bg, temp_print("Movement Speed: %f", movement_speed_scale));
            draw_string_shadowed((Vector2) {-0.95, 0.75}, text_offset, text_scale, text_fg, text_bg, draw_mode);

            draw_axis_arrow((Vector3) {0.05, 0.05, 0.05}, &camera);

        } else {

            // cursor
            draw_circle((Vector2) {0.014, 0.014}, (Vector4) {0, 0, 0, 0.2});
            draw_circle((Vector2) {0.012, 0.012}, (Vector4) {1, 1, 1, 0.8});
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

