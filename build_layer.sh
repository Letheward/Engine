# Windows + MinGW
name="win32.o"
src="src/layer/win32.c"
opt="-O3"
def="-D win32_layer_implementation"
etc="-std=c99 -pedantic -Wall"

# build
gcc $src $obj $opt $def $etc -c -o lib/object/$name
