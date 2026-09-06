# Compile probes

`probe.c` is not part of the runtime core. It exists only to make generated object code inspectable.

`verify_matrix.sh` cross-compiles the same probe for six initial targets. A pass proves only compiler acceptance of the source for those targets; it does not prove execution on hardware.

The probe must never be linked into production images unless explicitly requested.
