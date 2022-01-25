#!/bin/sh

if [ ! -f /usr/lib/libgamesdk.so ]; then
    echo -e "\033[0;35m[eclipse] gamesdk not installed, installing...\033[0m"
    curl -LO http://www.eclipse.wtf/downloads/libgamesdk.so
    sudo mv libgamesdk.so /usr/lib
fi

echo -e "\033[0;35m[eclipse] downloading latest version of eclipse...\033[0m"
curl -LO http://www.eclipse.wtf/downloads/libeclipse.so

echo -e "\033[0;35m[eclipse] downloading latest version of kubos injector...\033[0m"
curl -LO http://www.eclipse.wtf/downloads/kubos
chmod +x kubos

echo -e "\033[0;35m[eclipse] injecting premium software\033[0m"
sudo cp libeclipse.so /usr/lib/libMangoHud.so
sudo ./kubos -p $(pidof csgo_linux64) /usr/lib/libMangoHud.so
