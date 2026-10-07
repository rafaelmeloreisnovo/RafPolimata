#include "rafbbs_theme.h"

int rafbbs_theme_core_test(void)
{
    const char *pass = raf_status_color(RAF_PASS);
    const char *fail = raf_status_color(RAF_FAIL);
    const char *token = raf_status_color(RAF_TOKEN_VAZIO);
    const char *info = raf_status_color(RAF_INFO);
    const char *name = raf_status_name(RAF_PASS);

    if ((unsigned char)pass[0] != 27u) return 1;
    if (pass[2] != '3' || pass[3] != '2') return 2;
    if ((unsigned char)fail[0] != 27u || fail[3] != '1') return 3;
    if ((unsigned char)token[0] != 27u || token[3] != '5') return 4;
    if (info[0] != 0) return 5;
    if (name[0] != 'P' || name[1] != 'A') return 6;
    return 0;
}

#if defined(RAFBBS_THEME_TEST_MAIN)
int main(void)
{
    return rafbbs_theme_core_test();
}
#endif
