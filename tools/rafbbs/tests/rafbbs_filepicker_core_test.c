#include "rafbbs_filepicker_core.h"

int rafbbs_filepicker_core_test(void)
{
    RafFilePickerCore picker = raf_filepicker_core_init();
    const char *item;

    if (raf_filepicker_core_count() != 5u) return 1;
    if (picker.selected_index != 0u) return 2;

    item = raf_filepicker_core_selected(&picker);
    if (item[0] != 'A' || item[1] != 'p' || item[2] != 'k') return 3;

    if (raf_filepicker_core_select(&picker, 5u) != 4u) return 4;
    if (raf_filepicker_core_is_selected(&picker, 4u) != 1u) return 5;
    if (raf_filepicker_core_is_selected(&picker, 0u) != 0u) return 6;

    item = raf_filepicker_core_selected(&picker);
    if (item[0] != 'p' || item[1] != 'r' || item[2] != 'o') return 7;

    if (raf_filepicker_core_select(&picker, 0u) != 4u) return 8;
    if (raf_filepicker_core_select(&picker, 6u) != 4u) return 9;
    if (raf_filepicker_core_item(5u) != (const char *)0) return 10;

    return 0;
}

#if defined(RAFBBS_FILEPICKER_TEST_MAIN)
int main(void)
{
    return rafbbs_filepicker_core_test();
}
#endif
