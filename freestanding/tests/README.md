# Compile probes

`probe.c` is not part of the runtime core. It exists only to make generated object code inspectable.

`authorial_probe.c` is also compile-only. It exercises `include/raf_fs_authorial.h` by building a caller-owned item descriptor with `external_dep_count = 0` and a nonzero local fingerprint.

`verify_matrix.sh` cross-compiles the same probe for six initial targets. A pass proves only compiler acceptance of the source for those targets; it does not prove execution on hardware.

`verify_authorial.sh` cross-compiles the authorial probe for the same six OS-neutral target triples and rejects hosted headers, allocator calls, syscall instructions, inline assembly and unresolved helpers in that scope.

The probes must never be linked into production images unless explicitly requested.
