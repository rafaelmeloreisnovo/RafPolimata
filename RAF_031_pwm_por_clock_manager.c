#include "RAF_rafaelia_common.h"
#include "RAF_hw_status.h"

/*
 * Método M031: PWM por clock manager
 * Alvo: Raspberry Pi (BCM2835/BCM2837)
 * Domínio: PWM
 * Ganho estimado: precisão de frequência
 *
 * Configura o clock manager (CM) e PWM peripheral diretamente.
 * RPi requires explicit independent PWM and CM memory mappings.
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
/*
 * Explicit mapping API. Both pointers must refer to separate mapped regions
 * with at least 0x1000 bytes each and verified BCM variant/clock ownership.
 * PWM must not be sharing the clock with another system consumer.
 * Negative result leaves the PWM output disabled, not state-restored.
 */
int rafaelia_m031_pwm_configure_mapped(volatile uint32_t *pwm_base,
                                      volatile uint32_t *cm_base,
                                      uint32_t range, uint32_t duty)
{
#if defined(RASPBERRYPI)
    uint32_t n;
    if (!pwm_base || !cm_base || pwm_base == cm_base)
        return RAF_HW_ERR_INVALID;
    if (range == 0u || duty > range) return RAF_HW_ERR_INVALID;

    /* No writes when clock is already busy: prevent stealing ownership. */
    if (cm_base[CM_PWMCTL_OFFSET] & CM_PWMCTL_BUSY) return RAF_HW_ERR_IO;
    pwm_base[PWM_CTL_OFFSET] = 0u;
    cm_base[CM_PWMCTL_OFFSET] = CM_PASSWD | CM_PWMCTL_KILL;
    for (n = 0; n < RAF_HW_POLL_LIMIT; ++n)
        if (!(cm_base[CM_PWMCTL_OFFSET] & CM_PWMCTL_BUSY)) break;
    if (n == RAF_HW_POLL_LIMIT) return RAF_HW_ERR_TIMEOUT;

    cm_base[CM_PWMDIV_OFFSET] = CM_PASSWD | (5u << 12);
    cm_base[CM_PWMCTL_OFFSET] =
        CM_PASSWD | CM_PWMCTL_ENAB | CM_PWMCTL_SRC_PLLD;
    for (n = 0; n < RAF_HW_POLL_LIMIT; ++n)
        if (cm_base[CM_PWMCTL_OFFSET] & CM_PWMCTL_BUSY) break;
    if (n == RAF_HW_POLL_LIMIT) {
        cm_base[CM_PWMCTL_OFFSET] = CM_PASSWD | CM_PWMCTL_KILL;
        return RAF_HW_ERR_TIMEOUT;
    }

    pwm_base[PWM_RNG1_OFFSET] = range;
    pwm_base[PWM_DAT1_OFFSET] = duty;
    pwm_base[PWM_CTL_OFFSET] = PWM_CTL_MSEN1 | PWM_CTL_PWEN1;
    return RAF_HW_OK;
#else
    (void)pwm_base; (void)cm_base; (void)range; (void)duty;
    return RAF_HW_ERR_UNSUPPORTED;
#endif
}

/* Legacy one-pointer API cannot safely derive CM virtual address from PWM.
 * Fail closed without MMIO writes; callers must move to explicit mapped API.
 */
int rafaelia_m031_pwm_por_clock_manager(volatile uint32_t *mmio_base)
{
    if (!mmio_base) return RAF_HW_ERR_NO_DEVICE;
#if defined(RASPBERRYPI)
    return RAF_HW_ERR_CM_REQUIRED;
#else
    return RAF_HW_ERR_UNSUPPORTED;
#endif
}
