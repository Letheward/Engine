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
__declspec(dllexport) int NvOptimusEnablement                  = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

#include "runtime.c"
#include "file.c"
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



        /* ---- 2D ---- */

        {
            Vector2 pos        = lerp_v2((Vector2) {-0.65, 0.7}, (Vector2) {-0.65, 0.72}, sin_normalize(text_pulse.base));
            Vector2 pos2       = v2_add(pos, (Vector2) {0, -0.2});
            Vector2 scale      = {0.1, 0.1}; 
            Vector2 offset     = {0.005, -0.007};
            Vector4 color      = lerp_v4((Vector4) {0.5, 0.7, 0.95, 1}, (Vector4) {0.2, 0.8, 0.45, 1}, sin_normalize(text_pulse.base));
            Vector4 color_back = {0, 0, 0, 0.7}; 

            draw_mesh_string_shadowed(pos , offset, scale, color, color_back, string("WASD to move, QE to roll"));
            draw_mesh_string_shadowed(pos2, offset, scale, color, color_back, string("ESC to exit"));
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
            
            draw_mesh_string_shadowed((Vector2) {-0.95, 0.9},  offset, scale, color, color_back, fps);
            draw_mesh_string_shadowed((Vector2) {-0.95, 0.85}, offset, scale, color, color_back, temp_print("Engine   Speed: %f", engine_speed_scale));
            draw_mesh_string_shadowed((Vector2) {-0.95, 0.8},  offset, scale, color, color_back, temp_print("Movement Speed: %f", movement_speed_scale));
            draw_mesh_string_shadowed((Vector2) {-0.95, 0.75}, offset, scale, color, color_back, draw_mode);

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

