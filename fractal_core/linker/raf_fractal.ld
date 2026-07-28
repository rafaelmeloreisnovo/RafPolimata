/* Callable ROM image, not an OS executable. Entry is a library function. */
ENTRY(raf_fractal_step)
SECTIONS
{
    . = 0;
    .text : ALIGN(16) {
        KEEP(*(.text.raf.*))
        *(.text*)
    }
    .rodata : ALIGN(16) { *(.rodata*) }
    .data : ALIGN(16) { *(.data*) }
    .bss (NOLOAD) : ALIGN(16) { *(.bss*) *(COMMON) }
    /DISCARD/ : {
        *(.comment*)
        *(.note*)
        *(.eh_frame*)
        *(.eh_frame_hdr*)
        *(.gnu*)
    }
}
ASSERT(SIZEOF(.bss) == 0, "fractal core must not own mutable global state")
