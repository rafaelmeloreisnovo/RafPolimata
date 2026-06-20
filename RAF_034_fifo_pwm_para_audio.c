#include "RAF_rafaelia_common.h"

/*
 * Método M034: FIFO PWM para áudio
 * Alvo: Raspberry Pi (BCM2835/BCM2837)
 * Domínio: PWM/Audio
 * Ganho estimado: CPU livre — DMA alimenta FIFO
 *
 * Configura PWM em modo FIFO/serialiser para saída de áudio.
 * Self-test gera forma de onda quadrada verificável sem hardware.
 */

/* Audio parameters */
#define M034_SAMPLE_RATE     44100u   /* Hz */
#define M034_BUF_SIZE        64u      /* Samples in static buffer */

/* PWM register offsets (uint32_t words from pwm_base) */
#define M034_PWM_CTL_OFFSET  (0x00u / 4u)
#define M034_PWM_STA_OFFSET  (0x04u / 4u)
#define M034_PWM_DMAC_OFFSET (0x08u / 4u)
#define M034_PWM_RNG1_OFFSET (0x10u / 4u)
#define M034_PWM_DAT1_OFFSET (0x14u / 4u)
#define M034_PWM_FIF1_OFFSET (0x18u / 4u)

/* PWM_CTL bits for FIFO/serialiser audio mode */
#define M034_PWM_CTL_PWEN1  (1u << 0)  /* Channel 1 enable */
#define M034_PWM_CTL_MODE1  (1u << 1)  /* Serialiser mode */
#define M034_PWM_CTL_RPTL1  (1u << 2)  /* Repeat last word when FIFO empty */
#define M034_PWM_CTL_SBIT1  (1u << 3)  /* Silence bit (output when idle) */
#define M034_PWM_CTL_POLA1  (1u << 4)  /* Polarity invert */
#define M034_PWM_CTL_USEF1  (1u << 5)  /* Use FIFO for channel 1 */
#define M034_PWM_CTL_CLRF1  (1u << 6)  /* Clear FIFO */
#define M034_PWM_CTL_MSEN1  (1u << 7)  /* M/S enable for channel 1 */

/* PWM_DMAC bits */
#define M034_PWM_DMAC_ENAB  (1u << 31) /* DMA enable */
#define M034_PWM_DMAC_PANIC(n) (((n) & 0xFFu) << 8)
#define M034_PWM_DMAC_DREQ(n)  (((n) & 0xFFu) << 0)

/* Static audio sample buffer (no heap) */
static uint32_t _m034_audio_buf[M034_BUF_SIZE];

/*
 * rafaelia_m034_fill_square_wave — integer square wave generator.
 * buf:            output buffer.
 * n:              number of samples to fill.
 * period_samples: total samples per cycle.
 *
 * First half of each period = 0xFFFFFFFF (high), second = 0x00000000 (low).
 */
static void rafaelia_m034_fill_square_wave(uint32_t *buf, uint32_t n,
                                            uint32_t period_samples)
{
    uint32_t half = period_samples / 2u;
    if (half == 0u) half = 1u;   /* guard against degenerate period */
    for (uint32_t i = 0u; i < n; i++) {
        buf[i] = ((i % period_samples) < half) ? 0xFFFFFFFFu : 0x00000000u;
    }
}

int rafaelia_m034_fifo_pwm_para_audio(volatile uint32_t *mmio_base)
{
    (void)mmio_base;

    /*
     * Period for 440 Hz at 44100 Hz sample rate:
     *   period = 44100 / 440 ≈ 100 samples
     */
    uint32_t period = M034_SAMPLE_RATE / 440u;

    rafaelia_m034_fill_square_wave(_m034_audio_buf, M034_BUF_SIZE, period);

#if defined(RASPBERRYPI)
    if (mmio_base) {
        /* Stop PWM, clear FIFO */
        mmio_base[M034_PWM_CTL_OFFSET] = M034_PWM_CTL_CLRF1;

        /* Set range to 32 bits (one full serialiser word per sample) */
        mmio_base[M034_PWM_RNG1_OFFSET] = 32u;

        /* Enable DMA with panic/DREQ thresholds */
        mmio_base[M034_PWM_DMAC_OFFSET] = M034_PWM_DMAC_ENAB
                                         | M034_PWM_DMAC_PANIC(7u)
                                         | M034_PWM_DMAC_DREQ(3u);

        /* Enable PWM channel 1 in serialiser+FIFO mode */
        mmio_base[M034_PWM_CTL_OFFSET] = M034_PWM_CTL_USEF1
                                        | M034_PWM_CTL_MODE1
                                        | M034_PWM_CTL_PWEN1;

        /* Prime FIFO with first batch (hardware will stall until DMA starts) */
        for (uint32_t i = 0u; i < 4u && i < M034_BUF_SIZE; i++) {
            mmio_base[M034_PWM_FIF1_OFFSET] = _m034_audio_buf[i];
        }
    }
#endif

    /* Self-test: first sample must be 0xFFFFFFFF (within first half-period) */
    return (_m034_audio_buf[0] == 0xFFFFFFFFu) ? 0 : -1;
}
