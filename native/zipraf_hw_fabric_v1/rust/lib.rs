// RAFCODE-IP-NOTICE
// Copyright (c) 2026 Rafael Melo Reis.
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
// Berne Convention orientation: provenance notice only; applicable law controls.
// Module scope: LICENSE_SCOPE_V1.json. Third-party rights remain separate.

#![no_std]

pub const ABI_VERSION: u32 = 0x0001_0000;

pub const CAP_SCALAR: u32 = 1 << 0;
pub const CAP_ASM: u32 = 1 << 1;
pub const CAP_SIMD128: u32 = 1 << 2;
pub const CAP_SIMD256: u32 = 1 << 3;
pub const CAP_SIMD512: u32 = 1 << 4;

pub const PERM_READ: u64 = 1 << 0;
pub const PERM_WRITE: u64 = 1 << 1;
pub const PERM_EXEC: u64 = 1 << 2;
pub const PERM_VERIFY: u64 = 1 << 3;
pub const PERM_SIGN: u64 = 1 << 4;
pub const PERM_DERIVE: u64 = 1 << 5;
pub const PERM_PRIVATE: u64 = 1 << 8;

#[repr(u16)]
#[derive(Clone, Copy, Eq, PartialEq)]
pub enum Primitive {
    Sha256 = 1, Sha512 = 2, Sha3_256 = 3, Blake2s = 4, Blake2b = 5,
    Blake3 = 6, ChaCha20 = 7, Poly1305 = 8, XChaCha20Poly1305 = 9,
    Ed25519 = 10, X25519 = 11, Crc32c = 12, Md5Legacy = 13,
}

#[repr(u16)]
#[derive(Clone, Copy, Eq, PartialEq)]
pub enum Operation {
    Hash = 1, StreamXor = 2, Mac = 3, Seal = 4, Open = 5,
    Sign = 6, Verify = 7, Kex = 8, Checksum = 9,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct Profile {
    pub abi_version: u32,
    pub arch: u16,
    pub vector_bits: u16,
    pub native_word_bits: u16,
    pub max_parallel: u16,
    pub capability_bits: u32,
    pub allow_bits: u64,
    pub deny_bits: u64,
}

#[derive(Clone, Copy, Eq, PartialEq)]
pub enum PlanError { Invalid, Denied, TokenVazio }

pub const fn effective_permissions(allow: u64, deny: u64) -> u64 { allow & !deny }

pub const fn required_for(op: Operation) -> u64 {
    match op {
        Operation::Hash | Operation::Checksum => PERM_READ | PERM_EXEC,
        Operation::StreamXor | Operation::Seal | Operation::Open =>
            PERM_READ | PERM_WRITE | PERM_EXEC | PERM_PRIVATE,
        Operation::Mac => PERM_READ | PERM_EXEC | PERM_PRIVATE,
        Operation::Sign => PERM_READ | PERM_EXEC | PERM_SIGN | PERM_PRIVATE,
        Operation::Verify => PERM_READ | PERM_EXEC | PERM_VERIFY,
        Operation::Kex => PERM_READ | PERM_EXEC | PERM_DERIVE | PERM_PRIVATE,
    }
}

pub const fn parallel_lanes(p: &Profile, primitive_word_bits: u16, parallel_safe: bool) -> u16 {
    if !parallel_safe || primitive_word_bits == 0 || p.vector_bits == 0 { return 1; }
    let mut lanes = p.vector_bits / primitive_word_bits;
    if lanes == 0 { lanes = 1; }
    if p.max_parallel != 0 && lanes > p.max_parallel { lanes = p.max_parallel; }
    lanes
}

pub const fn authorize(p: &Profile, op: Operation, extra: u64) -> Result<(), PlanError> {
    if p.abi_version != ABI_VERSION || (p.capability_bits & CAP_SCALAR) == 0 {
        return Err(PlanError::TokenVazio);
    }
    let need = required_for(op) | extra;
    let effective = effective_permissions(p.allow_bits, p.deny_bits);
    if (effective & need) != need { return Err(PlanError::Denied); }
    Ok(())
}
