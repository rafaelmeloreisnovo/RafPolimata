# AVR ATmega328P Architecture Reference

This document covers the ATmega328P register map, timer formulas, interrupt
vectors, and the memory model as used in the RafPolimata project. All register
addresses and bit constants are taken directly from the RAF_001–RAF_020 source
files. Default values are F_CPU = 16,000,000 Hz (16 MHz).

---

## 1. Memory Model

The ATmega328P uses a Harvard architecture: program memory (Flash) and data
memory (SRAM + I/O space) are separate address spaces with different access
mechanisms.

| Space          | Size     | Address range  | Access |
|----------------|----------|----------------|--------|
| Flash (program)| 32 KiB   | 0x0000–0x3FFF  | LPM, SPM |
| SRAM (data)    | 2 KiB    | 0x0100–0x08FF  | LD/ST |
| I/O registers  | 64 bytes | 0x0020–0x005F  | IN/OUT |
| Extended I/O   | 160 bytes| 0x0060–0x00FF  | LD/ST only |
| EEPROM         | 1 KiB    | (separate space) | EEAR/EEDR |

Peripheral registers visible to C code appear in the data address space
(MMIO mapped into SRAM address range). The project accesses them via:

```c
#define RAFA_MMIO8(addr)  (*(volatile uint8_t *)(uintptr_t)(addr))
#define RAFA_MMIO16(addr) (*(volatile uint16_t *)(uintptr_t)(addr))
#define RAFA_MMIO32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))
```

These macros are defined in `RAF_rafaelia_common.h`. All ATmega328P peripheral
registers are 8-bit wide; multi-byte registers (Timer counters, OCR, ICR) occupy
two consecutive 8-bit addresses accessed as `HIGH:LOW` pairs.

Because the address space is mapped into data space, every peripheral register
is accessed with ordinary load/store instructions. There is no distinction between
MMIO and RAM from the C compiler's perspective; the `volatile` qualifier ensures
the compiler does not cache register values across accesses.

---

## 2. Register Address Map

All addresses below are data-space byte addresses (confirmed against source files
RAF_001–RAF_020).

### 2.1 GPIO (Port B)

| Register | Address | Source file | Description |
|----------|---------|-------------|-------------|
| PINB     | 0x23    | RAF_001, RAF_002 | Port B Input Pins |
| DDRB     | 0x24    | RAF_001, RAF_003, RAF_014 | Port B Data Direction Register |
| PORTB    | 0x25    | RAF_001 | Port B Data Output Register |

Writing a 1 to a PINB bit toggles the corresponding PORTB output bit without
a read-modify-write (RAF_002 demonstrates this pattern):
```c
RAFA_MMIO8(AVR_PINB_ADDR) = (uint8_t)(1u << 5u);  // toggle PB5
```

### 2.2 Timer 1 (16-bit)

| Register | Address | Source file | Description |
|----------|---------|-------------|-------------|
| TIMSK1   | 0x6F    | RAF_003     | Timer Interrupt Mask 1 |
| TCCR1A   | 0x80    | RAF_001, RAF_003, RAF_004 | Timer/Counter Control A |
| TCCR1B   | 0x81    | RAF_001, RAF_003, RAF_004 | Timer/Counter Control B |
| TCNT1L   | 0x84    | RAF_003, RAF_004 | Timer Counter Low byte |
| TCNT1H   | 0x85    | RAF_003, RAF_004 | Timer Counter High byte |
| ICR1     | 0x86    | (referenced in CLAUDE.md) | Input Capture Register 1 |
| OCR1AL   | 0x88    | RAF_003, RAF_004 | Output Compare Register A Low |
| OCR1AH   | 0x89    | RAF_003, RAF_004 | Output Compare Register A High |

TCNT1 and OCR1A are 16-bit registers accessed as two 8-bit reads/writes. The
sequence requires writing the high byte first for OCR1A when the timer is running.

### 2.3 ADC

| Register | Address | Source file | Description |
|----------|---------|-------------|-------------|
| ADCL     | 0x78    | RAF_001, RAF_008 | ADC Data Low byte (read first) |
| ADCH     | 0x79    | RAF_001, RAF_008 | ADC Data High byte |
| ADCSRA   | 0x7A    | RAF_001, RAF_008 | ADC Control and Status A |
| ADCSRB   | 0x7B    | RAF_008     | ADC Control and Status B |
| ADMUX    | 0x7C    | RAF_001, RAF_008 | ADC Multiplexer Selection |

Reading ADCL before ADCH locks the ADC result registers for consistent 16-bit
reads. Both must be read before the next conversion completes.

### 2.4 UART (USART0)

| Register | Address | Source file | Description |
|----------|---------|-------------|-------------|
| UCSR0A   | 0xC0    | RAF_001, RAF_012 | USART Control and Status A |
| UCSR0B   | 0xC1    | RAF_012     | USART Control and Status B |
| UCSR0C   | 0xC2    | RAF_012     | USART Control and Status C |
| UBRR0L   | 0xC4    | RAF_012     | USART Baud Rate Low |
| UBRR0H   | 0xC5    | RAF_012     | USART Baud Rate High |
| UDR0     | 0xC6    | RAF_001, RAF_012 | USART I/O Data Register |

### 2.5 SPI

| Register | Address | Source file | Description |
|----------|---------|-------------|-------------|
| SPCR     | 0x4C    | RAF_014     | SPI Control Register |
| SPSR     | 0x4D    | RAF_014     | SPI Status Register |
| SPDR     | 0x4E    | RAF_014     | SPI Data Register |

### 2.6 I2C / TWI

| Register | Address | Source file | Description |
|----------|---------|-------------|-------------|
| TWBR     | 0xB8    | RAF_016     | TWI Bit Rate Register |
| TWSR     | 0xB9    | RAF_016     | TWI Status Register |
| TWDR     | 0xBB    | RAF_016     | TWI Data Register |
| TWCR     | 0xBC    | RAF_016     | TWI Control Register |

### 2.7 Watchdog

| Register | Address | Source file | Description |
|----------|---------|-------------|-------------|
| WDTCSR   | 0x60    | RAF_017     | Watchdog Timer Control and Status |
| MCUSR    | 0x54    | RAF_017, RAF_020 | MCU Status Register (reset flags) |

### 2.8 Sleep Control

| Register | Address | Source file | Description |
|----------|---------|-------------|-------------|
| SMCR     | 0x53    | RAF_019     | Sleep Mode Control Register |

---

## 3. Timer 1 — Operating Modes

### 3.1 CTC Mode (Clear Timer on Compare)

Source file: RAF_003.

Configuration registers:
- `TCCR1A = 0x00` (no OC pin connection in ISR mode)
- `TCCR1B`: WGM12 = bit 3 (enables CTC), CS1x for prescaler

OCR1A formula:
```
OCR1A = (F_CPU / prescaler / target_hz) - 1
```

Example (1 kHz, /64 prescaler, F_CPU = 16 MHz):
```
OCR1A = 16,000,000 / 64 / 1000 - 1 = 249
```

Defined in source as:
```c
#define M003_OCR1A_1KHZ  ((uint16_t)((F_CPU / 64UL / 1000UL) - 1UL))
```

TIMSK1 bit `OCIE1A` (bit 1) at address 0x6F enables the Output Compare A match
interrupt which fires on every CTC match.

### 3.2 Fast PWM Mode (Mode 5: 8-bit Fast PWM)

Source file: RAF_004.

Configuration:
- `TCCR1A`: COM1A1 = bit 7 (non-inverting), WGM10 = bit 0
- `TCCR1B`: WGM12 = bit 3, CS11 | CS10 (/64 prescaler)
- TOP = 0xFF (8-bit, fixed)
- OCR1A = 127 for 50% duty cycle

SPCR value for mode 5:
```c
TCCR1A = (1 << COM1A1) | (1 << WGM10);    // 0b10000001 = 0x81
TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);  // /64
```

### 3.3 Phase-Correct PWM (RAF_005)

Mode 1 (8-bit phase-correct): WGM10 = 1 in TCCR1A. TOP = 0xFF. Timer counts
up then down, producing a symmetric output. Used for motor control where
symmetrical duty cycles avoid acoustic noise.

### 3.4 Prescaler Values for Timer 1

Timer 1 CS1[2:0] bits in TCCR1B:

| CS12 | CS11 | CS10 | Prescaler | Description |
|------|------|------|-----------|-------------|
| 0    | 0    | 0    | stopped   | Timer stopped |
| 0    | 0    | 1    | /1        | No prescaling |
| 0    | 1    | 0    | /8        | |
| 0    | 1    | 1    | /64       | Used in RAF_003, RAF_004 |
| 1    | 0    | 0    | /256      | |
| 1    | 0    | 1    | /1024     | |
| 1    | 1    | 0    | External T1 falling | |
| 1    | 1    | 1    | External T1 rising | |

---

## 4. ADC Configuration

Source files: RAF_008, RAF_009, RAF_010, RAF_011.

### 4.1 ADMUX Register

| Bit | Name    | Description |
|-----|---------|-------------|
| 7   | REFS1   | Reference select bit 1 |
| 6   | REFS0   | Reference select bit 0 (REFS1=0, REFS0=1 = AVcc) |
| 5   | ADLAR   | Left adjust result |
| 3:0 | MUX3:0  | Input channel select (0000 = ADC0 / PC0) |

Value used in RAF_008:
```c
#define M008_ADMUX_VAL  ((uint8_t)(1u << 6u))
// REFS0=1 (AVcc reference), MUX=0 (ADC0/PC0 channel)
```

### 4.2 ADCSRA Register

| Bit | Macro    | Description |
|-----|----------|-------------|
| 7   | ADEN     | ADC Enable |
| 6   | ADSC     | ADC Start Conversion |
| 5   | ADATE    | ADC Auto Trigger Enable |
| 4   | ADIF     | ADC Interrupt Flag (W1C) |
| 3   | ADIE     | ADC Interrupt Enable |
| 2   | ADPS2    | Prescaler bit 2 |
| 1   | ADPS1    | Prescaler bit 1 |
| 0   | ADPS0    | Prescaler bit 0 |

Free-running value from RAF_008:
```c
// ADEN | ADSC | ADATE | ADIE | ADPS2 | ADPS1 | ADPS0 (/128 prescaler)
// 16 MHz / 128 = 125 kHz ADC clock (within 50-200 kHz range)
ADCSRA = 0b11101111 = 0xEF
```

### 4.3 ADCSRB Register

ADCSRB = 0x00 selects free-running trigger source (ADTS2:ADTS0 = 000).
This causes the ADC to restart immediately after each conversion completes
and raise ADIF / call the ADC ISR continuously.

---

## 5. UART (USART0)

Source file: RAF_012.

### 5.1 UBRR Formula

```
UBRR = (F_CPU / (16 * baud_rate)) - 1
```

Examples at F_CPU = 16 MHz:

| Baud rate | UBRR value | Source |
|-----------|------------|--------|
| 9600      | 103        | RAF_012 (M012_UBRR_9600) |
| 19200     | 51         | (calculation) |
| 115200    | 8          | (calculation) |

UBRR = 103 for 9600 baud is defined in RAF_012 as:
```c
#define M012_UBRR_9600  ((uint16_t)((F_CPU / (16UL * 9600UL)) - 1UL))
```

### 5.2 Initialization Sequence

1. Write UBRR0H = high byte of UBRR.
2. Write UBRR0L = low byte of UBRR.
3. Write UCSR0C: UCSZ01 | UCSZ00 (8-bit frame, no parity, 1 stop bit).
4. Write UCSR0B: RXEN0 | TXEN0 (enable receiver and transmitter).

### 5.3 Polling Send/Receive

TX: poll UDRE0 (bit 5) in UCSR0A until 1, then write to UDR0.
RX: poll RXC0 (bit 7) in UCSR0A until 1, then read UDR0.

```c
// Send
while (!(RAFA_MMIO8(0xC0) & (1u << 5u)));  // wait UDRE0
RAFA_MMIO8(0xC6) = byte;                     // write UDR0

// Receive
while (!(RAFA_MMIO8(0xC0) & (1u << 7u)));  // wait RXC0
byte = RAFA_MMIO8(0xC6);                     // read UDR0
```

---

## 6. SPI

Source file: RAF_014.

### 6.1 DDRB Pin Assignments for SPI

| Pin  | DDRB bit | Direction | Signal |
|------|----------|-----------|--------|
| PB2  | bit 2    | Output    | SS (Slave Select) |
| PB3  | bit 3    | Output    | MOSI |
| PB4  | bit 4    | Input     | MISO |
| PB5  | bit 5    | Output    | SCK |

### 6.2 SPCR Register

| Bit | Macro  | Description |
|-----|--------|-------------|
| 7   | SPIE   | SPI Interrupt Enable |
| 6   | SPE    | SPI Enable |
| 5   | DORD   | Data Order (0 = MSB first) |
| 4   | MSTR   | Master/Slave Select (1 = master) |
| 3   | CPOL   | Clock Polarity |
| 2   | CPHA   | Clock Phase |
| 1   | SPR1   | Clock Rate select bit 1 |
| 0   | SPR0   | Clock Rate select bit 0 |

Value from RAF_014 (SPE=1, MSTR=1, mode 0, /4 prescaler = 0x50):
```c
#define M014_SPCR_VAL  ((uint8_t)((1u << M014_SPE) | (1u << M014_MSTR)))
// = (1<<6) | (1<<4) = 0x50
```

### 6.3 Transfer Sequence

Write to SPDR, poll SPIF (bit 7) in SPSR, read SPDR:
```c
RAFA_MMIO8(0x4E) = tx_byte;                  // write SPDR
while (!(RAFA_MMIO8(0x4D) & (1u << 7u)));   // wait SPIF
rx_byte = RAFA_MMIO8(0x4E);                  // read SPDR
```

---

## 7. I2C / TWI

Source file: RAF_016.

### 7.1 TWBR Formula

```
TWBR = ((F_CPU / SCL_freq) - 16) / 2
```

At F_CPU = 16 MHz, SCL = 100 kHz:
```
TWBR = ((16,000,000 / 100,000) - 16) / 2 = (160 - 16) / 2 = 72
```

Defined in RAF_016 as:
```c
#define M016_TWBR_100KHZ  ((uint8_t)(((F_CPU / 100000UL) - 16UL) / 2UL))
// = 72
```
TWSR prescaler bits must be 0 (prescaler = 1) for this formula to apply.

### 7.2 TWCR Bits

| Bit | Macro   | Description |
|-----|---------|-------------|
| 2   | TWEN    | TWI Enable |
| 4   | TWSTO   | TWI STOP Condition |
| 5   | TWSTA   | TWI START Condition |
| 6   | TWEA    | TWI Enable Acknowledge |
| 7   | TWINT   | TWI Interrupt Flag (W1C) |

Clearing TWINT by writing 1 to it triggers the next TWI action (START, data byte
send, etc.). Polling TWINT for 1 indicates the action is complete.

### 7.3 START with Timeout

RAF_016 implements a timeout of 1000 iterations:
```c
#define M016_TWI_TIMEOUT  1000u
```
If TWINT does not set within 1000 poll iterations, the function returns -1 to
indicate a bus lockup condition.

---

## 8. Watchdog Timer

Source file: RAF_017.

The Watchdog requires a timed two-step write sequence to change WDTCSR.
Both steps must complete within 4 clock cycles:

1. Write WDCE=1 and WDE=1 simultaneously to WDTCSR.
2. Within 4 cycles: write the desired WDP prescaler bits (and WDE=1 if reset mode).

A compiler barrier (`__asm__ __volatile__("" ::: "memory")`) is inserted between
the two writes to prevent reordering.

### 8.1 WDTCSR Bits

| Bit | Macro   | Description |
|-----|---------|-------------|
| 0   | WDP0    | Watchdog prescaler bit 0 |
| 1   | WDP1    | Watchdog prescaler bit 1 |
| 2   | WDP2    | Watchdog prescaler bit 2 |
| 3   | WDE     | Watchdog System Reset Enable |
| 4   | WDCE    | Watchdog Change Enable |
| 5   | WDP3    | Watchdog prescaler bit 3 |
| 6   | WDIE    | Watchdog Interrupt Enable |
| 7   | WDIF    | Watchdog Interrupt Flag |

### 8.2 WDP Prescaler Timeouts

| WDP[3:0] | Timeout (approx.) |
|----------|--------------------|
| 0b0000   | 16 ms |
| 0b0001   | 32 ms |
| 0b0010   | 64 ms |
| 0b0011   | 0.125 s |
| 0b0100   | 0.25 s |
| 0b0101   | 0.5 s |
| 0b0110   | 1 s |
| 0b0111   | 2 s (WDP2|WDP1|WDP0) |
| 0b1000   | 4 s |
| 0b1001   | 8 s |

RAF_017 uses WDP[3:0] = 0b0111 (≈2.1 s):
```c
#define M017_WDT_2S_VAL  ((uint8_t)((1u << M017_WDE) | \
                                     (1u << M017_WDP2) | \
                                     (1u << M017_WDP1) | \
                                     (1u << M017_WDP0)))
```

### 8.3 MCUSR Reset Flags

MCUSR at address 0x54 holds reset cause flags. Defined in RAF_017 and RAF_020:

| Bit | Macro   | Description |
|-----|---------|-------------|
| 0   | PORF    | Power-on Reset Flag |
| 1   | EXTRF   | External Reset Flag |
| 2   | BORF    | Brown-out Reset Flag |
| 3   | WDRF    | Watchdog Reset Flag |

MCUSR must be cleared before enabling the watchdog to prevent immediate re-
triggering. RAF_017 clears WDRF specifically; RAF_020 clears all flags:
```c
RAFA_MMIO8(AVR_MCUSR_ADDR) &= (uint8_t)(~((1u << BORF) | (1u << WDRF) |
                                            (1u << EXTRF) | (1u << PORF)));
```

---

## 9. Sleep Modes

Source file: RAF_019.

SMCR register at address 0x53:

| Bit | Macro | Description |
|-----|-------|-------------|
| 0   | SE    | Sleep Enable |
| 1   | SM0   | Sleep Mode select bit 0 |
| 2   | SM1   | Sleep Mode select bit 1 |
| 3   | SM2   | Sleep Mode select bit 2 |

SM[2:0] mode selection:

| SM2 | SM1 | SM0 | Mode |
|-----|-----|-----|------|
| 0   | 0   | 0   | Idle |
| 0   | 0   | 1   | ADC Noise Reduction |
| 0   | 1   | 0   | Power-down |
| 0   | 1   | 1   | Power-save |
| 1   | 0   | 0   | Reserved |
| 1   | 0   | 1   | Reserved |
| 1   | 1   | 0   | Standby |
| 1   | 1   | 1   | Extended Standby |

RAF_019 uses Idle mode (SM2:SM0 = 000) which halts the CPU clock but keeps
peripherals (timers, SPI, UART, ADC, I2C) running. The wake sequence:
1. Clear SM bits (ensure Idle mode).
2. Execute `sei` (enable global interrupts).
3. Set SE (Sleep Enable) in SMCR.
4. Execute `sleep` instruction.
5. CPU wakes on interrupt; ISR runs.
6. Clear SE after waking.

---

## 10. Common Interrupt Vectors

The ATmega328P has 26 interrupt vectors. Most-used in the project context:

| Vector | Address | Name | Source |
|--------|---------|------|--------|
| 0      | 0x0000  | RESET | Power-on, external, watchdog, brown-out |
| 1      | 0x0002  | INT0 | External Interrupt Request 0 |
| 2      | 0x0004  | INT1 | External Interrupt Request 1 |
| 3      | 0x0006  | PCINT0 | Pin Change Interrupt Request 0 |
| 4      | 0x0008  | PCINT1 | Pin Change Interrupt Request 1 |
| 5      | 0x000A  | PCINT2 | Pin Change Interrupt Request 2 |
| 6      | 0x000C  | WDT | Watchdog Time-out Interrupt |
| 11     | 0x0016  | TIMER1_CAPT | Timer/Counter1 Capture Event |
| 12     | 0x0018  | TIMER1_COMPA | Timer/Counter1 Compare Match A |
| 13     | 0x001A  | TIMER1_COMPB | Timer/Counter1 Compare Match B |
| 14     | 0x001C  | TIMER1_OVF | Timer/Counter1 Overflow |
| 21     | 0x0028  | ADC | ADC Conversion Complete |
| 24     | 0x002E  | USART_RX | USART Rx Complete |
| 25     | 0x0030  | USART_UDRE | USART Data Register Empty |
| 26     | 0x0032  | USART_TX | USART Tx Complete |

TIMER1_COMPA (vector 12) is the interrupt enabled by RAF_003 via TIMSK1 bit
OCIE1A. ADC vector 21 is used in the free-running mode of RAF_008.

---

## 11. Key Source Files

| File | Domain | Key technique |
|------|--------|---------------|
| RAF_001_acesso_direto_a_ddrx_portx_pinx.c | GPIO | DDRB/PORTB/PINB direct access |
| RAF_002_toggle_por_escrita_em_pinx.c | GPIO | PINB write-to-toggle |
| RAF_003_timer_ctc_para_evento_periodico.c | Timer | CTC mode, OCR1A formula |
| RAF_004_timer_fast_pwm_por_registrador.c | PWM | Fast PWM mode 5 |
| RAF_005_timer_phase_correct_pwm_para_controle_motor.c | PWM | Phase-correct PWM |
| RAF_006_input_capture_para_medir_pulso.c | Timer | ICR1, input capture |
| RAF_007_output_compare_para_gerar_onda_sem_cpu.c | Timer | OC1A compare |
| RAF_008_adc_free_running.c | ADC | Free-running + interrupt |
| RAF_009_adc_com_oversampling.c | ADC | Oversampling for resolution |
| RAF_010_adc_com_media_movel_inteira.c | ADC | Integer moving average |
| RAF_011_adc_com_filtro_iir_fixed_point.c | ADC | Fixed-point IIR filter |
| RAF_012_uart_polling_minimo.c | UART | UBRR formula, TX/RX polling |
| RAF_013_uart_interrupt_driven_com_ring_buffer.c | UART | ISR + ring buffer |
| RAF_014_spi_full_duplex_por_registrador.c | SPI | SPCR 0x50, SPIF polling |
| RAF_015_spi_burst_transfer.c | SPI | Burst multi-byte transfer |
| RAF_016_i2c_twi_com_timeout.c | I2C | TWBR formula, TWINT timeout |
| RAF_017_watchdog_como_recuperacao_de_travamento.c | WDT | Timed two-step write |
| RAF_018_watchdog_como_base_temporal_aproximada.c | WDT | WDT as timer source |
| RAF_019_sleep_mode_com_wake_por_interrupcao.c | Power | Idle sleep + interrupt wake |
| RAF_020_brown_out_flag_como_diagnostico_de_alimentacao.c | Power | MCUSR BORF read |
| RAF_rafaelia_common.h | All | RAFA_MMIO8/16/32 macros |
