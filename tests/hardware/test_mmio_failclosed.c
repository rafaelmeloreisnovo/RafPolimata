/* Host-only fake MMIO: never execute this test against /dev/mem or GPIO. */
#include <assert.h>
#include <stdint.h>
#include "../../RAF_hw_status.h"

int rafaelia_m029_spi_por_registrador_bcm(volatile uint32_t *);
int rafaelia_m030_i2c_por_registrador_bcm(volatile uint32_t *);
int rafaelia_m031_pwm_por_clock_manager(volatile uint32_t *);
int rafaelia_m031_pwm_configure_mapped(volatile uint32_t *,
                                       volatile uint32_t *, uint32_t, uint32_t);
int rafaelia_m016_i2c_twi_com_timeout(void);

int main(void)
{
    volatile uint32_t spi[64] = {0};
    volatile uint32_t bsc[64] = {0};
    volatile uint32_t pwm[64] = {0};
    volatile uint32_t cm[64] = {0};
    pwm[0] = 0x5a5au;
    assert(rafaelia_m029_spi_por_registrador_bcm((volatile uint32_t *)0) ==
           RAF_HW_ERR_NO_DEVICE);
    assert(rafaelia_m030_i2c_por_registrador_bcm((volatile uint32_t *)0) ==
           RAF_HW_ERR_NO_DEVICE);
    assert(rafaelia_m031_pwm_por_clock_manager((volatile uint32_t *)0) ==
           RAF_HW_ERR_NO_DEVICE);
    assert(rafaelia_m016_i2c_twi_com_timeout() ==
           RAF_HW_ERR_UNSUPPORTED);
#if defined(RASPBERRYPI)
    assert(rafaelia_m031_pwm_por_clock_manager(pwm) ==
           RAF_HW_ERR_CM_REQUIRED);
    assert(pwm[0] == 0x5a5au); /* legacy API must be side-effect free */
    assert(rafaelia_m031_pwm_configure_mapped(pwm, cm, 0, 0) ==
           RAF_HW_ERR_INVALID);
    assert(rafaelia_m031_pwm_configure_mapped(pwm, pwm, 1024, 512) ==
           RAF_HW_ERR_INVALID);
    assert(rafaelia_m029_spi_por_registrador_bcm(spi) ==
           RAF_HW_ERR_TIMEOUT); /* no TXD */
    assert(rafaelia_m030_i2c_por_registrador_bcm(bsc) != RAF_HW_OK);
    assert(rafaelia_m031_pwm_configure_mapped(pwm, cm, 1024, 512) ==
           RAF_HW_ERR_TIMEOUT); /* CM never asserts BUSY */
    assert((pwm[0] & 1u) == 0u); /* output not enabled */
#else
    assert(rafaelia_m031_pwm_por_clock_manager(pwm) ==
           RAF_HW_ERR_UNSUPPORTED);
    assert(rafaelia_m029_spi_por_registrador_bcm(spi) ==
           RAF_HW_ERR_UNSUPPORTED);
    assert(rafaelia_m030_i2c_por_registrador_bcm(bsc) ==
           RAF_HW_ERR_UNSUPPORTED);
    assert(rafaelia_m031_pwm_configure_mapped(pwm, cm, 1024, 512) ==
           RAF_HW_ERR_UNSUPPORTED);
#endif
    return 0;
}
