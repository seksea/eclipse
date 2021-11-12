#include "hooks.hpp"

#include <dlfcn.h>
#include <thread>
#include <string>
#include <unistd.h>

void mainThread() {
    /* if serverbrowser is not open then wait, (serverbrowser is last to be loaded) */
    while (!dlopen("./bin/linux64/serverbrowser_client.so", RTLD_NOLOAD | RTLD_NOW))
        usleep(500000);
    Hooks::init();
    puts("\e[32mSuccessfully loaded csgo-cheat\e[0m\n");
}

/* Called on uninject, if you ld_preload with this, then it will call it as soon as you inject, so only have this if PRELOAD compile def is not set */
#ifndef PRELOAD
void __attribute__((destructor)) unload() {
    Hooks::unload();
    puts("\e[32mUnloading csgo-cheat...\e[0m\n");
}
#endif

/* Called when injected */
int __attribute__((constructor)) main() {
	std::thread thread(mainThread);
    
	thread.detach();
    return 0;
}
