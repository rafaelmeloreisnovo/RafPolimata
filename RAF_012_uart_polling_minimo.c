#include "RAF_rafaelia_common.h"

/*
 * Método M012: UART polling mínimo
 * Alvo: MCU/AVR
 * Domínio: UART/Serial
 * Ganho estimado: eliminação da sobrecarga do HAL
 *
 * UBRR = F_CPU/(16*9600)-1 = 103 para 9600 baud a 16 MHz.
 * UCSR0C: UCSZ01|UCSZ00 (8-bit frame).
 * UCSR0B: RXEN0|TXEN0 (habilita RX e TX).
 * Polling: espera UDRE0 para TX, espera RXC0 para RX.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_UCSR0A_ADDR  0xC0u
#define AVR_UCSR0B_ADDR  0xC1u
#define AVR_UCSR0C_ADDR  0xC2u
#define AVR_UBRR0L_ADDR  0xC4u
#define AVR_UBRR0H_ADDR  0xC5u
#define AVR_UDR0_ADDR    0xC6u

/* UCSR0A bits */
#define M012_RXC0   7u   /* RX Complete */
#define M012_UDRE0  5u   /* USART Data Register Empty */

/* UCSR0B bits */
#define M012_RXEN0  4u   /* Receiver Enable */
#define M012_TXEN0  3u   /* Transmitter Enable */

/* UCSR0C bits */
#define M012_UCSZ01 2u   /* Character size bit 1 */
#define M012_UCSZ00 1u   /* Character size bit 0 */

/* UBRR for 9600 baud at F_CPU=16MHz: (16000000/(16*9600))-1 = 103 */
#define M012_UBRR_9600 ((uint16_t)((F_CPU / (16UL * 9600UL)) - 1UL))
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
static void rafaelia_m012_uart_init(void) {
    /* Set baud rate */
    RAFA_MMIO8(AVR_UBRR0H_ADDR) = (uint8_t)(M012_UBRR_9600 >> 8u);
    RAFA_MMIO8(AVR_UBRR0L_ADDR) = (uint8_t)(M012_UBRR_9600 & 0xFFu);

    /* 8-bit frame, no parity, 1 stop bit */
    RAFA_MMIO8(AVR_UCSR0C_ADDR) = (uint8_t)((1u << M012_UCSZ01) |
                                              (1u << M012_UCSZ00));

    /* Enable RX and TX */
    RAFA_MMIO8(AVR_UCSR0B_ADDR) = (uint8_t)((1u << M012_RXEN0) |
                                              (1u << M012_TXEN0));
}

void rafaelia_m012_uart_send(uint8_t b) {
    /* Wait until transmit buffer is empty */
    while (!(RAFA_MMIO8(AVR_UCSR0A_ADDR) & (uint8_t)(1u << M012_UDRE0))) {
        /* spin */
    }
    RAFA_MMIO8(AVR_UDR0_ADDR) = b;
}

uint8_t rafaelia_m012_uart_recv(void) {
    /* Wait until receive complete */
    while (!(RAFA_MMIO8(AVR_UCSR0A_ADDR) & (uint8_t)(1u << M012_RXC0))) {
        /* spin */
    }
    return RAFA_MMIO8(AVR_UDR0_ADDR);
}
#endif

void rafaelia_m012_uart_polling_minimo(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    rafaelia_m012_uart_init();
    /*
     * Application loop would call rafaelia_m012_uart_send() /
     * rafaelia_m012_uart_recv() as needed.
     * Example: echo loop — not entered here to avoid blocking in demo mode.
     */
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina
     * RAFAELIA_FORCE_AVR_DEMO apenas para inspeção de sintaxe.
     */
#endif
}
