# BBS-style operator surface

`tools/zipraf_bbs.c` intentionally uses an ANSI terminal look inspired by BBS-era software while keeping the control plane simple and auditable.

The visual shell is not a security boundary. Every action must map to a typed operation and an evidence receipt.

Canonical installation flow:

```text
DETECT -> DECLARE -> VERIFY TOOLCHAIN -> BUILD -> SELFTEST -> INSTALL -> RECEIPT
```

No installer may silently fetch executable code. Network acquisition, if later added, must be a separate explicit step with source URL/ref, expected digest and license metadata.

Suggested installed layout:

```text
bin/zipraf-bbs
lib/zipraf-hw/<arch>/
share/zipraf/registry/crypto_algorithms.json
share/zipraf/policy/ZIPRAF_FS_V1.md
var/lib/zipraf/receipts/
```
