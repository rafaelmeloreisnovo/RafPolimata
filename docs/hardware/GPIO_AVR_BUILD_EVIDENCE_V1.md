# GPIO / AVR build evidence — fail-closed V1

**Scope:** M021–M023 and `Benchmark/build_mcu.sh`. Producer RafPolimata.
**Dependency:** layered on P0 MMIO PR #439, `RAF_hw_status.h` definitions.

- M021 (/dev/mem) and M022 (/dev/gpiomem) read only; unrecognized
  host returns `RAF_HW_ERR_UNSUPPORTED` without opening devices.
- M021 requires explicit RASPBERRYPI + RAFAELIA_BCM2837 compile gates;
  physical SoC + address must still be independently checked.
- M023 is disabled for physical writes unless the dedicated
  `RAFAELIA_ALLOW_GPIO4_PHYSICAL_WRITE` macro is enabled along with the
  BCM2837 target. The flag **does not** provide safety authorization,
  independent proof, or automatic restoration. NEVER enable by default.
- OS open/mmap/cleanup errors no longer return false success.
- AVR build now uses `avr-gcc -c` and produces object files, not pretend
  executable `.elf` without `main`; absence of AVR toolchain is exit 2.
  `COMPILE_OK != LINK_OK != FLASH_OK != DEVICE_OK`.

Run `sh tests/hardware/run_gpio_stubs.sh` for bounded host safety check.
Hardware register-map review, firmware packaging, original pin state
restoration, electrical safety, and full CI/provider enforcement remain
separate gates. No module is claimed executable on physical hardware.
