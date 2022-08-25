#!/bin/bash

gdb="$(dirname "$0")/gdb" # For using a gdb build such as the cathook one (The one included)
libname="libMangoHud.so" # Pretend to be gamemode, change this to another lib that may be in /maps (if already using real gamemode, I'd suggest using libMangoHud.so)
csgo_pid=$(pidof csgo_linux64)

# Set to true to compile with clang. (if changing to true, make sure to delete the build directory from gcc)
export USE_CLANG="true"

if [[ $EUID -eq 0 ]]; then
    echo "You cannot run this as root." 
    exit 1
fi

rm -rf /tmp/dumps
mkdir -p --mode=000 /tmp/dumps

function unload {
    echo "Unloading cheat..."
    echo "2" | sudo tee /proc/sys/kernel/yama/ptrace_scope
    if grep -q "$libname" "/proc/$csgo_pid/maps"; then
        $gdb -n -q -batch -ex "attach $csgo_pid" \
            -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
            -ex "set \$dlclose = (int(*)(void*)) dlclose" \
            -ex "set \$library = \$dlopen(\"/usr/lib/$libname\", 6)" \
            -ex "call \$dlclose(\$library)" \
            -ex "call \$dlclose(\$library)" \
            -ex "detach" \
            -ex "quit"
    fi
    echo "Unloaded!"
}

function load {
    echo "Loading cheat..."
    # https://www.kernel.org/doc/Documentation/security/Yama.txt
    echo "2" | sudo tee /proc/sys/kernel/yama/ptrace_scope # Only allows root to inject code. This is temporary until reboot.
    sudo cp build/libeclipse.so /usr/lib/$libname
    sudo strip -s -v /usr/lib/$libname
    sudo patchelf --set-soname $libname /usr/lib/$libname
    sudo $gdb -n -q -batch \
    -ex "set auto-load safe-path /usr/lib/" \
    -ex "attach $csgo_pid" \
    -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
    -ex "call \$dlopen(\"/usr/lib/$libname\", 1)" \
    -ex "detach" \
    -ex "quit" 2
}

function load_debug {
    echo "Loading cheat..."
    # https://www.kernel.org/doc/Documentation/security/Yama.txt
    echo "2" | sudo tee /proc/sys/kernel/yama/ptrace_scope # Only allows root to inject code. This is temporary until reboot.
    sudo cp build/libeclipse.so /usr/lib/$libname
    sudo patchelf --set-soname $libname /usr/lib/$libname
    sudo $gdb -n -q -batch \
        -ex "set auto-load safe-path /usr/lib:/usr/lib/" \
        -ex "attach $csgo_pid" \
        -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
        -ex "call \$dlopen(\"/usr/lib/$libname\", 1)" \
        -ex "detach" \
        -ex "quit"
    sudo $gdb -p "$csgo_pid"
}

function build {
    echo "Building cheat..."
    mkdir -p build
    cd build
    cmake -D CMAKE_BUILD_TYPE=Release ..
    make -j $(nproc --all)
    cd ..
    sudo cp build/libeclipse.so /usr/lib/$libname
    sudo patchelf --set-soname $libname /usr/lib/$libname
}

function build_debug {
    echo "Building cheat... (debug)"
    mkdir -p build
    cd build
    cmake -D CMAKE_BUILD_TYPE=Debug ..
    make -j $(nproc --all)
    cd ..
}

function pull {
    git pull
}

function clean {
    rm -rf build
    rm /usr/lib/$libname
}

while [[ $# -gt 0 ]]
do
keys="$1"
case $keys in
    -u|--unload)
        unload
        shift
        ;;
    -l|--load)
        load
        shift
        ;;
    -ld|--load_debug)
        load_debug
        shift
        ;;
    -b|--build)
        build
        shift
        ;;
    -bd|--build_debug)
        build_debug
        shift
        ;;
    -p|--pull)
        pull
        shift
        ;;
    -c|--clean)
        clean
        shift
        ;;
    -r|--release)
        cp build/libeclipse.so release/server/
        shift
        ;;
    -h|--help)
        echo "
 help
Toolbox script for the beste lincuck cheat 2021
=======================================================================
| Argument             | Description                                  |
| -------------------- | -------------------------------------------- |
| -u (--unload)        | Unload the cheat from CS:GO if loaded.       |
| -l (--load)          | Load/inject the cheat via gdb.               |
| -ld (--load_debug)   | Load/inject the cheat and debug via gdb.     |
| -b (--build)         | Build to the build/ dir.                     |
| -bd (--build_debug)  | Build to the build/ dir as debug.            |
| -p (--pull)          | Update the cheat.                            |
| -c (--clean)         | Clean build.                                 |
| -h (--help)          | Show help.                                   |
=======================================================================

All args are executed in the order they are written in, for
example, \"-p -u -b -l\" would update the cheat, then unload, then build it, and
then load it back into csgo.
"
        exit
        ;;
    *)
        echo "Unknown arg $1, use -h for help"
        exit
        ;;
esac
done
