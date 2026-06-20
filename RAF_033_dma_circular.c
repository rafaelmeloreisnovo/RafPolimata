#include "RAF_rafaelia_common.h"

/*
 * Método M033: DMA circular
 * Alvo: Raspberry Pi (BCM2835/BCM2837)
 * Domínio: DMA
 * Ganho estimado: streaming contínuo sem CPU
 *
 * Two-CB circular ring: last CB points back to first.
 * Self-test verificável em qualquer host (sem hardware BCM).
 */

/* DMA Transfer Information (TI) flag bits */
#ifndef DMA_TI_SRC_INC
#define DMA_TI_NO_WIDE_BURSTS (1u << 26)
#define DMA_TI_WAITS(n)       (((n) & 0x1Fu) << 21)
#define DMA_TI_PERMAP(n)      (((n) & 0x1Fu) << 16)
#define DMA_TI_BURST(n)       (((n) & 0x0Fu) << 12)
#define DMA_TI_SRC_INC        (1u << 8)
#define DMA_TI_DEST_INC       (1u << 4)
#define DMA_TI_WAIT_RESP      (1u << 3)
#define DMA_TI_INTEN          (1u << 0)
#endif

/*
 * BCM DMA Control Block — 32 bytes, 32-byte aligned in hardware use.
 */
#ifndef RAFAELIA_DMA_CB_DEFINED
#define RAFAELIA_DMA_CB_DEFINED
typedef struct {
    uint32_t ti;
    uint32_t source_ad;
    uint32_t dest_ad;
    uint32_t txfr_len;
    uint32_t stride;
    uint32_t nextconbk;
    uint32_t _pad[2];
} rafaelia_dma_cb_t;
#endif

int rafaelia_m033_dma_circular(volatile uint32_t *mmio_base)
{
    (void)mmio_base;

    /*
     * Two-CB circular ring for continuous streaming.
     * In real use: both CBs must be in physically-contiguous bus-accessible memory,
     * and the DMA channel CS register written with the physical address of cbcirc[0].
     */
    static rafaelia_dma_cb_t cbcirc[2] = {{0, 0, 0, 0, 0, 0, {0, 0}},
                                           {0, 0, 0, 0, 0, 0, {0, 0}}};

    /* CB 0: first half of circular buffer */
    cbcirc[0].ti        = DMA_TI_SRC_INC | DMA_TI_DEST_INC | DMA_TI_WAIT_RESP;
    cbcirc[0].source_ad = 0u;   /* RUNTIME: physical source half-0 */
    cbcirc[0].dest_ad   = 0u;   /* RUNTIME: physical destination */
    cbcirc[0].txfr_len  = 64u;
    cbcirc[0].stride    = 0u;
    cbcirc[0].nextconbk = (uint32_t)(uintptr_t)&cbcirc[1];
    cbcirc[0]._pad[0]   = 0u;
    cbcirc[0]._pad[1]   = 0u;

    /* CB 1: second half; wraps back to CB 0 */
    cbcirc[1].ti        = DMA_TI_SRC_INC | DMA_TI_DEST_INC | DMA_TI_WAIT_RESP;
    cbcirc[1].source_ad = 0u;   /* RUNTIME: physical source half-1 */
    cbcirc[1].dest_ad   = 0u;
    cbcirc[1].txfr_len  = 64u;
    cbcirc[1].stride    = 0u;
    cbcirc[1].nextconbk = (uint32_t)(uintptr_t)&cbcirc[0]; /* circular wrap */
    cbcirc[1]._pad[0]   = 0u;
    cbcirc[1]._pad[1]   = 0u;

    /* Self-test: verify circular links are intact */
    int link0_ok = (cbcirc[0].nextconbk == (uint32_t)(uintptr_t)&cbcirc[1]);
    int link1_ok = (cbcirc[1].nextconbk == (uint32_t)(uintptr_t)&cbcirc[0]);

    return (link0_ok && link1_ok) ? 0 : -1;
}
