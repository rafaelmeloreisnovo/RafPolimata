#include "raf_compile.h"

#include <stdio.h>
#include <string.h>

static RafCtx G;

int main(int argc, char **argv) {
  raf_ctx_init(&G);
  if (argc < 2 || !strcmp(argv[1], "--help")) {
    puts("Usage: raf_compile <src> [out_base] [O0|O1|O2|O3|Os] [--native]");
    return 0;
  }
  const char *src = argv[1];
  const char *out = argc > 2 ? argv[2] : "raf_out";
  for (int i = 3; i < argc; ++i) {
    if (!strcmp(argv[i], "O0")) G.opt = RAF_OPT_0;
    else if (!strcmp(argv[i], "O1")) G.opt = RAF_OPT_1;
    else if (!strcmp(argv[i], "O2")) G.opt = RAF_OPT_2;
    else if (!strcmp(argv[i], "O3")) G.opt = RAF_OPT_3;
    else if (!strcmp(argv[i], "Os")) G.opt = RAF_OPT_S;
  }
  int rc = raf_compile_file(&G, src, out, 0);
  if (rc == 0) raf_ctx_report(&G);
  else fprintf(stderr, "[raf] compile error=%d\n", rc);
  return rc;
}
