# RafPolimata SDK reference v1

This directory is the first bounded productization slice of the low-level work.

## Contract

- API version: **0.1.0**
- ABI version: **1**
- distribution form in this cut: static C library + public header;
- runtime dependencies of the library: none;
- allocator: caller-owned memory only; storage base must be 8-byte aligned and misaligned storage is rejected;
- syscall/OS ABI: none;
- hidden heap/GC/TLS: none.

The public reference implementation is intentionally conservative. Specialized internal kernels may be faster, but they must prove functional equivalence against this reference before promotion.

## Build

```sh
bash sdk/rafpolimata_v1/build.sh
./build/sdk/rafpolimata_v1/rafpolimata_v1_smoke
```

Public symbols and compatibility rules are machine-bound by `contracts/rafpolimata_api_abi_v1.json`.

This slice is an engineering SDK candidate, not a production-support or security-certification claim. Shared-library SONAME, Android AAR/APK distribution and language bindings are later product layers.

## CRC32C low-level ownership

The public `rafp_v1_crc32c` symbol and ABI 1 remain unchanged. Its pure,
allocator-free CRC32C implementation now lives in
`../../freestanding/include/raf_fs_crc32c.h`; the SDK retains only its API
boundary (including the legacy invalid-NULL return). Run
`sh freestanding/tests/verify_crc32c.sh` and the SDK/maturity gates before
promoting a release. Cross-target object compilation does not establish
physical device runtime equivalence.
