fn rotl32(x: u32, comptime n: u5) u32 {
    return (x << n) | (x >> (32 - n));
}

export fn rhf_zig_mix32(a0: u32, b0: u32, c0: u32) callconv(.C) u32 {
    var a = a0;
    var b = b0;
    var c = c0;
    a +%= b +% 0x9E3779B9;
    c ^= a;
    c = rotl32(c, 16);
    b +%= c;
    a ^= b;
    a = rotl32(a, 12);
    return a ^ c ^ rotl32(b, 7);
}

export fn rhf_zig_lanes32(vector_bits: u16) callconv(.C) u8 {
    return if (vector_bits == 512) 16 else if (vector_bits == 256) 8 else if (vector_bits == 128) 4 else 1;
}
