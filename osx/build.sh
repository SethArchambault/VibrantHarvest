set -e
trap 'echo "\"${BASH_COMMAND}\" command filed with exit code $?."' EXIT
clear 
cc -o project project.c -I./opt/raylib/src  -framework OpenGL -framework OpenAL -framework Cocoa -framework IOKit -framework CoreVideo /opt/raylib/release/libs/osx/libraylib.a
./project
