// Build intent: ldc2 -betterC -O3 -c raf_hash_fabric.d
module raf_hash_fabric;

alias u16 = ushort;
alias u32 = uint;

private u32 rotl32(u32 x, u32 n) @nogc nothrow pure @safe {
    return (x << n) | (x >> (32u - n));
}

extern(C) u32 rhf_d_mix32(u32 a, u32 b, u32 c) @nogc nothrow {
    a += b + 0x9E3779B9u;
    c ^= a;
    c = rotl32(c, 16u);
    b += c;
    a ^= b;
    a = rotl32(a, 12u);
    return a ^ c ^ rotl32(b, 7u);
}

extern(C) ubyte rhf_d_lanes32(u16 vectorBits) @nogc nothrow {
    return cast(ubyte)(vectorBits == 512u ? 16u :
                       vectorBits == 256u ? 8u :
                       vectorBits == 128u ? 4u : 1u);
}
