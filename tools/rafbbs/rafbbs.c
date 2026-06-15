#include "rafbbs_tui.h"
int main(int argc, char **argv) {
    if (argc == 1) return raf_tui();
    return raf_cli(argc, argv);
}
