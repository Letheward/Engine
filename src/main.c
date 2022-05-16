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
            .position    = {0, 0, 2},
            .scale       = {1, 1, 1},
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

    Timer lerp_clock   = {0};
    Timer cursor_pulse = {0};
    Timer fps_clock    = {.interval = 1};

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
        f64 scaled_dt      = dt * engine_speed_scale;
        lerp_clock.base   += scaled_dt;
        cursor_pulse.base += dt * 4;
        
        f32 lerp_value          = sin_normalize(lerp_clock.base);
        object.base.orientation = nlerp_r3d(R3D_DEFAULT, r3d_from_plane_angle(B3_XY, TAU * 0.25), lerp_value);
        object.base.position    = lerp_v3((Vector3) {0, 0, 1}, (Vector3) {0, 0, 3}, lerp_value);


        // render, todo: this is the most expensive part of the loop
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 3D        
        draw_model(&room, 1, &camera);
        draw_model(&object, 1, &camera);

        // 2D
        if (window_info.show_debug_info) {

            String speed = {temp_alloc(128), 128};
            String fps   = {temp_alloc(128), 128};

            {
                Timer* t = &fps_clock;
                f64 v = t->counter / t->base / t->interval;
                fps.count   = snprintf((char*) fps.data,   fps.count,   "Frametime: %fms  FPS: %f", 1000 / v, v);
                speed.count = snprintf((char*) speed.data, speed.count, "Engine Speed: %f", engine_speed_scale);
                if (window_info.is_first_frame) fps.count = 0;
            }
            
            Vector2 text_scale = {0.04, 0.04};
            Vector4 text_color = {0.9, 0.9, 0.9, 1};
            draw_string((Vector2) {-0.95, 0.9}, text_scale, text_color, fps);
            draw_string((Vector2) {-0.95, 0.8}, text_scale, text_color, speed);
            draw_axis_arrow((Vector3) {0.05, 0.05, 0.05}, &camera);

        } else {
            
            // cursor
            Vector2 pulse = lerp_v2((Vector2) {0.010, 0.010}, (Vector2) {0.012, 0.012}, sin_normalize(cursor_pulse.base));
            draw_circle(pulse, (Vector4) {1, 1, 1, 1});
            draw_ring(v2_add(pulse, (Vector2) {0.005, 0.005}), 2, (Vector4) {0.3, 0.3, 0.3, 1});
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

