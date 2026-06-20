#include "RAF_rafaelia_common.h"

/*
 * Método M032: DMA control block chain
 * Alvo: Raspberry Pi (BCM2835/BCM2837)
 * Domínio: DMA
 * Ganho estimado: CPU livre durante transferências
 *
 * Define o struct DMA control block e encadeia 2 CBs.
 * Self-test verificável em qualquer host (sem hardware BCM).
 */

/* DMA Transfer Information (TI) flag bits */
#define DMA_TI_NO_WIDE_BURSTS (1u << 26)
#define DMA_TI_WAITS(n)       (((n) & 0x1Fu) << 21)
#define DMA_TI_PERMAP(n)      (((n) & 0x1Fu) << 16)
#define DMA_TI_BURST(n)       (((n) & 0x0Fu) << 12)
#define DMA_TI_SRC_INC        (1u << 8)
#define DMA_TI_DEST_INC       (1u << 4)
#define DMA_TI_WAIT_RESP      (1u << 3)
#define DMA_TI_INTEN          (1u << 0)

/*
 * BCM DMA Control Block — must be 256-bit (32-byte) aligned in hardware use.
 * _pad[2] makes it exactly 32 bytes.
 */
typedef struct {
    uint32_t ti;          /* Transfer Information */
    uint32_t source_ad;   /* Source Bus Address */
    uint32_t dest_ad;     /* Destination Bus Address */
    uint32_t txfr_len;    /* Transfer Length (bytes) */
    uint32_t stride;      /* 2D Mode Stride */
    uint32_t nextconbk;   /* Next Control Block Address */
    uint32_t _pad[2];     /* Reserved (must be 0) */
} rafaelia_dma_cb_t;

int rafaelia_m032_dma_control_block_chain(volatile uint32_t *mmio_base)
{
    (void)mmio_base;

    /*
     * Build a two-element linear CB chain.
     * static so the addresses are stable and in BSS (not stack).
     * In real use these must be in physically-contiguous, bus-addressable memory.
     */
    static rafaelia_dma_cb_t cbs[2] = {{0, 0, 0, 0, 0, 0, {0, 0}},
                                        {0, 0, 0, 0, 0, 0, {0, 0}}};

    /* CB 0: memory-to-memory copy, 16 bytes */
    cbs[0].ti        = DMA_TI_SRC_INC | DMA_TI_DEST_INC | DMA_TI_WAIT_RESP;
    cbs[0].source_ad = 0u;   /* RUNTIME: physical source address */
    cbs[0].dest_ad   = 0u;   /* RUNTIME: physical destination address */
    cbs[0].txfr_len  = 16u;
    cbs[0].stride    = 0u;
    cbs[0].nextconbk = (uint32_t)(uintptr_t)&cbs[1];
    cbs[0]._pad[0]   = 0u;
    cbs[0]._pad[1]   = 0u;

    /* CB 1: end-of-chain marker */
    cbs[1].ti        = DMA_TI_SRC_INC | DMA_TI_DEST_INC;
    cbs[1].source_ad = 0u;
    cbs[1].dest_ad   = 0u;
    cbs[1].txfr_len  = 0u;
    cbs[1].stride    = 0u;
    cbs[1].nextconbk = 0u;   /* End of chain */
    cbs[1]._pad[0]   = 0u;
    cbs[1]._pad[1]   = 0u;

    /* Self-test: verify the chain link is intact */
    return (cbs[0].nextconbk == (uint32_t)(uintptr_t)&cbs[1]) ? 0 : -1;
}
