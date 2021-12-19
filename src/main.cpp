#include "hooks.hpp"
#include "util/log.hpp"
#include "interfaces.hpp"
#include "features/chams.hpp"
#include "sdk/netvars.hpp"
#include "features/discordrpc.hpp"

#include <dlfcn.h>
#include <thread>
#include <string>
#include <unistd.h>

void mainThread() {
    /* if serverbrowser is not open then wait, (serverbrowser is last to be loaded) */
    while (!dlopen("./bin/linux64/serverbrowser_client.so", RTLD_NOLOAD | RTLD_NOW))
        usleep(500000);
    Interfaces::init();
    Chams::createMaterials();
    Netvars::init();
    Hooks::init();
    LOG("Successfully loaded eclipse!");
}

/* Called on uninject, if you ld_preload with this, then it will call it as soon as you inject, so only have this if PRELOAD compile def is not set */
#ifndef PRELOAD
void __attribute__((destructor)) unload() {
    LOG("Unloading eclipse...");
    Hooks::unload();
    LOG("Unloaded eclipse!");
    DiscordRPC::core->~Core();
}
#endif

/* Called when injected */
int __attribute__((constructor)) main() {
	std::thread thread(mainThread);
    
	thread.detach();
    return 0;
}
