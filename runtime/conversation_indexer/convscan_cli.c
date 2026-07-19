#include "raf_convscan.h"
#include <stdio.h>

int main(int argc, char **argv) {
    FILE *fp;
    rcs_ctx ctx;
    rcs_stats s;
    unsigned char buffer[1024u * 1024u];
    size_t n;
    int rc;

    if (argc != 2) {
        fprintf(stderr, "usage: %s conversations.json\n", argv[0]);
        return 64;
    }
    fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 66;
    }
    rcs_init(&ctx);
    while ((n = fread(buffer, 1u, sizeof(buffer), fp)) != 0u) {
        rc = rcs_feed(&ctx, buffer, (rcs_u32)n);
        if (rc != RCS_OK) {
            fprintf(stderr, "scan error=%d byte=%llu\n", rc, (unsigned long long)ctx.stats.bytes);
            fclose(fp);
            return 65;
        }
    }
    if (ferror(fp)) {
        perror("fread");
        fclose(fp);
        return 74;
    }
    fclose(fp);
    rc = rcs_finish(&ctx, &s);
    if (rc != RCS_OK) {
        fprintf(stderr, "finish error=%d byte=%llu\n", rc, (unsigned long long)ctx.stats.bytes);
        return 65;
    }
    printf("{\n");
    printf("  \"state\": \"VERIFIED\",\n");
    printf("  \"bytes\": %llu,\n", (unsigned long long)s.bytes);
    printf("  \"crc32c\": \"%08x\",\n", s.crc32c);
    printf("  \"objects\": %llu,\n", (unsigned long long)s.objects);
    printf("  \"arrays\": %llu,\n", (unsigned long long)s.arrays);
    printf("  \"strings\": %llu,\n", (unsigned long long)s.strings);
    printf("  \"numbers\": %llu,\n", (unsigned long long)s.numbers);
    printf("  \"max_depth\": %u,\n", s.max_depth);
    printf("  \"keys\": {\n");
    printf("    \"id\": %llu,\n", (unsigned long long)s.conversation_id_keys);
    printf("    \"title\": %llu,\n", (unsigned long long)s.title_keys);
    printf("    \"create_time\": %llu,\n", (unsigned long long)s.create_time_keys);
    printf("    \"update_time\": %llu,\n", (unsigned long long)s.update_time_keys);
    printf("    \"mapping\": %llu,\n", (unsigned long long)s.mapping_keys);
    printf("    \"message\": %llu\n", (unsigned long long)s.message_keys);
    printf("  }\n");
    printf("}\n");
    return 0;
}
