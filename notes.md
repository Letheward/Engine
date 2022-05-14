# Notes

## Bugs

- m4_mul and m2_mul and backwards????? why??????
    - because we use column major order
        - maybe name the m*_mul() function parameters better to column major order




## TODO

- Shader Loading System
    - remove hardcoded stuff

- UI
    - draw a 2D cursor 
        - we can do it now! but off pixel looks ugly
- use multiple shaders
    - kinda make sense now
    - how does it actually work?
- draw custom object
- camera reverse rotation problem
    - multiplication order???
    - maybe this is just how it is?

- dt based framerate independence
    - dt based velocity kinda done, but is it accurate or optimal?
- handle screen resize, aspect ratio and fullscreen
    - 3D part sorta done
    - 2D part, some progress

- shader hot reloading
- use SPIR-V? or better shader compilation

- current 60FPS bound is about 32768 cubes, improve that
    - seems like draw call and send data to GPU is the bottleneck

Long Term Goals:
- in-game editor
- audio
- multithreading
- replace GLFW with own version
- replace GLAD (maybe not possible or necessary)
- replace stb_image (maybe not necessary)

etc.:
