# TraceBuf follow-up scope

> **Entrada canônica:** docs/AGENTES.md §7 (CI gates — gate "RAF TraceBuf freestanding smoke gate" em `.github/workflows/ci.yml`) e §4 (regras de não-colisão — freestanding, sem heap, sem transporte externo embutido). Este marcador registra o escopo do follow-up de TraceBuf pós-PR #93.

Este marcador registra que os commits posteriores ao merge inicial do PR #93 adicionam:

- `Benchmark/raf_trace_sink.h`
- inclusao do sink no agregador `Benchmark/raf_tracebuf.h`
- `tools/raf_tracebuf_smoke.c`
- gate CI `RAF TraceBuf freestanding smoke gate`
- complemento documental em `docs/RAF_TRACEBUF_C_ASM_HEX.md`

Escopo: eventos locais do proprio processo, sem transporte externo embutido e sem captura de processos externos.
