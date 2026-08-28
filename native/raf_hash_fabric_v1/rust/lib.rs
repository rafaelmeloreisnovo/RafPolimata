#![no_std]

#[inline(always)]
fn rotl32(x: u32, n: u32) -> u32 { x.rotate_left(n) }

#[no_mangle]
pub extern "C" fn rhf_rust_mix32(mut a: u32, mut b: u32, mut c: u32) -> u32 {
    a = a.wrapping_add(b).wrapping_add(0x9E37_79B9);
    c ^= a;
    c = rotl32(c, 16);
    b = b.wrapping_add(c);
    a ^= b;
    a = rotl32(a, 12);
    a ^ c ^ rotl32(b, 7)
}

#[no_mangle]
pub extern "C" fn rhf_rust_lanes32(vector_bits: u16) -> u8 {
    match vector_bits {
        512 => 16,
        256 => 8,
        128 => 4,
        _ => 1,
    }
}
