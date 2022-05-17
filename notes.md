# Notes

## Bugs

- 3D Transparent texture leak problem


## TODO

- m4_mul and m2_mul and backwards
    - because we use column major order
        - maybe name the m*_mul() function parameters better to column major order

- Shader Loading System
    - remove hardcoded stuff

- UI
    - draw a 2D cross cursor 
- use multiple shaders
    - kinda make sense now
    - how does it actually work?
- draw custom object
- camera reverse rotation problem
    - reverse multiplication order, maybe this is just how it is?

- dt based framerate independence
    - dt based velocity kinda done, but is it accurate or optimal?
- handle screen resize, aspect ratio and fullscreen
    - 3D part sorta done
    - 2D part sorta done

- shader hot reloading
- use SPIR-V? or better shader compilation


Long Term Goals:
- in-game editor
- audio
- multithreading
- replace GLFW with own version
- replace GLAD (maybe not possible or necessary)
- replace stb_image (maybe not necessary)

etc.:
