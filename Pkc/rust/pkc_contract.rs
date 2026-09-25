#![no_std]

/*
 * PKC Rust contract mirror V1.\n * Governance: CLOSURE_L2 (target/runtime evidence).
 *
 * No external crates. This is NOT yet a claim that every Rust final artifact
 * is runtime-free on every target. Final strict Rust parity remains
 * TOKEN_VAZIO_RUST_FINAL until object-level gates prove the selected target.
 */

pub const PKC_REQ_NO_HEAP: u64             = 1u64 << 0;
pub const PKC_REQ_NO_EXTERNAL_RUNTIME: u64 = 1u64 << 1;
pub const PKC_REQ_NO_LIBC: u64             = 1u64 << 2;
pub const PKC_REQ_NO_LIBM: u64             = 1u64 << 3;
pub const PKC_REQ_NO_SYSCALL: u64          = 1u64 << 4;
pub const PKC_REQ_NO_TAILCALL: u64         = 1u64 << 5;
pub const PKC_REQ_NO_SHADOW: u64           = 1u64 << 6;
pub const PKC_REQ_NO_UNWIND: u64           = 1u64 << 7;
pub const PKC_REQ_ZERO_UNDEFINED: u64      = 1u64 << 8;
pub const PKC_REQ_CALLER_BUFFER: u64       = 1u64 << 9;
pub const PKC_REQ_NO_DUP_KERNEL: u64       = 1u64 << 10;
pub const PKC_REQ_DETERMINISTIC: u64       = 1u64 << 11;

pub const PKC_STRICT_REQUIREMENTS: u64 =
    PKC_REQ_NO_HEAP |
    PKC_REQ_NO_EXTERNAL_RUNTIME |
    PKC_REQ_NO_LIBC |
    PKC_REQ_NO_LIBM |
    PKC_REQ_NO_SYSCALL |
    PKC_REQ_NO_TAILCALL |
    PKC_REQ_NO_SHADOW |
    PKC_REQ_NO_UNWIND |
    PKC_REQ_ZERO_UNDEFINED |
    PKC_REQ_CALLER_BUFFER |
    PKC_REQ_NO_DUP_KERNEL |
    PKC_REQ_DETERMINISTIC;

#[no_mangle]
pub extern "C" fn pkc_rust_contract_missing(observed: u64) -> u64 {
    PKC_STRICT_REQUIREMENTS & !observed
}
