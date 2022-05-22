# Windows + MinGW
name="game"
src="src/main.c"
dep="-I lib/hdr -L lib/"
lin="-lglfw3 -lglad -lstb_image -lgdi32"
opt="-O0"
#con="-mwindows"
etc="-std=c99 -Wall -pedantic -static"

# build
gcc $src $dep $lin $opt $dbg $etc -o bin/$name &&

# run
cd bin && ./$name && cd ..
