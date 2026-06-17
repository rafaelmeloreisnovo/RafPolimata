#include "RAF_rafaelia_common.h"

/*
 * Método M031: PWM por clock manager
 * Alvo: Raspberry Pi (BCM2835/BCM2837)
 * Domínio: PWM
 * Ganho estimado: precisão de frequência
 *
 * Configura o clock manager (CM) e PWM peripheral diretamente.
 * Gated em RASPBERRYPI — retorna TOKEN_VAZIO (0) em outros hosts.
 */

/* PWM register offsets (in uint32_t words from pwm_base) */
#define PWM_CTL_OFFSET   (0x00u / 4u)   /* Control */
#define PWM_STA_OFFSET   (0x04u / 4u)   /* Status */
#define PWM_DMAC_OFFSET  (0x08u / 4u)   /* DMA Configuration */
#define PWM_RNG1_OFFSET  (0x10u / 4u)   /* Channel 1 Range */
#define PWM_DAT1_OFFSET  (0x14u / 4u)   /* Channel 1 Data */
#define PWM_FIF1_OFFSET  (0x18u / 4u)   /* FIFO Input */
#define PWM_RNG2_OFFSET  (0x20u / 4u)   /* Channel 2 Range */
#define PWM_DAT2_OFFSET  (0x24u / 4u)   /* Channel 2 Data */

/* PWM_CTL bit positions */
#define PWM_CTL_MSEN2   (1u << 15)  /* Channel 2 M/S Enable */
#define PWM_CTL_PWEN2   (1u << 8)   /* Channel 2 Enable */
#define PWM_CTL_MSEN1   (1u << 7)   /* Channel 1 M/S Enable */
#define PWM_CTL_CLRF1   (1u << 6)   /* Clear FIFO */
#define PWM_CTL_USEF1   (1u << 5)   /* Use FIFO for Ch1 */
#define PWM_CTL_POLA1   (1u << 4)   /* Polarity */
#define PWM_CTL_SBIT1   (1u << 3)   /* Silence Bit */
#define PWM_CTL_RPTL1   (1u << 2)   /* Repeat last data */
#define PWM_CTL_MODE1   (1u << 1)   /* Serialiser mode */
#define PWM_CTL_PWEN1   (1u << 0)   /* Channel 1 Enable */

/*
 * Clock Manager offsets relative to CM base (0x3F101000).
 * The CM base is SEPARATE from the PWM base (0x3F20C000).
 * We receive pwm_base; the caller must also map CM if running on hardware.
 * Here we define the CM offsets as a reference constant for documentation.
 */
#define CM_PWMCTL_OFFSET (0xA0u / 4u)  /* CM PWM Control (word index from CM base) */
#define CM_PWMDIV_OFFSET (0xA4u / 4u)  /* CM PWM Divisor */

/* Clock Manager password (must OR into every CM write) */
#define CM_PASSWD        (0x5A000000u)

/* CM_PWMCTL bits */
#define CM_PWMCTL_BUSY   (1u << 7)   /* Clock is running */
#define CM_PWMCTL_KILL   (1u << 5)   /* Kill clock */
#define CM_PWMCTL_ENAB   (1u << 4)   /* Enable clock */
#define CM_PWMCTL_SRC_OSC (1u)       /* Source = 19.2 MHz oscillator */
#define CM_PWMCTL_SRC_PLLD (6u)      /* Source = PLLD (500 MHz) */

/*
 * rafaelia_m031_pwm_setup — configure CM + PWM for a given duty cycle.
 * pwm_base: mapped PWM peripheral register base.
 * cm_base:  mapped Clock Manager register base.
 * range:    PWM period in clock ticks (RNG1).
 * duty:     High ticks within period (DAT1).
 */
static void rafaelia_m031_pwm_setup(volatile uint32_t *pwm_base,
                                     volatile uint32_t *cm_base,
                                     uint32_t range, uint32_t duty)
{
    /* 1. Stop PWM channel 1 */
    pwm_base[PWM_CTL_OFFSET] = 0u;

    /* 2. Stop CM PWM clock: kill it */
    cm_base[CM_PWMCTL_OFFSET] = CM_PASSWD | CM_PWMCTL_KILL;

    /* 3. Wait until clock is not busy */
    while (cm_base[CM_PWMCTL_OFFSET] & CM_PWMCTL_BUSY)
        ;

    /*
     * 4. Set divisor: PLLD (500 MHz) / 5 = 100 MHz PWM clock.
     *    DIV register: bits 23:12 = integer part, bits 11:0 = fractional.
     *    DIVI=5 => 0x00005000 | passwd
     */
    cm_base[CM_PWMDIV_OFFSET] = CM_PASSWD | (5u << 12);

    /* 5. Select PLLD source and enable */
    cm_base[CM_PWMCTL_OFFSET] = CM_PASSWD | CM_PWMCTL_ENAB | CM_PWMCTL_SRC_PLLD;

    /* 6. Wait for clock to lock */
    while (!(cm_base[CM_PWMCTL_OFFSET] & CM_PWMCTL_BUSY))
        ;

    /* 7. Set PWM range and duty */
    pwm_base[PWM_RNG1_OFFSET] = range;
    pwm_base[PWM_DAT1_OFFSET] = duty;

    /* 8. Enable PWM channel 1 in M/S mode */
    pwm_base[PWM_CTL_OFFSET] = PWM_CTL_MSEN1 | PWM_CTL_PWEN1;
}

int rafaelia_m031_pwm_por_clock_manager(volatile uint32_t *mmio_base)
{
    if (!mmio_base) return 0; /* TOKEN_VAZIO — no BCM hardware present */

#if defined(RASPBERRYPI)
    /*
     * On real hardware the caller must separately map CM (0x3F101000).
     * Here we use mmio_base as pwm_base and derive cm_base from a known
     * physical offset. In a proper integration, pass both bases.
     *
     * PWM range=1024, duty=512 → 50% duty cycle.
     */
    volatile uint32_t *cm_base = mmio_base - ((0x3F20C000u - 0x3F101000u) / 4u);
    rafaelia_m031_pwm_setup(mmio_base, cm_base, 1024u, 512u);
    return 0;
#else
    /* Logic complete; hardware not present on this host. */
    (void)rafaelia_m031_pwm_setup;
    return 0;
#endif
}
