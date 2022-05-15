#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#define GLFW_INCLUDE_NONE
#define STB_TRUETYPE_IMPLEMENTATION 

#include "glad.h"
#include "glfw3.h"
#include "stb_image.h"
#include "stb_truetype.h"

#include "clean.c"
#include "linear_algebra.c"
#include "backend.c"



// temp
void draw_char(int id, Camera* cam) {

    Matrix4 m = M4_IDENTITY;
    Mesh* mesh = &geometry_primitives.cube;

    glUseProgram(mesh->id.shader); 
    glBindVertexArray(mesh->id.vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->id.vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->id.indices);
     
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(mesh->id.shader, "texture0"), 0);
    
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "projection"), 1, GL_FALSE, (f32*) &cam->projection);
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "view"), 1, GL_FALSE, (f32*) &cam->view);
    
    glUniformMatrix4fv(glGetUniformLocation(mesh->id.shader, "model"), 1, GL_FALSE, (f32*) &m);
    glDrawElements(cam->draw_mode, 6, GL_UNSIGNED_INT, NULL);
}



int main(int c_arg_count, char** c_args) {
    
    setup(c_arg_count, c_args);

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


    Timer lerp_clock = {0};
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


        process_inputs(dt);


        // simulate
        f64 scaled_dt    = dt * engine_speed_scale;
        lerp_clock.base += scaled_dt;
        
        float lerp_value = (1 + sin(lerp_clock.base)) / 2;
        object.base.orientation  = nlerp_r3d(R3D_DEFAULT, r3d_from_plane_angle(B3_XY, TAU * 0.25), lerp_value);
        object.base.position     = lerp_v3((Vector3) {0, 0, 1}, (Vector3) {0, 0, 3}, lerp_value);
        

        // render, todo: this is the most expensive part of the loop
        fill_FPS_at_title(&fps_clock, dt);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw_model(&room, 1, &camera);
        
        draw_char(my_fid, &camera);
        draw_model(&object, 1, &camera);
        draw_axis_arrow((Vector3) {0.05, 0.05, 0.05}, &camera);
    
        //draw_cursor();
        //draw_circle();

        // swap buffer
        if (glfwWindowShouldClose(window_info.handle)) break;
        glfwSwapBuffers(window_info.handle);
        glfwPollEvents();
    }
    
    save_position();
    glfwTerminate(); 

    return 0;
}

