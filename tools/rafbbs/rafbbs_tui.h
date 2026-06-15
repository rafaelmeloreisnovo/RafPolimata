#ifndef RAFBBS_TUI_H
#define RAFBBS_TUI_H
#include <stdio.h>
#include "rafbbs_cli.h"
static int raf_tui(void) {
    char line[32];
    int choice = 0;
    RafFilePicker picker;
    raf_filepicker_init(&picker);
    puts("╔════════════════════════════════════════════════════╗");
    puts("║ RAFPOLIMATA BBS OPERATOR CONSOLE                  ║");
    puts("╠══════════════════╦═════════════════════════════════╣");
    puts("║ Arquivos         ║ Rotinas                         ║");
    printf("║ > %-14s ║ 1 Testar encoders               ║\n", picker.selected);
    puts("║   tests/         ║ 2 Assembler roundtrip           ║");
    puts("║   proofs/        ║ 3 Validar APKC                  ║");
    puts("╠══════════════════╩═════════════════════════════════╣");
    puts("║ SYSLOG: escolha uma rotina para iniciar            ║");
    puts("╠════════════════════════════════════════════════════╣");
    puts("║ ENTER=1 | 2/3 Executar | L Listar | F Arquivos | Q ║");
    puts("╚════════════════════════════════════════════════════╝");
    if (!fgets(line, sizeof(line), stdin)) return 0;
    if (line[0] == 'q' || line[0] == 'Q') return 0;
    if (line[0] == 'l' || line[0] == 'L') { raf_list_pipelines(); return 0; }
    if (line[0] == 'f' || line[0] == 'F') { raf_filepicker_print(&picker); return 0; }
    if (line[0] >= '1' && line[0] <= '3') choice = line[0] - '0'; else choice = 1;
    if (choice == 1) return raf_execute_pipeline("encoders");
    if (choice == 2) return raf_execute_pipeline("roundtrip");
    return raf_execute_pipeline("apkc_validate");
}
#endif
