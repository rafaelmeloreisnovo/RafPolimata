# RAFDB RAC / replication coordination boundary

RAC is a *planned* RAFAELIA replication/availability coordinator; it is **not** Oracle Real Application Clusters. No consensus, failover, multi-writer ordering, leader election, durable recovery, quorum or distributed transactions implemented.

Required promotion gates: exact replay at two nodes, deterministic conflict policy, fencing/term, crash consistency, network partition and duplicate/reordered record tests, durability and rollback receipts. Until these pass: TOKEN_VAZIO.
