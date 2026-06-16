#include "RAF_rafaelia_common.h"

/*
 * Método M013: UART interrupt-driven com ring buffer
 * Alvo: MCU/AVR
 * Domínio: UART
 * Ganho estimado: CPU livre
 *
 * Fila circular por interrupção.
 *
 * Status: skeleton C para integração em firmware bare-metal.
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
#define AVR_DDRB_ADDR   0x24u
#define AVR_PINB_ADDR   0x23u
#define AVR_PORTB_ADDR  0x25u
#define AVR_TCCR1A_ADDR 0x80u
#define AVR_TCCR1B_ADDR 0x81u
#define AVR_OCR1A_ADDR  0x88u
#define AVR_UCSR0A_ADDR 0xC0u
#define AVR_UCSR0B_ADDR 0xC1u
#define AVR_UCSR0C_ADDR 0xC2u
#define AVR_UDR0_ADDR   0xC6u
#define AVR_ADCSRA_ADDR 0x7Au
#define AVR_ADMUX_ADDR  0x7Cu
#define AVR_ADCL_ADDR   0x78u
#define AVR_ADCH_ADDR   0x79u
#endif

void rafaelia_m013_uart_interrupt_driven_com_ring_buffer(void) {
#if defined(__AVR_ATmega328P__) || defined(RAFAELIA_FORCE_AVR_DEMO)
    /*
     * Ajuste este bloco conforme o método específico.
     * Mantido simples para permitir auditoria direta de registrador.
     */
    RAFA_MMIO8(AVR_DDRB_ADDR) |= (uint8_t)(1u << 5u);
    RAFA_MMIO8(AVR_PINB_ADDR) = (uint8_t)(1u << 5u);
#else
    /*
     * Método específico de MCU/AVR. Compile com avr-gcc ou defina RAFAELIA_FORCE_AVR_DEMO
     * apenas para inspeção de sintaxe.
     */
#endif
}
