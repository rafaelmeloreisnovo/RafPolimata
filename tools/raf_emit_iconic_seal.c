#include <stdio.h>

int main(void) {
    puts("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"360\" height=\"120\" viewBox=\"0 0 360 120\">");
    puts("<title>RAFAELIA Operational Gate Seal</title>");
    puts("<rect width=\"360\" height=\"120\" rx=\"18\" fill=\"#07111f\"/>");
    puts("<circle cx=\"60\" cy=\"60\" r=\"42\" fill=\"#123f7a\" stroke=\"#38bdf8\" stroke-width=\"4\"/>");
    puts("<path d=\"M60 22 L70 50 L100 50 L76 68 L86 98 L60 80 L34 98 L44 68 L20 50 L50 50 Z\" fill=\"#facc15\"/>");
    puts("<text x=\"120\" y=\"56\" fill=\"#e5f0ff\" font-family=\"monospace\" font-size=\"13\">PASS | FAILSAFE</text>");
    puts("<text x=\"120\" y=\"84\" fill=\"#e5f0ff\" font-family=\"monospace\" font-size=\"13\">FAILOVER | ROLLBACK</text>");
    puts("<text x=\"120\" y=\"106\" fill=\"#94a3b8\" font-family=\"monospace\" font-size=\"10\">local evidence seal - no certification claim</text>");
    puts("</svg>");
    return 0;
}
