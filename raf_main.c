#include "raf_compile.h"

#include <stdio.h>
#include <string.h>

static RafCtx G;

int main(int argc, char **argv) {
  raf_ctx_init(&G);
  if (argc < 2 || !strcmp(argv[1], "--help")) {
    puts("Usage: raf_compile <src> [out_base] [O0|O1|O2|O3|Os] [--native]");
    puts("  --native writes the raw encoded instruction stream to <out_base>.bin");
    return 0;
  }

  const char *src = argv[1];
  const char *out = argc > 2 ? argv[2] : "raf_out";
  int do_native = 0;

  for (int i = 3; i < argc; ++i) {
    if (!strcmp(argv[i], "O0")) G.opt = RAF_OPT_0;
    else if (!strcmp(argv[i], "O1")) G.opt = RAF_OPT_1;
    else if (!strcmp(argv[i], "O2")) G.opt = RAF_OPT_2;
    else if (!strcmp(argv[i], "O3")) G.opt = RAF_OPT_3;
    else if (!strcmp(argv[i], "Os")) G.opt = RAF_OPT_S;
    else if (!strcmp(argv[i], "--native")) do_native = 1;
    else {
      fprintf(stderr, "[raf] unknown option: %s\n", argv[i]);
      return 2;
    }
  }

  int rc = raf_compile_file(&G, src, out, do_native);
  if (rc == 0) raf_ctx_report(&G);
  else fprintf(stderr, "[raf] compile error=%d\n", rc);
  return rc;
}
