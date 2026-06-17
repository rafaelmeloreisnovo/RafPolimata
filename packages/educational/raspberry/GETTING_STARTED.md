# Raspberry Pi — Getting Started com RafPolimata M021-M035

Guia para usar os metodos M021-M035 no Raspberry Pi (BCM2835/BCM2837/BCM2711).
Os arquivos fonte de referencia estao na raiz do repositorio (`RAF_021_*.c`
ate `RAF_035_*.c`) e em `RAF_rafaelia_common.h`.

## Pre-requisitos

- Raspberry Pi 2/3/4 com Raspberry Pi OS (Lite ou Desktop) ou Ubuntu Server
- gcc 8+ ou clang 7+ instalado (`sudo apt install build-essential`)
- Para GPIO via `/dev/mem`: `sudo` ou usuario no grupo `gpio` + `root`
- Para GPIO via `/dev/gpiomem`: usuario no grupo `gpio` (sem root)
  ```bash
  sudo usermod -aG gpio $USER && newgrp gpio
  ```
- Cabecalhos da kernel para /dev/gpiomem: nenhum — acesso via mmap POSIX
- Para SPI BCM (M029): habilitar SPI0 no raspi-config ou em `/boot/config.txt`:
  ```
  dtparam=spi=on
  ```

---

## Nota sobre TOKEN_VAZIO

Os metodos M021-M035 retornam `0` (sem erro) quando o hardware-alvo nao
esta presente. Isso e o padrao TOKEN_VAZIO do projeto: a funcao "vazia" e
sintaticamente e semanticamente correta, mas nao acessa hardware ausente.

Exemplos de TOKEN_VAZIO em ambiente nao-RPi:
- `rafaelia_m021_gpio_por_mmap()` retorna 0 se `/dev/mem` nao existe ou
  `mmap` falha (nao e um Pi).
- `rafaelia_m029_spi_por_registrador_bcm(NULL)` retorna 0 se `mmio_base == NULL`.
- `rafaelia_m036_afinidade_de_thread_em_linux_android()` retorna 0 se
  `sched_setaffinity` retorna EPERM (container, CI sem privilegio).

Veja `docs/TOKEN_VAZIO_PARABOLAS_MESTRES.md` para o protocolo completo.

---

## Mapa de registradores BCM2835/BCM2837

| Periferico | Base fisica (BCM2837/Pi3) | Base fisica (BCM2835/Pi1/2) |
|-----------|--------------------------|------------------------------|
| GPIO      | 0x3F200000               | 0x20200000                   |
| SPI0      | 0x3F204000               | 0x20204000                   |
| DMA       | 0x3F007000               | 0x20007000                   |
| PWM       | 0x3F20C000               | 0x2020C000                   |
| Clock Mgr | 0x3F101000               | 0x20101000                   |
| I2C BSC0  | 0x3F205000               | 0x20205000                   |

Para BCM2711 (Pi4): base peripheral = 0xFE000000.
Detectar em runtime: ler `/proc/device-tree/soc/ranges` ou comparar
o campo SoC do `/proc/cpuinfo`.

---

## Exemplo comentado: GPIO via mmap (M021)

Arquivo de referencia: `RAF_021_gpio_por_mmap.c`

Objetivo: mapear o bloco de registradores GPIO no espaco de enderecamento
do processo via `/dev/mem` e `mmap(2)`. Permite acesso a qualquer GPIO sem
biblioteca externa, com latencia de acesso equivalente a uma escrita em
memoria do processo (sem syscall no hot path apos o mmap inicial).

Gated em `__linux__ && !__ANDROID__` em `RAF_021_gpio_por_mmap.c`.

```c
// Equivale a rafaelia_m021_gpio_por_mmap — versao comentada expandida
#include <fcntl.h>       // open(2)
#include <sys/mman.h>    // mmap(2), munmap(2)
#include <unistd.h>      // close(2)
#include <stdint.h>

// Enderecos BCM2837 (Pi 2/3)
#define BCM_GPIO_BASE   0x3F200000UL
#define GPIO_BLOCK_SIZE 0x1000u     // 4 KB cobre todos os registradores GPIO

// Offsets em words (uint32_t):
// GPFSEL0-5: 0x00-0x14  — funcao de pino (INPUT/OUTPUT/ALT)
// GPSET0:    0x1C        — setar pino (escreve 1 para HIGH)
// GPCLR0:    0x28        — limpar pino (escreve 1 para LOW)
// GPLEV0:    0x34        — ler nivel atual

static volatile uint32_t *gpio_map = NULL;

int gpio_init(void) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) return 0;  // TOKEN_VAZIO — sem /dev/mem ou sem permissao

    gpio_map = mmap(
        NULL,
        GPIO_BLOCK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        (off_t)BCM_GPIO_BASE
    );
    close(fd);

    if (gpio_map == MAP_FAILED) {
        gpio_map = NULL;
        return 0;  // TOKEN_VAZIO — nao e um Pi ou sem BCM no endereco
    }
    return 1;  // hardware real encontrado
}

// Configura pino como OUTPUT (GPFSEL: 3 bits por pino, valor 001 = output)
void gpio_set_output(unsigned pin) {
    unsigned reg  = pin / 10u;
    unsigned shift = (pin % 10u) * 3u;
    gpio_map[reg] = (gpio_map[reg] & ~(7u << shift)) | (1u << shift);
}

// Seta pino HIGH — escrita em GPSET0 (offset 0x1C / 4 = 7)
void gpio_set(unsigned pin) {
    gpio_map[7u] = (1u << pin);   // GPSET0
}

// Limpa pino LOW — escrita em GPCLR0 (offset 0x28 / 4 = 10)
void gpio_clear(unsigned pin) {
    gpio_map[10u] = (1u << pin);  // GPCLR0
}

// Le nivel — GPLEV0 (offset 0x34 / 4 = 13)
int gpio_read(unsigned pin) {
    return (gpio_map[13u] >> pin) & 1u;
}

void gpio_cleanup(void) {
    if (gpio_map) munmap((void *)gpio_map, GPIO_BLOCK_SIZE);
    gpio_map = NULL;
}

int main(void) {
    if (!gpio_init()) return 0;   // TOKEN_VAZIO — nao e Pi

    gpio_set_output(18);          // BCM GPIO 18 = pino fisico 12
    gpio_set(18);                 // HIGH
    // ...delay via M024 cntvct_el0 ou nanosleep...
    gpio_clear(18);               // LOW
    gpio_cleanup();
    return 0;
}
```

Compilacao e execucao:
```bash
gcc -std=c11 -O2 RAF_021_gpio_por_mmap.c -I /path/to/RafPolimata -o gpio_test
sudo ./gpio_test
# sudo necessario para /dev/mem; usar /dev/gpiomem (M022) para evitar root
```

Alternativa sem root — usar M022 (`RAF_022_gpio_por_dev_gpiomem.c`):
```bash
gcc -std=c11 -O2 RAF_022_gpio_por_dev_gpiomem.c -I /path/to/RafPolimata \
  -o gpio_test_safe
./gpio_test_safe   # sem sudo, usuario deve estar no grupo gpio
```

---

## Exemplo comentado: SPI por registrador BCM (M029)

Arquivo de referencia: `RAF_029_spi_por_registrador_bcm.c`

Pre-requisito: SPI0 habilitado em `/boot/config.txt` com `dtparam=spi=on`
e reboot. Verificar:
```bash
ls /dev/spidev0.*   # deve listar spidev0.0 e spidev0.1
```

M029 acessa SPI0 diretamente via MMIO (base 0x3F204000), sem usar
`/dev/spidev` ou `spidev_ioctl`. Latencia de setup: 1x mmap. Hot path:
3 leituras + 2 escritas em registrador volatil por byte transferido.

```c
// Equivale a rafaelia_m029_spi_por_registrador_bcm — versao comentada
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define BCM_SPI0_BASE   0x3F204000UL
#define SPI_BLOCK_SIZE  0x1000u

// Offsets em words (de RAF_029_spi_por_registrador_bcm.c):
#define SPI0_CS_OFF    0u   // Control/Status
#define SPI0_FIFO_OFF  1u   // TX/RX FIFO
#define SPI0_CLK_OFF   2u   // Clock Divider

// Bits CS (de RAF_029_spi_por_registrador_bcm.c):
#define SPI0_CS_TA       (1u << 7)    // Transfer Active
#define SPI0_CS_DONE     (1u << 16)   // Transfer Done
#define SPI0_CS_TXD      (1u << 18)   // TX FIFO tem espaco
#define SPI0_CS_CLEAR_RX (1u << 5)
#define SPI0_CS_CLEAR_TX (1u << 4)

static volatile uint32_t *spi0 = NULL;

int spi_init(void) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) return 0;  // TOKEN_VAZIO
    spi0 = mmap(NULL, SPI_BLOCK_SIZE, PROT_READ | PROT_WRITE,
                MAP_SHARED, fd, (off_t)BCM_SPI0_BASE);
    close(fd);
    if (spi0 == MAP_FAILED) { spi0 = NULL; return 0; }

    // Clock divider = 250 => SPI clock = 250 MHz / 250 = 1 MHz
    spi0[SPI0_CLK_OFF] = 250u;
    return 1;
}

// Transferencia full-duplex de um byte (identico a rafaelia_m029_spi_xfer)
uint8_t spi_xfer(uint8_t tx) {
    // Limpa FIFOs e ativa transferencia
    spi0[SPI0_CS_OFF] = SPI0_CS_CLEAR_RX | SPI0_CS_CLEAR_TX;
    spi0[SPI0_CS_OFF] = SPI0_CS_TA;

    while (!(spi0[SPI0_CS_OFF] & SPI0_CS_TXD));  // aguarda TX pronto
    spi0[SPI0_FIFO_OFF] = (uint32_t)tx;           // escreve byte

    while (!(spi0[SPI0_CS_OFF] & SPI0_CS_DONE));  // aguarda fim
    uint8_t rx = (uint8_t)(spi0[SPI0_FIFO_OFF] & 0xFFu);  // le RX

    spi0[SPI0_CS_OFF] &= ~(uint32_t)SPI0_CS_TA;  // desativa
    return rx;
}

int main(void) {
    if (!spi_init()) return 0;  // TOKEN_VAZIO — nao e Pi ou SPI desabilitado
    uint8_t rx = spi_xfer(0xAA);
    (void)rx;
    munmap((void *)spi0, SPI_BLOCK_SIZE);
    return 0;
}
```

```bash
gcc -std=c11 -O2 RAF_029_spi_por_registrador_bcm.c \
  -I /path/to/RafPolimata -o spi_test
sudo ./spi_test
```

---

## Exemplo comentado: DMA chain (M032)

Arquivo de referencia: `RAF_032_dma_control_block_chain.c`

O struct `rafaelia_dma_cb_t` (definido em `RAF_032_dma_control_block_chain.c`)
e de 32 bytes (256 bits) e deve ser alinhado a 256 bits em uso real.
O campo `nextconbk` deve conter o endereco de barramento (fisico), nao virtual.

```c
// Uso do DMA chain — extraido e comentado de RAF_032_dma_control_block_chain.c
#include <stdint.h>

// Flags TI (Transfer Information) — de RAF_032_dma_control_block_chain.c:
#define DMA_TI_SRC_INC   (1u << 8)   // auto-incrementa endereco fonte
#define DMA_TI_DEST_INC  (1u << 4)   // auto-incrementa endereco destino
#define DMA_TI_WAIT_RESP (1u << 3)   // aguarda AXI write response

typedef struct {
    uint32_t ti;          // Transfer Information
    uint32_t source_ad;   // Endereco de barramento da fonte (RUNTIME — fisico)
    uint32_t dest_ad;     // Endereco de barramento do destino (RUNTIME — fisico)
    uint32_t txfr_len;    // Tamanho da transferencia em bytes
    uint32_t stride;      // Stride 2D (0 para 1D linear)
    uint32_t nextconbk;   // Endereco do proximo CB (0 = fim de chain)
    uint32_t _pad[2];     // Reservado — deve ser 0
} dma_cb_t;

// IMPORTANTE: usar volatile para CBs — o DMA controller le/escreve esses campos.
// nextconbk deve ser o endereco fisico do CB seguinte, nao o virtual.
// Em producao, obter fisico via /proc/self/pagemap ou DMA-BUF kernel API.

// Chain de 2 CBs: copia 16 bytes, encadeia para CB1, CB1 = fim.
static volatile dma_cb_t cbs[2] __attribute__((aligned(32)));

void dma_chain_build(uint32_t phys_src, uint32_t phys_dst,
                     uint32_t phys_cb1_addr) {
    // CB0: copia 16 bytes de phys_src para phys_dst
    cbs[0].ti        = DMA_TI_SRC_INC | DMA_TI_DEST_INC | DMA_TI_WAIT_RESP;
    cbs[0].source_ad = phys_src;
    cbs[0].dest_ad   = phys_dst;
    cbs[0].txfr_len  = 16u;
    cbs[0].stride    = 0u;
    cbs[0].nextconbk = phys_cb1_addr;   // RUNTIME — endereco fisico de CB1
    cbs[0]._pad[0]   = 0u;
    cbs[0]._pad[1]   = 0u;

    // CB1: marcador de fim de chain (nextconbk = 0)
    cbs[1].ti        = DMA_TI_SRC_INC | DMA_TI_DEST_INC;
    cbs[1].source_ad = 0u;
    cbs[1].dest_ad   = 0u;
    cbs[1].txfr_len  = 0u;
    cbs[1].stride    = 0u;
    cbs[1].nextconbk = 0u;   // fim de chain
    cbs[1]._pad[0]   = 0u;
    cbs[1]._pad[1]   = 0u;
}
// Para DMA circular (M033 — RAF_033_dma_circular.c):
// cbs[N-1].nextconbk = phys_cb0_addr;  // aponta de volta para o inicio
```

```bash
gcc -std=c11 -O2 RAF_032_dma_control_block_chain.c \
  -I /path/to/RafPolimata -o dma_test
# Self-test nao requer hardware BCM — executa em qualquer host Linux
./dma_test    # retorna 0 se estrutura montada corretamente
```

---

## Compilar e executar no Raspberry Pi

### Compilacao generica (M021-M035):
```bash
gcc -std=c11 -O2 RAF_021_gpio_por_mmap.c \
  -I /path/to/RafPolimata -o gpio_test
sudo ./gpio_test
```

### Com otimizacoes ARM64 especificas (Pi3/Pi4):
```bash
gcc -std=c11 -O2 -march=armv8-a+crc -mcpu=cortex-a53 \
  RAF_024_leitura_de_contador_arm64_cntvct_el0.c \
  -I /path/to/RafPolimata -o counter_test
./counter_test   # sem sudo — cntvct_el0 disponivel em EL0 no RPi OS
```

### Verificar acesso a cntvct_el0:
```bash
# Deve imprimir "EL0VCTEN=1" ou equivalente
dmesg | grep -i "cntfrq\|cntvct\|virt"
# Em RPi OS, o kernel habilita EL0VCTEN por padrao.
# Se CNTKCTL_EL1.EL0VCTEN=0, mrs cntvct_el0 dispara SIGILL.
# Nesse caso, usar clock_gettime(CLOCK_MONOTONIC) como fallback.
```

### /dev/gpiomem (M022 — sem root):
```bash
gcc -std=c11 -O2 RAF_022_gpio_por_dev_gpiomem.c \
  -I /path/to/RafPolimata -o gpio_safe
# Nao precisa de sudo se usuario esta no grupo gpio:
./gpio_safe
```

---

## Nota sobre TOKEN_VAZIO em metodos que retornam 0

Os metodos M021-M035 retornam `0` em dois casos distintos:
1. **Sucesso com hardware**: o metodo executou e o hardware respondeu.
2. **TOKEN_VAZIO**: hardware ausente ou sem permissao — retorna 0 sem erro.

Para distinguir os dois casos em producao, use o padrao de retorno binario:
```c
// Modificar assinatura para retornar 1 em sucesso real, 0 em TOKEN_VAZIO:
// Implementacao alternativa de m021 com retorno distintos:
int gpio_init_verbose(void) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) return 0;   // TOKEN_VAZIO — /dev/mem inacessivel
    // ... mmap ...
    if (gpio_map == MAP_FAILED) return 0;  // TOKEN_VAZIO — endereco BCM invalido
    return 1;  // hardware real confirmado
}
```

Isso e descrito no protocolo `docs/TOKEN_VAZIO_PARABOLAS_MESTRES.md` e
refletido na coverage matrix em `docs/BENCHMARK_VISUAL.md`.
