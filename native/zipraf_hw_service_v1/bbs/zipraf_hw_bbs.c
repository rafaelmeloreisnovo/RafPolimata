#include <stdio.h>
#include <string.h>
#include "../include/zipraf_hw_service.h"

#define C0 "\033[0m"
#define CY "\033[36m"
#define GR "\033[32m"
#define YE "\033[33m"
#define MA "\033[35m"

static void logo(void) {
    puts(CY "+----------------------------------------------------------+" C0);
    puts(GR "| ZIPRAF-HW :: RAFPOLIMATA :: CAPABILITY SERVICE V1       |" C0);
    puts(YE "| C11 | Rust no_std | ASM measured-only | FS capabilities |" C0);
    puts(MA "| SOURCE != EXECUTION != EVIDENCE != CLAIM                 |" C0);
    puts(CY "+----------------------------------------------------------+" C0);
}

static void list_crypto(void) {
    zhw_u32 i;
    for (i=1; i < (zhw_u32)ZHW_ALG_COUNT; ++i) {
        const zhw_algorithm_info *a = zhw_algorithm((zhw_algorithm_id)i);
        if (a) printf("%2u  %-20s class=%u status=%u\n", a->id, a->name, a->primitive_class, a->default_status);
    }
}

int main(int argc, char **argv) {
    logo();
    if (argc > 1 && strcmp(argv[1], "crypto") == 0) { list_crypto(); return 0; }
    puts("1) crypto registry");
    puts("2) hardware profile route [adapter to existing RafPolimata hw dispatch]");
    puts("3) fs capability policy [library core]");
    puts("4) receipts / evidence route");
    puts("\nUse: zipraf-hw crypto");
    return 0;
}
