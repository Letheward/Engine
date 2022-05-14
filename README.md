# Engine

Toy game engine in C.

## Building

Currently this is only a backup, haven't think about collaboration yet.

If you really want to build it, you'll need to:

- Have a Windows or Linux machine. It compiles and runs on:
    - Windows 10 (developed on) 
    - Fedora 36 (through a great pain of modifying headers and copy `.so` to make gcc find it, etc. The `build.sh` will need some changes)
- Get gcc (on Windows, from MinGW64)
- Get a `sh` style shell (for `build.sh`, although it's pretty simple to translate to other shells)
- Get a binary of GLFW
- Compile the `glad` and `stb` sources in repo
- Find some assets (bitmaps and fonts not checked in yet)

## License

No idea yet, all rights reserved for now.
