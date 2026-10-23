# Hardware MMIO fail-closed gate — V1

**Authority:** RafPolimata is the code producer. This is a source/host-mock
safety patch, not confirmation of execution on an Arduino or Raspberry Pi.

## P0/P1 changes
- M031's original single PWM mapping is no longer interpreted as an MMIO
  mapping of the independent Clock Manager. Legacy entrypoint **does not
  modify hardware** and returns `RAF_HW_ERR_CM_REQUIRED` on RPi.
- Explicit `rafaelia_m031_pwm_configure_mapped(pwm_base, cm_base, range, duty)`
  requires two independently mapped, verified blocks (minimum 0x1000 bytes).
  It rejects invalid input and limits clock waits. Clock ownership and board
  model must be checked **by the caller** before using it physically.
- M029 SPI0 and M030 BSC I2C waits are bounded; negative outcomes use typed
  codes, not zero. These helpers still require valid target-specific mappings.
- M016 now returns unsupported for non-AVR instead of falsely returning success.
- `RAF_hw_status.h` contains the shared negative error constants.

## Tests and claims

Run `sh tests/hardware/run_mmio_failclosed.sh` on a host with a C11 compiler.
Mocks are memory-backed arrays, not an AVR board or Raspberry Pi peripheral.
`PASS_HOST_MOCK_ONLY` is bounded negative test evidence, not CI/device proof.

**Not claimed:** physical GPIO/PWM/SPI/I2C success, actual timing deadlines,
per-board SoC address validation, Raspberry Pi 5/RP1, AVR firmware generation,
thermal/power safety, or no regressions outside this touched surface.

## Controlled next gates

1. Compile C11 host negative tests with both simulated and no-target profiles.
2. Run independent exact-head CI (failure must remain visible).
3. Verify physical SoC /dev/device-tree map, separate PWM/CM mappings, pin
   ownership, timeout calibration, and safe restoration/rollback policy.
4. Acquire physical same-artifact receipt only after separate human review.

**Rollback:** revert only this branch/PR; no upstream main mutation.
**Claim boundary:** SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM;
`IMPLEMENTED_UNTESTED != PASS`; `TOKEN_VAZIO != 0` (governance: `CLOSURE_L16`, still materially open).
