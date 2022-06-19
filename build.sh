# Windows + MinGW
name="game"
src="src/main.c"
obj="lib/object/win32.o"
fol="-I lib/header -L lib/static"
lin="-lglfw3 -lglad -lstb_image -lgdi32"
opt="-O0"
#dbg="-g"
#con="-mwindows"
def="-D OS_WINDOWS"
etc="-std=c99 -pedantic -Wall -static"

# build
gcc $src $obj $fol $lin $opt $dbg $con $def $etc -o bin/$name &&

# run
cd bin && ./$name && cd ..
