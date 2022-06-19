# Notes

## Bugs

- why dynamically changing vertices give bad outputs? (even though data in RenderDoc is correct)

- 3D transparent texture leak problem
- 2D texture leak problem (in bitmap fonts)
- if we want to do unity build using `win32_helper.c`, in current header order, something in `windows.h` makes gcc unhappy

## TODO

- m4_mul and m2_mul and backwards
    - because we use column major order
        - maybe name the m*_mul() function parameters better to column major order

- Shader Loading System
    - remove hardcoded stuff

- cleanup mesh font

- improve text drawing performance

- Configable oversampling at runtime

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
