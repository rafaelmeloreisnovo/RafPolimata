#include <assert.h>
#include "../../RAF_hw_status.h"
int rafaelia_m021_gpio_por_mmap(void);
int rafaelia_m022_gpio_por_dev_gpiomem(void);
int rafaelia_m023_gpio_por_dev_mem_controlado(void);
int main(void) {
    assert(rafaelia_m021_gpio_por_mmap() == RAF_HW_ERR_UNSUPPORTED);
    assert(rafaelia_m022_gpio_por_dev_gpiomem() == RAF_HW_ERR_UNSUPPORTED);
    assert(rafaelia_m023_gpio_por_dev_mem_controlado() == RAF_HW_ERR_UNSUPPORTED);
    return 0;
}
