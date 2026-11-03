# ZIPRAF lower-layer adapter contract — no implicit equivalence

The live `banco/rafdb.c` layer emits optional deterministic **RDB1** records:
`ASCII RDB1 (4) | op u8 | key_len u8 | value_len u8 | generation LE64 | key bytes | value bytes | FNV1a32 LE`.
Max length 177 bytes. Operations: INSERT=1, UPDATE=2, DELETE=3.
The FNV tag is an accidental-error check only; it is **not** a cryptographic digest, MAC, signature, encryption, durable WAL, nor a custody receipt.

If `rf_hooks(..., append, ...)` configures an append callback, it is invoked **before** RAM mutation. Nonzero response blocks mutation. Zero means the caller accepted the record; the caller is responsible for durable write+fsync semantics, idempotent replay, parent generation, ZIPRAF-FS policy, content digest, security, private key handling, and verified custody receipt. No callback => volatile-only mode; do not claim persistence.

The producer source for ZIPRAF-FS is `canonical/zipraf-hw-v1/fs/ZIPRAF_FS_V1.md`: append-only successor generation, permissions, cryptographic digest chain, quarantine on mismatch. **RDB1 is an adapter feed, not a native ZIPRAF archive**. A genuine archive writer/recovery path has not been implemented or tested.
