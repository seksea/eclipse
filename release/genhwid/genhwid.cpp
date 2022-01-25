#include <stdio.h>
#include <cpuid.h>
#include <cstdint>

uint64_t __attribute__ ((always_inline)) gethwid() {
    unsigned int a, b, model1, model2;
    __get_cpuid(0, &a, &b, &model1, &model2);

    uint64_t model164 = model1;
    
    return ((model164 << 32) +
            model2 + a + b) ^ 0x4e7a8f34c219b3ac;
}

int main() {
    printf("your hwid: %lx (give to sekc)\n", gethwid());
}