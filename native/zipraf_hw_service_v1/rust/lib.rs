#![no_std]

pub const TOKEN_VAZIO_U32: u32 = u32::MAX;
pub const Q16_ONE: u32 = 65_536;

#[repr(u16)]
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum AlgorithmId {
    Sha256 = 1, Sha512, Sha3_256, Blake2s256, Blake2b512, Blake3_256,
    ChaCha20, ChaCha20Poly1305, Ed25519, HmacSha256, Aes128Gcm, Aes256Gcm, Md5,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct WeightsQ16 {
    pub latency: u32,
    pub cycles: u32,
    pub throughput_inverse: u32,
    pub code_size: u32,
    pub stack: u32,
    pub energy: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct NormCostQ16 {
    pub latency: u32,
    pub cycles: u32,
    pub throughput_inverse: u32,
    pub code_size: u32,
    pub stack: u32,
    pub energy: u32,
    pub complete_mask: u8,
}

pub const fn weights_valid(w: &WeightsQ16) -> bool {
    (w.latency as u64 + w.cycles as u64 + w.throughput_inverse as u64 +
     w.code_size as u64 + w.stack as u64 + w.energy as u64) == Q16_ONE as u64
}

pub fn score_q16(c: &NormCostQ16, w: &WeightsQ16, required_mask: u8) -> u32 {
    if !weights_valid(w) || (c.complete_mask & required_mask) != required_mask { return TOKEN_VAZIO_U32; }
    let v = [c.latency,c.cycles,c.throughput_inverse,c.code_size,c.stack,c.energy];
    if v.iter().any(|x| *x > Q16_ONE) { return TOKEN_VAZIO_U32; }
    let ww = [w.latency,w.cycles,w.throughput_inverse,w.code_size,w.stack,w.energy];
    let mut acc: u64 = 0;
    let mut i = 0;
    while i < 6 { acc += v[i] as u64 * ww[i] as u64; i += 1; }
    (acc >> 16) as u32
}
