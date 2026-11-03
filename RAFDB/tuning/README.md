# RAFDB tuning, benchmarks, and 16XOR hypothesis

Focus: deterministic capacity, operation latency (ns/op), p50/p95/p99, throughput, memory, write amplification, persisted bytes/row, tail latency and energy on identified hardware.

Baseline competitors: MySQL and Oracle **when their environment, licenses, dataset, configuration, concurrency and workload are documented**. Never extrapolate from a single in-memory C lookup to a complete RDBMS. Same data/transaction semantics and durable commits are required for comparative claims.

`16XOR` is the private **experiment profile/codename**. Hypothesis: RAFDB is 16 times faster on a pre-registered operation family. Until paired measurements, CI+physical run, statistical intervals, and equivalent durability: `claim_allowed=false`, performance=TOKEN_VAZIO. XOR is not a security primitive by itself; never call it encryption.
