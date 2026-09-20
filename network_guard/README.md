# RAFCODEΦ Network Guard Lite Contract V1

Status: `IMPLEMENTED_UNTESTED_PROVIDER` on this branch until CI/local execution is bound to the exact commit.

This directory is the low-level policy/evidence core for a defensive network guard. It is intentionally not a packet sniffer, injector, root firewall, Frida clone, or VPN implementation.

## Contract

The engine receives already-observed tuples from an authorized adapter and returns a deterministic decision.

```text
OBSERVE -> NORMALIZE -> DECIDE -> ENFORCE(adapter) -> OBSERVE_POST -> RECEIPT
```

The core itself performs no allocation, syscalls, DNS, file IO, socket IO, process attachment or payload capture.

### Invariants

- default production policy SHOULD be `DENY` unless an explicit allow rule matches;
- the enforcement adapter MUST run before credential or sensitive payload release;
- logs are metadata-only by default;
- `pre tuple` and `post tuple` are distinct objects;
- NAT/PAT is asserted only when both tuples are independently observable;
- if post-translation state is not observable, NAT/PAT state is `TOKEN_VAZIO`, never guessed;
- UDP is represented as protocol 17 and is not treated as a connected TCP session;
- rules are first-match, ordered, deterministic;
- ports are host byte order inside the engine; adapters normalize network byte order;
- IPv4 occupies address bytes 0..3 and zeros the remaining bytes;
- no claim of device-wide coverage follows from a userspace adapter.

## Event minimum

`raf_ng_event` carries:

- sequence and phase;
- adapter identifier and direction;
- pre/post 5-tuples;
- rule/decision;
- NAT/PAT transform flags;
- byte count;
- optional caller-supplied SHA-256 or BLAKE3-256 digest;
- error code.

The hash slot is for a digest supplied by an authorized adapter. The core does not retain packet bodies.

## Build/self-test

```sh
cc -std=c11 -Wall -Wextra -Werror \
  raf_netguard.c test_raf_netguard.c \
  -o raf_netguard_test
./raf_netguard_test
```

Expected output:

```text
RAF_NETGUARD_SELFTEST PASS
```

## Adapter capability matrix

| Adapter | Observe own process | Block | Device-wide | NAT/PAT post tuple |
|---|---:|---:|---:|---:|
| OkHttp interceptor | yes | yes | no | no |
| LD_PRELOAD / libc wrapper | child/dynamic only | yes | no | usually no |
| strace/ptrace child tracer | yes | observe-first | no | usually no |
| Android VpnService | selected device traffic | yes | user-consented VPN scope | userspace translation only if implemented |
| Frida debug adapter | attached authorized process | script-dependent | no | layer-dependent |
| kernel netfilter/eBPF | platform-dependent/root/privileged | yes | potentially | yes |

No adapter should claim a stronger row than its measured environment permits.

## R3

- F_ok: deterministic tuple policy and transformation classification are source-materialized.
- F_gap: exact-commit build/run, Android adapters, DNS binding and append-only receipt sink remain separate gates.
- F_next: bind one adapter at a time and require a pre/post receipt before promotion.
