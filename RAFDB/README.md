# RAFDB V0 — ZIPRAF-backed architecture (implementation boundary)

RAFAELIA / RafPolimata bounded experimental database. Maintains a small working C11 core while preserving the existing ZIPRAF-FS custody authority rather than copying/relicensing it.

| Directory | Responsibility | Current evidence |
|---|---|---|
| `client/` | caller/API route | documented |
| `banco/` | 64-row RAM KV core, triggers BEFORE/AFTER, RDB1 append callback | source implemented, gate pending |
| `gateway/` | strict SQL-like subset (INSERT/SELECT/UPDATE/DELETE) | source implemented, **not MySQL wire** |
| `rac/` | future replication/HA coordinator | TOKEN_VAZIO |
| `tuning/` | benchmark gates | TOKEN_VAZIO |
| `zipraf/` | RDB1→ZIPRAF-FS adapter boundary | documented; real ZIPRAF persistence TOKEN_VAZIO |
| `tests/` | basic failure, rollback and SQL subset checks | `sh RAFDB/tests/run.sh` |

Memory: fixed 64 slots, key 1–31 bytes and value 1–127 bytes. Single-process and single-writer, no allocator/libc in L0 core. Records are emitted before mutation; a nonzero append result aborts. Triggers are synchronous; BEFORE may veto, AFTER is notification only. Callback reentry into writes is denied. No persistence without an external adapter; even a successful callback is not itself proof of durability.

**Safety:** no production deployment, no real MySQL protocol, no RAC, no ACID claim, no ZIPRAF archive writer/recovery and no 16× Oracle measurement. `SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`; `claim_allowed=false`.

**Path of authority:** `canonical/zipraf-hw-v1/fs/ZIPRAF_FS_V1.md` in RafPolimata. The private `16XOR` experiment lives separately in `rafaelmeloreisnovo/Rafaelia_Private/RAFDB_16XOR`; no secret or private body is mirrored here.

**Rollback:** revert the PR/commit; none of the existing sources, persisted datasets, build pipelines or license files are changed. Validate on current commit before promotion.
