#![no_std]

/*
ZIPRAF Hardware Capability Core V1 — Rust no_std mirror.
Provenance/license scope is file-specific; no relicensing is implied.
*/

pub const TOKEN_VAZIO: i32 = 1;

#[inline(always)]
pub const fn patch_u64(current: u64, value: u64, mask: u64) -> u64 {
    current ^ ((current ^ value) & mask)
}

#[inline(always)]
pub const fn vector_lanes(register_bits: u32, element_bits: u32) -> u32 {
    if element_bits == 0 { 0 } else { register_bits / element_bits }
}

#[inline(always)]
pub const fn block_fit(container_bytes: u32, block_bytes: u32) -> u32 {
    if block_bytes == 0 { 0 } else { container_bytes / block_bytes }
}

#[inline(always)]
pub const fn write_amplification_q16(semantic_bytes: u32, physical_granule_bytes: u32) -> u32 {
    if semantic_bytes == 0 || physical_granule_bytes == 0 { return 0; }
    let v = ((physical_granule_bytes as u64) << 16) / semantic_bytes as u64;
    if v > u32::MAX as u64 { u32::MAX } else { v as u32 }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test] fn patch() { assert_eq!(patch_u64(0xaaaa, 0x5555, 0x00ff), 0xaa55); }
    #[test] fn lanes() { assert_eq!(vector_lanes(256, 32), 8); }
    #[test] fn amp() { assert_eq!(write_amplification_q16(1, 64), 64 << 16); }
}
