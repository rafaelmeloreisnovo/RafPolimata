# Hardware-capability mathematics — execution, not mythology

**Governance:** `CLOSURE_L12` for runtime/device evidence; `TOKEN_VAZIO` never means numeric zero.

The math in this directory only models quantities that can be mapped to machine evidence.

## 1. Logical masked update

For current word `x`, desired word `v`, mask `m`:

```text
x' = x XOR ((x XOR v) AND m)
```

Only selected bits change semantically. Physical memory/storage can still rewrite a byte, word, cache line, page or erase block.

## 2. Vector capacity

```text
lanes = floor(register_bits / element_bits)
```

This is a capacity bound, not an instructions-per-cycle claim.

## 3. Container/block fit

```text
fit = floor(container_bytes / block_bytes)
```

Useful for cache-line packing, ZIPRAF chunking and fixed-size crypto blocks.

## 4. Write amplification

```text
A_write = physical_write_granule / semantic_bytes_changed
```

The C/Rust API returns Q16.16. A 1-byte logical change on a 64-byte granule yields 64.0x amplification.

## 5. Measured backend selection

For compatible candidate `i`:

```text
cost_i = p95_cycles_i / useful_bytes_i
select argmin(cost_i)
```

A backend without a `PASS` measurement is not selected merely because the ISA is theoretically wider. Ties are deterministic by backend ID.

## 6. Safety boundary

Performance scoring cannot promote cryptographic correctness. Selection occurs only after KAT/equivalence and provider gates. Architecture optimizations change implementation paths, not algorithm definitions.

## 7. Authorial integer division boundary

The C core avoids ordinary integer division in the hardware-selection path.
`zh_udiv32_authorial` and `zh_udiv64_authorial` use shift/subtract long
division so 32-bit targets do not silently acquire compiler runtime helpers such
as `__aeabi_uidiv` or 64-bit libgcc division helpers.

The exact-head CI compiles the pure C sources for six OS-neutral ISA targets and
rejects undefined symbols. This is a source/object property, not physical
execution evidence.

## 8. PREHOT / HOT benchmark mathematics

The benchmark core requires 31 valid observations per promoted phase and uses
fixed order-statistic positions:

```text
min=0, p05=1, median=15, p95=29, max=30
```

No mean, floating point or hidden division is necessary for the canonical
summary.

The PREHOT/HOT relation is based on the median only:

```text
hot_median < prehot_median  -> HOT_FASTER
hot_median = prehot_median  -> EQUAL
hot_median > prehot_median  -> HOT_SLOWER
missing phase               -> TOKEN_VAZIO
```

The compact receipt stores the speed relation as the exact rational pair
`prehot_median / hot_median`; interpretation across incompatible units or speed
groups is forbidden.

## 9. Interference does not equal causality

Thirty-two numbered mechanisms are grouped into toolchain, ISA,
microarchitecture, memory, power/thermal, OS/scheduler, virtualization,
observer/I/O and physical/external layers.

A factor has its own three-state vocabulary:

```text
UNKNOWN != INACTIVE != ACTIVE
```

Observed timing changes are not assigned to an ACTIVE factor without a stronger
intervention design. See `PURE_BENCHMARK_LAB_V1.md`.

