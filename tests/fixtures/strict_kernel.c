#include <stdint.h>
#include <string.h>
#include <stdio.h>

__attribute__((visibility("default")))
uint32_t raf_patch(uint32_t current, uint32_t value, uint32_t mask) {
    char probe[4];
    memset(probe, 0, sizeof(probe));
    if (strlen(probe) != 0u) return 0u;
    return current ^ ((current ^ value) & mask);
}

__attribute__((visibility("default")))
void ANativeActivity_onCreate(void *activity, void *saved_state, unsigned long saved_state_size) {
    (void)activity;
    (void)saved_state;
    (void)saved_state_size;
}

__attribute__((visibility("default")))
int android_main(void *app) {
    (void)app;
    return (int)raf_patch(0xaau, 0x55u, 0x0fu);
}
