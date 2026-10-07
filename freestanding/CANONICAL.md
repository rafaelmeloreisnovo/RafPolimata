# Canonical first-cut core

`include/raf_fs_core.h` remains the canonical L0 hot-core header for this branch.

`include/raf_fs_authorial.h` is the canonical L0 authorial item descriptor for this branch. It records provider/source identity, a nonzero local fingerprint and `external_dep_count = 0` without hosted headers, allocation, syscall instructions, inline assembly or external helper symbols.

The first macro-shell draft was consolidated in place. The canonical core file now avoids the conventional source-level `do { } while (0)` shell and uses GCC/Clang statement expressions for fixed-width primitives.

Reason: although compilers normally eliminate the shell, RAFAELIA L0 keeps the source representation aligned with the no-synthetic-loop invariant as well as the generated binary. The authorial descriptor follows the same boundary by using a fixed four-word fold rather than scans, hidden tails or runtime lookups.
