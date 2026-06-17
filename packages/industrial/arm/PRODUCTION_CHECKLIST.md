# ARM Embarcado — Production Checklist RafPolimata

Checklist de producao para ARM embarcado nao-Android: Cortex-A53/A72/A78
(Raspberry Pi, SoC industrial), Cortex-M (bare metal), ARM32 (ARMv7-A).
Arquivos de referencia na raiz: `RAF_021_*.c` a `RAF_040_*.c`.
Cabecalho base: `RAF_rafaelia_common.h`. Harness de benchmark: `Benchmark/raf_bench.h`.

---

## Memory barriers — quando usar cada uma

Arquivos: `RAF_026_memory_barrier_dmb.c`, `RAF_027_memory_barrier_dsb.c`,
`RAF_028_memory_barrier_isb.c`.

### DMB (Data Memory Barrier) — M026

```c
// De RAF_026_memory_barrier_dmb.c:
__asm__ __volatile__("dmb sy" ::: "memory");
```

Use DMB **antes de uma escrita em MMIO que outro core vai ler**.
Garante que todas as leituras/escritas de dados anteriores ao DMB
sejam visiveis para todos os observadores antes da escrita MMIO.

Casos de uso:
- Escrever em registrador de controle DMA antes de ativar o DMA engine.
- Atualizar buffer compartilhado antes de sinalizar produtor/consumidor.
- Antes de `cbs[0].nextconbk = phys_cb1` em M032 quando o DMA controller
  e um observador separado no barramento AXI.

Variantes:
- `dmb sy`  — full system: todos os dominios de sharebility
- `dmb ish` — inner shareable: suficiente para comunicacao entre cores do mesmo cluster
- `dmb oshst` — outer shareable, escritas apenas

### DSB (Data Synchronization Barrier) — M027

```c
// De RAF_027_memory_barrier_dsb.c:
__asm__ __volatile__("dsb sy" ::: "memory");
```

Use DSB **antes de um branch dependente de MMIO** (ex: poll de flag de hardware
que determina qual branch tomar). DSB e mais forte que DMB: garante que todos
os acessos de memoria anteriores foram completados *no barramento*, nao apenas
ordenados.

Casos de uso:
- Antes de ler um registrador de status de DMA que determina se a transferencia
  terminou (evitar leitura especulativa do status).
- Antes de `wfe`/`wfi` em loops de espera de hardware.
- Apos escrever em MMIO e antes de ler o resultado da escrita no mesmo periferico.

### ISB (Instruction Synchronization Barrier) — M028

```c
// De RAF_028_memory_barrier_isb.c:
__asm__ __volatile__("isb" ::: "memory");
```

Use ISB **apos atualizar permissoes ou tabelas de pagina** que afetam o pipeline
de instrucoes. ISB drena o pipeline de instrucoes e forca re-fetch do instruction
stream a partir do PC corrente.

Casos de uso:
- Apos escrever em registradores de sistema que alteram privilegio (CPSR, SPSR).
- Apos atualizar TTBR (translation table base register) antes de acessar
  novo espaco de enderecamento.
- Em Apkc (`Apkc/apkc.c`): apos emitir codigo JIT antes de executar —
  garante que a CPU nao execute instrucoes antigas do icache.

Sequencia padrao para escrita JIT + execucao:
```c
// Escreve codigo em buffer
memcpy(code_buf, arm64_insns, len);

// Limpa dcache (dados) e invalida icache para o range
__builtin___clear_cache(code_buf, code_buf + len);

// ISB: drena pipeline antes de executar o codigo novo
__asm__ __volatile__("isb" ::: "memory");

// Agora seguro executar code_buf como funcao
((void(*)(void))code_buf)();
```

---

## DMA — volatile, enderecos fisicos, producao

Arquivos: `RAF_032_dma_control_block_chain.c`, `RAF_033_dma_circular.c`.

### Regra 1: volatile em DMA control blocks

O DMA controller e um master AXI independente — le e atualiza os campos
do CB diretamente na memoria fisica. O compilador nao sabe disso e pode
reordenar, eliminar ou cachear acessos aos campos do CB.

```c
// CORRETO — volatile garante que cada acesso gera instrucao de load/store:
static volatile rafaelia_dma_cb_t cbs[2] __attribute__((aligned(32)));

// ERRADO — compilador pode eliminar escritas "mortas" ao CB:
static rafaelia_dma_cb_t cbs[2];  // sem volatile
```

### Regra 2: nextconbk deve ser endereco fisico, nao virtual

O DMA controller BCM usa enderecos de barramento (bus addresses), que no
BCM2835/BCM2837 sao enderecos fisicos com bit 30 setado para acesso cache-bypass:
`bus_addr = phys_addr | 0xC0000000` (VC4 SDRAM bus view).

Para obter o endereco fisico de um buffer virtual no Linux:
```c
// Ler /proc/self/pagemap para obter PFN do VMA:
// (Requer /proc acesso — disponivel sem root no Linux 4.0+)
uint64_t pagemap_entry;
int fd = open("/proc/self/pagemap", O_RDONLY);
lseek(fd, (uintptr_t)virt_addr / 4096 * sizeof(uint64_t), SEEK_SET);
read(fd, &pagemap_entry, sizeof(uint64_t));
uint64_t pfn = pagemap_entry & 0x7FFFFFFFFFFFFFULL;
uint64_t phys_addr = (pfn << 12) | ((uintptr_t)virt_addr & 0xFFF);
close(fd);

// Bus address para BCM2837:
uint32_t bus_addr = (uint32_t)(phys_addr | 0xC0000000UL);
cbs[0].nextconbk = bus_addr;  // aponta para CB1 via endereco de barramento
```

### Regra 3: DMA circular (M033)

Para streaming continuo (`RAF_033_dma_circular.c`): o ultimo CB aponta de
volta para o primeiro via `nextconbk`. O DMA roda indefinidamente sem
interrupcao da CPU.

```c
// Chain circular de N CBs:
for (int i = 0; i < N; i++) {
    cbs[i].nextconbk = phys_addr_of_cb[(i + 1) % N];
}
// cbs[N-1].nextconbk aponta para cbs[0] — circular.
```

Cuidado: sem mecanismo de parada, o DMA roda ate que o bit ACTIVE do
registrador DMA_CS seja limpo explicitamente.

---

## GPIO segura: /dev/gpiomem vs /dev/mem

| Metodo | Arquivo | Requer root | Acesso | Risco           |
|--------|---------|------------|--------|-----------------|
| M021   | `RAF_021_gpio_por_mmap.c`    | Sim (sudo) | /dev/mem | Full memory map — acessa toda a RAM fisica |
| M022   | `RAF_022_gpio_por_dev_gpiomem.c` | Nao (grupo gpio) | /dev/gpiomem | Limitado ao bloco GPIO — recomendado |
| M023   | `RAF_023_gpio_por_dev_mem_controlado.c` | Sim | /dev/mem | Acesso controlado com offset especifico |

Recomendacao de producao:
- Usar M022 (`/dev/gpiomem`) sempre que possivel — sem root, sem exposicao
  da RAM do sistema ao processo.
- Usar M021 (`/dev/mem`) apenas quando o offset de periferico nao esta
  disponivel via `/dev/gpiomem` (ex: SPI, I2C, DMA).
- M023 demonstra o padrao de acesso controlado com `offset` explicito para
  limitar o range mapeado.

---

## Thread affinity e prioridade SCHED_FIFO

Arquivos: `RAF_036_afinidade_de_thread_em_linux_android.c`,
`RAF_037_prioridade_de_thread_para_benchmark.c`.

### Afinidade (M036)

```c
cpu_set_t cpuset;
CPU_ZERO(&cpuset);
CPU_SET(2, &cpuset);  // fixar no nucleo 2 (evitar nucleo 0 que recebe IRQs)
sched_setaffinity(0, sizeof(cpuset), &cpuset);
// EPERM se sem CAP_SYS_NICE — TOKEN_VAZIO
```

Para sistemas com `isolcpus=2,3` no kernel cmdline: os nucleos 2 e 3
nao recebem tarefas do scheduler por padrao, reduzindo jitter a < 1 us.
Verificar: `cat /sys/devices/system/cpu/isolated`.

### SCHED_FIFO prio=1 (M037)

```c
// De RAF_037_prioridade_de_thread_para_benchmark.c:
struct sched_param param = { .sched_priority = 1 };
// SCHED_FIFO prio=1 requer CAP_SYS_NICE ou root.
// Em sistemas embarcados dedicados: executar como root ou com capabilities.
int rc = sched_setscheduler(0, SCHED_FIFO, &param);
if (rc == -1) {
    // EPERM — TOKEN_VAZIO: continua com SCHED_OTHER
}
```

Hierarquia de prioridade: SCHED_FIFO > SCHED_RR > SCHED_OTHER.
Para latencia deterministica: SCHED_FIFO prio=1 garante que a thread
nao sera preemptada por outras threads SCHED_OTHER.

### Isolamento de nucleo (M038 — `RAF_038_isolamento_de_nucleo_quando_disponivel.c`)

```bash
# Adicionar ao /boot/cmdline.txt (Raspberry Pi OS):
isolcpus=3 nohz_full=3 rcu_nocbs=3

# Verificar apos reboot:
cat /sys/devices/system/cpu/isolated   # deve mostrar "3"
taskset -c 3 ./benchmark               # executar no nucleo isolado
```

---

## Timer counters — cntvct_el0

Arquivo: `RAF_024_leitura_de_contador_arm64_cntvct_el0.c`,
`RAF_025_uso_de_cntfrq_el0_para_converter_ciclos_em_tempo.c`.

```c
// De raf_bench.h (Benchmark/raf_bench.h):
// ARM64: cntvct_el0 — timer 19.2 MHz, resolucao ~52 ns, sem syscall
static inline uint64_t raf_tsc(void) {
#if defined(__aarch64__)
    uint64_t v;
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(v));
    return v;
#endif
}
```

Condicao para `cntvct_el0` estar disponivel em EL0:
`CNTKCTL_EL1.EL0VCTEN = 1` (bit 1 do registrador CNTKCTL_EL1).

O kernel Linux habilita esse bit por padrao em sistemas Cortex-A com
suporte a VDSO clock. Se o bit for 0, `mrs cntvct_el0` dispara SIGILL.

Verificar em runtime:
```c
// Testar via signal handler antes de usar cntvct_el0:
#include <signal.h>
static volatile int _cntvct_ok = 0;
static void _sigill_handler(int s) { (void)s; _cntvct_ok = -1; }

int check_cntvct(void) {
    struct sigaction sa_new = {0}, sa_old;
    sa_new.sa_handler = _sigill_handler;
    sigaction(SIGILL, &sa_new, &sa_old);
    _cntvct_ok = 1;
    uint64_t v;
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(v));
    sigaction(SIGILL, &sa_old, NULL);
    return _cntvct_ok;  // 1 = disponivel, -1 = SIGILL disparado
}
```

Frequencia do contador: `mrs x0, cntfrq_el0` — tipicamente 19.2 MHz no
BCM2837 (Pi3) e 54 MHz no BCM2711 (Pi4).
Conversao para ns: `ns = (ticks * 1000000000ULL) / cntfrq` (de `raf_bench.h:ticks_to_ns()`).

---

## Benchmark: sempre capturar p95/p99 e jitter

Arquivos: `RAF_039_medicao_de_p95_e_p99_de_latencia.c`,
`RAF_040_medicao_de_jitter_por_amostra.c`.
Harness: `Benchmark/raf_bench.h` (mediana de BENCH_K=31, insertion sort).

Nunca otimizar com base apenas na media ou no minimo. Em sistemas embarcados
com preempcao, IRQs e throttle termico, a distribuicao de latencia e
multi-modal. Metricas de producao:

| Metrica | Coleta | Interpretacao |
|---------|--------|---------------|
| Mediana (p50) | `bench_analyze().med` | Performance nominal |
| p95 | `bench_analyze().p95` | Pior caso em producao normal |
| p99 | `a[29]` em BENCH_K=31 | Pior caso extremo (SLA critico) |
| Jitter (M040) | max(|delta_i - media|) | Variabilidade — determinismo |
| Min | `bench_analyze().min` | Limite fisico do hardware |

```c
// Padrao de coleta de producao (de raf_bench.h + M039/M040):
static uint64_t samp[BENCH_K];
BENCH_RUN(samp, { /* operacao a medir */ });
BenchResult r = bench_analyze(samp);
// r.med = mediana, r.p95 = p95, r.p5 = p5, r.min = minimo, r.max = maximo
```

Antes de qualquer otimizacao baseada em Benchmarks (S11 do projeto):
capturar pelo menos 3 rodadas de `bench_analyze()` com nucleo isolado
(M038) e SCHED_FIFO (M037). Registrar p95/p99 como baseline.

---

## Hash de artefato — SHA-256 em cada deploy (S29)

Antes de cada deploy em producao, registrar o SHA-256 do binario:

```bash
sha256sum ./raf_enterprise_a64 > raf_enterprise_a64.sha256
cat raf_enterprise_a64.sha256
# exemplo: a3f2...  raf_enterprise_a64

# Verificar apos transferencia para o device:
sha256sum -c raf_enterprise_a64.sha256
```

Integrar no pipeline de CI: `.github/workflows/ci.yml` pode adicionar
um step de `sha256sum --check` para validar o binario gerado antes do upload.

A coerencia geometrica do compilador e auditavel via o campo `[phi=... attractor=...]`
impresso apos cada `build_apk()` — rastreabilidade de qual atrator T^7 foi
selecionado pelo `phi_fst()` (de `Apkc/coherence.h`).

---

## Checklist rapido pre-deploy

- [ ] Memory barriers: DMB antes de MMIO write observado por outro master (M026)
- [ ] Memory barriers: DSB antes de branch dependente de leitura MMIO (M027)
- [ ] Memory barriers: ISB apos atualizacao de permissoes ou emissao JIT (M028)
- [ ] DMA: CBs declarados `volatile` com `__attribute__((aligned(32)))`
- [ ] DMA: `nextconbk` contem endereco fisico (bus address), nao virtual
- [ ] GPIO: usar /dev/gpiomem (M022) quando root nao e disponivel
- [ ] Thread: afinidade (M036) e SCHED_FIFO (M037) testados em device real
- [ ] Thread: EPERM em CI e TOKEN_VAZIO — nao bloquear pipeline por isso
- [ ] cntvct_el0: verificar EL0VCTEN=1 antes de usar em producao
- [ ] Benchmark: p95/p99 e jitter coletados antes de qualquer otimizacao (M039/M040)
- [ ] Hash: SHA-256 do binario registrado antes de cada deploy (S29)
- [ ] Sem malloc no hot path — buffers estaticos ou stack conforme CLAUDE.md
- [ ] `clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc
       -ffreestanding -I Apkc Apkc/apkc.c` passa sem erros
