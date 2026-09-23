# Hardware-capability mathematics — execution, not mythology

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
