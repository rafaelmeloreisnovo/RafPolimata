# Arduino — Getting Started com RafPolimata M001-M020

Guia para usar os metodos M001-M020 em sketches Arduino (ATmega328P).
Os arquivos fonte de referencia estao na raiz do repositorio (`RAF_001_*.c`
ate `RAF_020_*.c`) e em `RAF_rafaelia_common.h`.

## Pre-requisitos

- Arduino IDE 1.8.x ou 2.x (com suporte a ATmega328P)
- Board: Arduino Uno, Nano ou qualquer placa com ATmega328P
- Toolchain: avr-gcc 5.4+ (incluida na IDE) ou standalone
- Opcional: avr-libc para uso de `avr/io.h` e `avr/interrupt.h`

Verificar versao do avr-gcc:
```
avr-gcc --version
```

---

## Mapeando RAFA_MMIO8 para avr/io.h

`RAF_rafaelia_common.h` define:
```c
#define RAFA_MMIO8(addr)  (*(volatile uint8_t *)(uintptr_t)(addr))
```

No Arduino com `avr/io.h`, os registradores sao macros do tipo `DDRB`,
`PORTB`, `PINB`, que expandem para o mesmo padrao de ponteiro volatil.
Para integrar em sketches, voce pode usar qualquer um:

```c
// Equivalentes — escolha um estilo por arquivo:
RAFA_MMIO8(0x24u) |= (1u << 5);   // DDRB |= (1 << PB5) via endereco
DDRB              |= (1u << PB5);  // identico, via avr/io.h macro
```

Os enderecos AVR usados nos metodos M001-M020 sao definidos como constantes
locais em cada arquivo (ex: `AVR_DDRB_ADDR 0x24u`) e podem ser substituidos
pelos simbolos da avr/io.h sem perda de comportamento.

---

## Tabela: metodo -> Arduino pin mapping -> registrador AVR

| Metodo | Arduino pin | Registrador AVR   | Endereco | Notas                          |
|--------|-------------|-------------------|---------|--------------------------------|
| M001   | D0-D7       | DDRD/PORTD/PIND   | 0x2A-0x2C | GPIO porta D                 |
| M001   | D8-D13      | DDRB/PORTB/PINB   | 0x24-0x25 | GPIO porta B (D13=PB5=LED)   |
| M002   | D8-D13      | PINB              | 0x23    | toggle escrevendo 1 em PINx    |
| M003   | D9 (OC1A)   | TCCR1A/B, OCR1A   | 0x80-0x89 | Timer1 CTC, pino PB1         |
| M004   | D9/D10      | TCCR1A/B, OCR1A/B | 0x80-0x89 | Fast PWM 8-bit               |
| M005   | D9/D10      | TCCR1A/B          | 0x80-0x81 | Phase Correct PWM             |
| M006   | D8 (ICP1)   | TCCR1B, ICR1      | 0x81,0x86 | Input Capture, pino PB0      |
| M007   | D9 (OC1A)   | TCCR1B, OCR1A     | 0x81,0x88 | Output Compare, pino PB1     |
| M008   | A0-A5       | ADMUX, ADCSRA     | 0x7C,0x7A | ADC free-running, canal 0-5  |
| M009   | A0          | ADMUX, ADCSRA     | 0x7C,0x7A | ADC oversampling 16x         |
| M010   | A0          | ADCSRA, ADCL/H    | 0x7A,0x78 | Media movel sem float        |
| M011   | A0          | ADCSRA, ADCL/H    | 0x7A,0x78 | Filtro IIR Q8 fixed-point    |
| M012   | TX/RX       | UDR0, UCSR0A/B    | 0xC6,0xC0 | UART polling sem Serial.h    |
| M013   | TX/RX       | UCSR0B, UDR0      | 0xC1,0xC6 | UART IRQ + ring buffer 16 B  |
| M014   | D10-D13     | SPCR, SPSR, SPDR  | 0x4C-0x4E | SPI full-duplex hardware     |
| M015   | D10-D13     | SPDR              | 0x4E    | SPI burst, sem loop espera   |
| M016   | A4/A5 (SDA/SCL) | TWCR, TWDR    | 0xBC,0xBB | I2C/TWI com timeout         |
| M017   | —           | WDTCSR            | 0x60    | Watchdog reset recovery       |
| M018   | —           | WDTCSR            | 0x60    | Watchdog base temporal aprox  |
| M019   | —           | SMCR, MCUCR       | 0x53,0x55 | Sleep + wake por INT0        |
| M020   | —           | MCUSR             | 0x54    | Brown-out flag BORF          |

---

## Exemplo comentado: Timer CTC (M003) em Arduino

Arquivo de referencia: `RAF_003_timer_ctc_para_evento_periodico.c`

Objetivo: gerar evento periodico a 1 kHz (1 ms) via Timer1 em modo CTC,
sem usar `delay()` ou `millis()`. O pino OC1A (D9/PB1) alterna em hardware
sem intervencao da CPU apos a configuracao.

```c
// sketch Arduino — equivale a RAF_003_timer_ctc_para_evento_periodico.c
// Compila com avr-gcc ou Arduino IDE (ATmega328P, F_CPU=16000000)

#include <avr/io.h>
#include <avr/interrupt.h>

// OCR1A para 1 kHz: F_CPU / prescaler / freq - 1
// = 16000000 / 64 / 1000 - 1 = 249
#define OCR1A_1KHZ 249u

volatile uint16_t tick_count = 0;

ISR(TIMER1_COMPA_vect) {
    tick_count++;           // incrementado a cada 1 ms
}

void setup() {
    // Configura D9 (PB1 / OC1A) como saida
    DDRB |= (1u << PB1);

    // Para o timer antes de reconfigurar
    TCCR1B = 0;
    TCCR1A = 0;
    TCNT1  = 0;

    // OCR1A = valor de comparacao para CTC
    OCR1A = OCR1A_1KHZ;

    // TCCR1B: WGM12=1 (CTC, TOP=OCR1A), CS11+CS10=1 (prescaler /64)
    TCCR1B = (1u << WGM12) | (1u << CS11) | (1u << CS10);

    // Habilita interrupcao de comparacao A
    TIMSK1 = (1u << OCIE1A);

    sei();  // habilita interrupcoes globais
}

void loop() {
    // CPU livre — tick_count atualizado pela ISR a 1 kHz
    if (tick_count >= 1000) {
        tick_count = 0;
        // acao a cada 1 s
        PINB |= (1u << PB5);  // toggle LED D13 via PINx (M002)
    }
}
```

Compilacao standalone (sem IDE):
```bash
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -std=c11 \
  RAF_003_timer_ctc_para_evento_periodico.c -o RAF_003.elf
avr-objcopy -O ihex RAF_003.elf RAF_003.hex
avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 -U flash:w:RAF_003.hex
```

---

## Exemplo comentado: ADC oversampling (M009) em Arduino

Arquivo de referencia: `RAF_009_adc_com_oversampling.c`

Objetivo: obter resolucao efetiva de 12 bits (de 10 bits) acumulando 16
amostras e deslocando 4 bits para direita. Padrao AVR Application Note AN8003.

```c
// sketch Arduino — equivale a RAF_009_adc_com_oversampling.c (versao AVR)
#include <avr/io.h>

// Retorna leitura oversampled de 12 bits no canal ch (0-5)
uint16_t adc_oversample_12bit(uint8_t ch) {
    // Configura canal e referencia AVCC
    ADMUX = (1u << REFS0) | (ch & 0x07u);

    // Habilita ADC, prescaler /128 (125 kHz a 16 MHz)
    ADCSRA = (1u << ADEN) | (1u << ADPS2) | (1u << ADPS1) | (1u << ADPS0);

    uint32_t sum = 0;
    for (uint8_t i = 0; i < 16u; i++) {
        ADCSRA |= (1u << ADSC);           // inicia conversao
        while (ADCSRA & (1u << ADSC));    // aguarda fim (polling)
        sum += ADC;                        // ADC = ADCL | (ADCH << 8)
    }
    // 16 amostras x 10 bits = 14 bits; shift 2 => 12 bits efetivos
    // Formula AN8003: n_samples = 4^extra_bits; shift = extra_bits
    // 16 = 4^2, extra_bits = 2 => resolucao +2 bits = 12 bits
    return (uint16_t)(sum >> 2u);

    // Para +4 bits (14 bits efetivos): acumular 256 amostras, shift 4
    // Custo: 256 conversoes x ~112 us = ~28 ms por leitura
}

void setup() {
    Serial.begin(115200);
}

void loop() {
    uint16_t val = adc_oversample_12bit(0);  // canal A0
    Serial.println(val);                      // range 0-4095
    delay(100);
}
```

Nota de timing: cada conversao ADC com prescaler /128 leva ~112 us.
16 amostras = ~1.8 ms por leitura oversampled de 12 bits.
Evitar usar `delay()` dentro do loop de oversampling — bloqueia CPU
e distorce o resultado se houver ruido periodico sincronizado.

---

## Exemplo comentado: UART ring buffer (M013) em Arduino

Arquivo de referencia: `RAF_013_uart_interrupt_driven_com_ring_buffer.c`

Objetivo: receber bytes na UART via ISR, armazenar em ring buffer de 16
entradas sem bloquear a CPU. A funcao `loop()` consome o buffer quando
disponivel.

```c
// sketch Arduino — equivale a RAF_013_uart_interrupt_driven_com_ring_buffer.c
#include <avr/io.h>
#include <avr/interrupt.h>

#define BUF_SIZE 16u
#define BUF_MASK ((uint8_t)(BUF_SIZE - 1u))

static volatile uint8_t rxbuf[BUF_SIZE];
static volatile uint8_t head = 0;   // ISR escreve
static volatile uint8_t tail = 0;   // loop() le

// ISR: USART RX Complete — chamada a cada byte recebido
ISR(USART_RX_vect) {
    uint8_t b    = UDR0;                        // sempre ler UDR0
    uint8_t next = (head + 1u) & BUF_MASK;
    if (next != tail) {                          // nao cheio
        rxbuf[head] = b;
        head = next;
    }
    // se cheio: byte descartado (sem bloqueio, sem corrupcao do estado)
}

// Pop: retorna -1 se vazio, 0 se byte disponivel em *b
static int rx_pop(uint8_t *b) {
    if (head == tail) return -1;
    *b   = rxbuf[tail];
    tail = (tail + 1u) & BUF_MASK;
    return 0;
}

void setup() {
    // UART 9600 baud, ATmega328P a 16 MHz
    // UBRR0 = F_CPU / (16 * baud) - 1 = 16000000 / (16 * 9600) - 1 = 103
    UBRR0H = 0;
    UBRR0L = 103u;
    // Habilita RX, TX e interrupcao RX
    UCSR0B = (1u << RXEN0) | (1u << TXEN0) | (1u << RXCIE0);
    // 8 bits de dados, 1 stop, sem paridade
    UCSR0C = (1u << UCSZ01) | (1u << UCSZ00);
    sei();
}

void loop() {
    uint8_t b;
    while (rx_pop(&b) == 0) {
        // processa byte sem bloquear
        // ex: eco via polling TX
        while (!(UCSR0A & (1u << UDRE0)));
        UDR0 = b;
    }
    // CPU livre para outras tarefas enquanto aguarda RX
}
```

O ring buffer usa apenas mascaramento (`& BUF_MASK`) — nenhuma divisao,
nenhuma chamada a funcao de sistema. O padrao e identico ao implementado
em `RAF_013_uart_interrupt_driven_com_ring_buffer.c` (M013).

---

## Compilacao standalone avr-gcc

Para compilar qualquer metodo M001-M020 fora da Arduino IDE:

```bash
# Sintaxe: avr-gcc -mmcu=<mcu> -DF_CPU=<freq>UL [flags] arquivo.c -o saida.elf

# Exemplo M003 (Timer CTC):
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -std=c11 \
  -I /path/to/RafPolimata \
  RAF_003_timer_ctc_para_evento_periodico.c -o RAF_003.elf

# Exemplo M009 (ADC oversampling) — force AVR demo em host:
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -DRAFAELIA_FORCE_AVR_DEMO \
  -Os -std=c11 -I /path/to/RafPolimata \
  RAF_009_adc_com_oversampling.c -o RAF_009.elf

# Verificar tamanho .text:
avr-size --format=avr --mcu=atmega328p RAF_003.elf
```

Para teste em host x86_64 (sem AVR hardware):
```bash
# O guard #if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
# garante que o codigo de registrador nao compila em host.
# A versao de self-test (sem AVR) e compilada automaticamente.
gcc -std=c11 -DRAFAELIA_FORCE_AVR_DEMO -I /path/to/RafPolimata \
  RAF_009_adc_com_oversampling.c -o RAF_009_host
./RAF_009_host  # self-test retorna 0 se pass
```
