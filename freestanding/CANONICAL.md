# Canonical first-cut core

`include/raf_fs_core_noloop.h` is the canonical L0 header for this branch.

The earlier `include/raf_fs_core.h` was an intermediate draft that used the conventional source-level `do { } while (0)` macro shell. It is not the canonical implementation and must not be consumed by production code.

Reason: although compilers eliminate that shell, RAFAELIA L0 keeps the source representation aligned with the no-synthetic-loop invariant as well as the generated binary.
