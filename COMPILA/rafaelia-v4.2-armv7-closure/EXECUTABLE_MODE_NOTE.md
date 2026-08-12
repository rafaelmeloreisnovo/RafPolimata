# Executable mode note

The V4.2 transport files are written append-only through the GitHub contents API. The final V4.2 tree commit promotes only `MATERIALIZE_AND_VERIFY.sh` and `RUN_OMEGA_ON_DEVICE.sh` from mode `100644` to `100755` while preserving their blob SHA identities.
