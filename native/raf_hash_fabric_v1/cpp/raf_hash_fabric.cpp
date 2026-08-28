// C++20 freestanding subset backend. No STL, exceptions, RTTI or allocation.
using u16 = unsigned short;
using u32 = unsigned int;

static constexpr u32 rotl32(u32 x, u32 n) noexcept {
    return (x << n) | (x >> (32u - n));
}

extern "C" u32 rhf_cpp_mix32(u32 a, u32 b, u32 c) noexcept {
    a += b + 0x9E3779B9u;
    c ^= a;
    c = rotl32(c, 16u);
    b += c;
    a ^= b;
    a = rotl32(a, 12u);
    return a ^ c ^ rotl32(b, 7u);
}

extern "C" unsigned char rhf_cpp_lanes32(u16 vector_bits) noexcept {
    return static_cast<unsigned char>(vector_bits == 512u ? 16u :
                                      vector_bits == 256u ? 8u :
                                      vector_bits == 128u ? 4u : 1u);
}
