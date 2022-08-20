#include "memory.hpp"
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

namespace Memory {
    namespace VMT {
        int pagesize = sysconf(_SC_PAGE_SIZE);
        int pagemask = ~(pagesize-1);

        void* hook(void* instance, void* hook, int offset) {
            uintptr_t vtable = *((uintptr_t*)instance);
            uintptr_t entry = vtable + sizeof(uintptr_t) * offset;
            uintptr_t original = *((uintptr_t*) entry);

            mprotect((void*) ((uintptr_t)entry & pagemask), pagesize, PROT_READ|PROT_WRITE|PROT_EXEC);
            *((uintptr_t*)entry) = (uintptr_t)hook;
            mprotect((void*)((uintptr_t)entry & pagemask), pagesize, PROT_READ|PROT_EXEC);

            return (void*)original;
        }

        void detour(char* src, char* dest) {
            mprotect((void*)((uintptr_t)src & pagemask), pagesize, PROT_READ|PROT_WRITE|PROT_EXEC);
            intptr_t  relativeAddress = (intptr_t)(dest - (intptr_t)src) - 5;
            *src = (char)'\xE9';
            *(intptr_t*)((intptr_t)src + 1) = relativeAddress;
            mprotect((void*)((uintptr_t)src & pagemask), pagesize, PROT_READ|PROT_EXEC);
        }
    }
    namespace Johannes {
        void* findUnusedMemory(void* begin) {
            unsigned int pagesize = getpagesize();
            unsigned long currentPage = reinterpret_cast<size_t>(begin) / pagesize;
            unsigned long maxOffset = /*FRAMEWORK_TWO_GB*/0x7ffffff0 / pagesize;
            unsigned long offset = 0;
            while (offset < maxOffset) {
                for (int sign = -1; sign <= (offset != 0 ? 1 : -1); sign += 2) {
                    if (sign < 0 && offset > currentPage) continue;
                    unsigned long page = currentPage + offset * sign;
                    void* pointer = mmap(reinterpret_cast<void*>(page * pagesize), pagesize, PROT_READ | PROT_WRITE | PROT_EXEC,
                                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, 0, 0);
                    if (pointer != MAP_FAILED) return pointer;
                }
                offset++;
            }
            return nullptr;
        }
        void writeAbsJmp(void* addr, void* goal) {
            unsigned char absJumpInstructions[] = {
                 0x49, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // mov r15, goal
                 0x41, 0xFF, 0xE7                                             // jmp r15
            };
            memcpy(absJumpInstructions + 2, &goal, sizeof(void*));
            memcpy(addr, absJumpInstructions, /*FRAMEWORK_ABS_JMP_LENGTH*/13);
        }
        void writeNearJmp(void* addr, void* goal) {
            unsigned char jmpInstruction[] = {
                 0xE9, 0x0, 0x0, 0x0, 0x0  // jmp goal
            };
            // Calculation for a relative jmp
            void* jmpTarget = reinterpret_cast<void*>(
                 static_cast<char*>(goal) -
                 (static_cast<char*>(addr) +
                  /*FRAMEWORK_NEAR_JMP_LENGTH*/5));  // Jumps always start at the rip, which has already increased
            memcpy(jmpInstruction + 1, &jmpTarget, sizeof(jmpInstruction) - 1 /* E9 */);
            memcpy(addr, jmpInstruction, sizeof(jmpInstruction));
        }
        void protect(void* addr, int prot) {
            unsigned long pagesize = sysconf(_SC_PAGESIZE);
            void* aligned =
                 reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(addr) + pagesize - 1) & ~(pagesize - 1)) - pagesize;
            mprotect(aligned, pagesize, prot);
        }
        void* detour(void* original, void* hook, int instructionLength) {
            if (/*FRAMEWORK_NEAR_JMP_LENGTH*/5 > instructionLength) return nullptr;
            void* unusedMemory = findUnusedMemory(original);
            if (unusedMemory) {
                // Jmp into our code
                writeAbsJmp(unusedMemory, hook);
                // Copy the stolen bytes
                memcpy(static_cast<char*>(unusedMemory) + /*FRAMEWORK_ABS_JMP_LENGTH*/13, original, instructionLength);
                // Write the jmp back into the original code
                writeNearJmp(static_cast<char*>(unusedMemory) + /*FRAMEWORK_ABS_JMP_LENGTH*/13 + instructionLength,
                                       static_cast<char*>(original) + /*FRAMEWORK_NEAR_JMP_LENGTH*/5);
                protect(unusedMemory, PROT_READ | PROT_EXEC);

                protect(original, PROT_READ | PROT_WRITE | PROT_EXEC);
                // Write near jmp into the code, we are hooking
                writeNearJmp(original, unusedMemory);
                /*
                 * In case we copied too many instructions, we can clean up the code by replacing the orphaned bytes with NOPs
                 * This is purely optional, but maybe some debugger goes crazy
                 * On the other side, anticheats shouldn't care about it either, they ban you
                 * as soon as they see the jmp instruction at the beginning, they ban you anyway
                 */
                memset(static_cast<char*>(original) + /*FRAMEWORK_NEAR_JMP_LENGTH*/5, 0x90 /* NOP */,
                       instructionLength - /*FRAMEWORK_NEAR_JMP_LENGTH*/5);
                protect(original, PROT_READ | PROT_EXEC);

                // This is the point after the abs jmp, means right where the stolen bytes begin
                return static_cast<char*>(unusedMemory) + /*FRAMEWORK_ABS_JMP_LENGTH*/13;
            } else {
                return nullptr;
            }
        }
    }  // namespace Johannes
}  // namespace Memory