# Canonical first-cut core

`include/raf_fs_core.h` is the canonical L0 header for this branch.

The first macro-shell draft was consolidated in place. The canonical file now avoids the conventional source-level `do { } while (0)` shell and uses GCC/Clang statement expressions for fixed-width primitives.

Reason: although compilers normally eliminate the shell, RAFAELIA L0 keeps the source representation aligned with the no-synthetic-loop invariant as well as the generated binary.
