#include "memory.hpp"
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

namespace Memory {
    namespace VMT {
        int pagesize = sysconf(_SC_PAGE_SIZE);
        int pagemask = ~(pagesize-1);

        void* VMT::hook(void* instance, void* hook, int offset) {
            uintptr_t vtable = *((uintptr_t*)instance);
            uintptr_t entry = vtable + sizeof(uintptr_t) * offset;
            uintptr_t original = *((uintptr_t*) entry);

            int originalProtection = mprotect((void*) ((uintptr_t)entry & pagemask), pagesize, PROT_READ|PROT_WRITE|PROT_EXEC);
            *((uintptr_t*)entry) = (uintptr_t)hook;
            mprotect((void*)((uintptr_t)entry & pagemask), pagesize, PROT_READ|PROT_EXEC);

            return (void*)original;
        }
    }
}