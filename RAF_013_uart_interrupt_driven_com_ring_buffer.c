#include "RAF_rafaelia_common.h"

/*
 * Método M013: UART interrupt-driven com ring buffer
 * Alvo: MCU/AVR
 * Domínio: UART/Serial
 * Ganho estimado: não bloqueia CPU em RX
 *
 * Ring buffer de 16 bytes para RX via interrupção.
 * Self-test não-AVR: push 5 bytes, pop 5 bytes, verificar ordem FIFO.
 * Retorno int: 0=pass, -1=fail.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_UCSR0B_ADDR  0xC1u

/* UCSR0B bits */
#define M013_RXCIE0  7u   /* RX Complete Interrupt Enable */
#define M013_RXEN0   4u
#define M013_TXEN0   3u
#endif

/* Ring buffer — always compiled for self-test */
#define M013_BUF_SIZE 16u
#define M013_BUF_MASK ((uint8_t)(M013_BUF_SIZE - 1u))

static volatile uint8_t _m013_rxbuf[M013_BUF_SIZE];
static volatile uint8_t _m013_head = 0u;  /* write index (ISR writes) */
static volatile uint8_t _m013_tail = 0u;  /* read  index (caller reads) */

/*
 * rafaelia_m013_rx_push: called from ISR (or test code) to enqueue a byte.
 * Silently drops byte if buffer is full (head+1 == tail).
 */
void rafaelia_m013_rx_push(uint8_t b) {
    uint8_t next = (uint8_t)((_m013_head + 1u) & M013_BUF_MASK);
    if (next != _m013_tail) {           /* buffer not full */
        _m013_rxbuf[_m013_head] = b;
        _m013_head = next;
    }
}

/*
 * rafaelia_m013_rx_pop: dequeue one byte into *b.
 * Returns 0 on success, -1 if buffer empty.
 */
int rafaelia_m013_rx_pop(uint8_t *b) {
    if (_m013_head == _m013_tail) {
        return -1;                      /* empty */
    }
    *b = _m013_rxbuf[_m013_tail];
    _m013_tail = (uint8_t)((_m013_tail + 1u) & M013_BUF_MASK);
    return 0;
}

/*
 * ISR stub (comment only — real AVR ISR would be:
 *   ISR(USART_RX_vect) {
 *       rafaelia_m013_rx_push(RAFA_MMIO8(AVR_UDR0_ADDR));
 *   }
 * Cannot register real vectors in hosted/non-avr-gcc compilation.
 */

int rafaelia_m013_uart_interrupt_driven_com_ring_buffer(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /* Enable USART RX Complete interrupt (requires RX already enabled) */
    RAFA_MMIO8(AVR_UCSR0B_ADDR) |= (uint8_t)((1u << M013_RXCIE0) |
                                               (1u << M013_RXEN0)  |
                                               (1u << M013_TXEN0));
    return 0;
#else
    /* Non-AVR self-test: push 5 bytes, pop 5 bytes, verify FIFO order */
    static const uint8_t test_data[5] = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u};
    uint8_t i;
    uint8_t popped;

    /* Reset ring buffer */
    _m013_head = 0u;
    _m013_tail = 0u;

    /* Push 5 bytes */
    for (i = 0u; i < 5u; i++) {
        rafaelia_m013_rx_push(test_data[i]);
    }

    /* Pop 5 bytes and verify order */
    for (i = 0u; i < 5u; i++) {
        if (rafaelia_m013_rx_pop(&popped) != 0) {
            return -1;
        }
        if (popped != test_data[i]) {
            return -1;
        }
    }

    /* Buffer must be empty now */
    if (rafaelia_m013_rx_pop(&popped) != -1) {
        return -1;
    }

    return 0;
#endif
}
