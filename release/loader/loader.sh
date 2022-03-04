#!/bin/sh

rm -rf /tmp/dumps
mkdir -p --mode=000 /tmp/dumps
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope


if [[ $1 == "unload" ]]; then
    echo -e "\033[0;35m[eclipse] downloading latest version of gdb...\033[0m"
    curl -LO http://eclipse.wtf/downloads/gdb
    chmod +x gdb

    if grep -q "libMangoHud.so" "/proc/$(pidof csgo_linux64)/maps"; then
        ./gdb -n -q -batch \
            -ex "attach $(pidof csgo_linux64)" \
            -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
            -ex "set \$dlclose = (int(*)(void*)) dlclose" \
            -ex "set \$library = \$dlopen(\"/usr/lib/libMangoHud.so\", 6)" \
            -ex "call \$dlclose(\$library)" \
            -ex "call \$dlclose(\$library)" \
            -ex "detach" \
            -ex "quit"
    fi
    exit 0
fi


if [ ! -f /usr/lib/libgamesdk.so ]; then
    echo -e "\033[0;35m[eclipse] gamesdk not installed, installing...\033[0m"
    curl -LO http://eclipse.wtf/downloads/libgamesdk.so
    sudo mv libgamesdk.so /usr/lib
fi

echo -e "\033[0;35m[eclipse] downloading latest version of eclipse...\033[0m"
curl -LO http://eclipse.wtf/downloads/libeclipse.so
    sudo mv libeclipse.so /usr/lib/libMangoHud.so


if [[ $1 == "kubos" ]]; then
    echo -e "\033[0;35m[eclipse] downloading latest version of kubos injector...\033[0m"
    curl -LO http://eclipse.wtf/downloads/kubos
    chmod +x kubos

    echo -e "\033[0;35m[eclipse] injecting premium software\033[0m"
    sudo ./kubos $(pidof csgo_linux64) /usr/lib/libMangoHud.so
else
    echo -e "\033[0;35m[eclipse] downloading latest version of gdb...\033[0m"
    curl -LO http://eclipse.wtf/downloads/gdb
    chmod +x gdb

    ./gdb -n -q -batch \
    -ex "set auto-load safe-path /usr/lib/" \
    -ex "attach $(pidof csgo_linux64)" \
    -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" \
    -ex "call \$dlopen(\"/usr/lib/libMangoHud.so\", 1)" \
    -ex "detach" \
    -ex "quit"
fi