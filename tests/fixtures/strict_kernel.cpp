#include <stdint.h>
#include <stddef.h>
#include <string.h>

RAF_EXPORT uint32_t raf_cpp_patch(uint32_t current, uint32_t value, uint32_t mask) {
    char probe[8];
    memset(probe, 0, sizeof(probe));
    strncpy(probe, "ok", sizeof(probe));
    return strlen(probe) == 2u ? current ^ ((current ^ value) & mask) : 0u;
}

RAF_EXPORT void ANativeActivity_onCreate(void *activity, void *saved_state, size_t saved_state_size) {
    (void)activity;
    (void)saved_state;
    (void)saved_state_size;
}

RAF_EXPORT int android_main(void *app) {
    (void)app;
    return (int)raf_cpp_patch(0xaau, 0x55u, 0x0fu);
}
