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
            .position    = {1, 2, 2},
            .scale       = {1, 1, 1},
            .orientation = {1, 0, 0, 0},
        },
        .mesh = &gp->tetrahedron,
    };

    Model3D room = {
        .base = {
            .position    = {3, 3, 3},
            .scale       = {100, 100, 10},
            .orientation = {1, 0, 0, 0},
        },
        .mesh = &gp->cube,
    };

    Timer  lerp_clock = {0};
    Timer  fps_clock  = {.interval = 1};
    String fps_string = {calloc(256, sizeof(u8)), 256};

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
        update_FPS_string(&fps_clock, dt, &fps_string);


        process_inputs(dt);


        // simulate
        f64 scaled_dt    = dt * engine_speed_scale;
        lerp_clock.base += scaled_dt;
        
        float lerp_value = (1 + sin(lerp_clock.base)) / 2;
        object.base.orientation = nlerp_r3d(R3D_DEFAULT, r3d_from_plane_angle(B3_XY, TAU * 0.25), lerp_value);
        object.base.position    = lerp_v3((Vector3) {0, 0, 1}, (Vector3) {0, 0, 3}, lerp_value);


        // render, todo: this is the most expensive part of the loop
        //fill_FPS_at_title(&fps_clock, dt);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        draw_model(&room, 1, &camera);
        draw_model(&object, 1, &camera);
        
        draw_ring((Vector2) {0.1, 0.1}, (Vector4) {0.5, 0.5, 0.5, 1});
        draw_axis_arrow((Vector3) {0.05, 0.05, 0.05}, &camera);

        //draw_rect(&m, 1, (Vector4) {1, 1, 1, 1});
        //draw_circle();
        
        if (window_info.show_debug_info) {

            String speed = {temp_alloc(128), 128};
            speed.count = snprintf((char*) speed.data, speed.count, "Engine Speed: %f", engine_speed_scale);
            
            draw_string((Vector2) {-0.95, 0.9}, (Vector2) {0.04, 0.04}, fps_string);
            draw_string((Vector2) {-0.95, 0.8}, (Vector2) {0.04, 0.04}, speed);
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

