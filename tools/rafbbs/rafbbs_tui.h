#ifndef RAFBBS_TUI_H
#define RAFBBS_TUI_H
#include <stdio.h>
#include "rafbbs_cli.h"
#include "rafbbs_tui_core.h"
static int raf_tui(void) {
    char line[32];
    RafTuiRoute route;
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
    route = raf_tui_route_char(line[0]);
    if (route.action == RAF_TUI_QUIT) return 0;
    if (route.action == RAF_TUI_LIST) { raf_list_pipelines(); return 0; }
    if (route.action == RAF_TUI_FILES) { raf_filepicker_print(&picker); return 0; }
    return raf_execute_pipeline(route.pipeline_id);
}
#endif
