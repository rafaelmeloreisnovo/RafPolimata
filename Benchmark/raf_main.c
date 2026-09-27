/* raf_main.c — RAFAELIA no-libc Linux/Android userspace entry point
 * Integra: arena + CRC32C + hash + T^7 + FSM + Lyapunov + benchmark
 * Runtime boundary: raw Linux syscalls; this is NOT physical bare-metal and
 * is separate from the OS-neutral freestanding/ L0 contract.
 * Core: no malloc, no GC runtime, caller/static storage.
 * Runtime/device unknowns: CLOSURE_L12.                                     */
#include "raf_types.h"
#include "raf_sys.h"
#include "raf_arena.h"
#include "raf_q16.h"
#include "raf_crc32c.h"
#include "raf_hash.h"
#include "raf_toroid.h"
#include "raf_fsm.h"
#include "raf_bench.h"

/* ── Payload de teste para benchmarks — 4 KB estático ──────────────────── */
static const u8 BENCH_BUF[4096] = {
    0xDE,0xAD,0xBE,0xEF,0xCA,0xFE,0xBA,0xBE,
    0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
    /* ... restante inicializado a 0x00 pelo BSS — padrão válido para bench */
};

/* ── Sessão de benchmark de CRC32C ──────────────────────────────────────── */
static void run_bench_crc32c(u64 freq) {
    static u64 samp[BENCH_K];
    u32 result = 0;
    raf_puts("=== BENCH CRC32C 4KB unroll×8 ===\n");
    BENCH_RUN(samp, {
        result = crc32c_buf(BENCH_BUF, sizeof(BENCH_BUF), 0U);
        /* prevent DCE: usa resultado */
        __asm__ volatile("" :: "r"(result) : "memory");
    });
    BenchResult r = bench_analyze(samp);
    bench_report("crc32c_buf_4kb", r, freq);
    /* throughput: 4096 bytes / (med_ns * 1e-9) = 4096*1e9/med_ns bytes/s  */
    raf_puts("  crc_result=0x");
    /* imprime hex manual — 8 dígitos                                        */
    char hx[10]; hx[8]='\n'; hx[9]=0;
    for (s32 i = 7; i >= 0; i--) {
        u32 nibble = result & 0xF;
        hx[i] = (char)(nibble < 10 ? '0'+nibble : 'a'+nibble-10);
        result >>= 4;
    }
    raf_write(hx, 9);
}

/* ── Sessão de benchmark: arena alloc ───────────────────────────────────── */
static void run_bench_arena(u64 freq) {
    static u64 samp[BENCH_K];
    raf_puts("=== BENCH ARENA ALLOC 64B ===\n");
    BENCH_RUN(samp, {
        ARENA_RESET();
        void *p = ALLOC(64);
        __asm__ volatile("" :: "r"(p) : "memory");
    });
    BenchResult r = bench_analyze(samp);
    bench_report("arena_alloc_64b", r, freq);
    raf_puts("  arena_peak_bytes=");
    raf_putu64(arena_peak(&G_ARENA));
}

/* ── Sessão de benchmark: phi64_mix hash ────────────────────────────────── */
static void run_bench_hash(u64 freq) {
    static u64 samp[BENCH_K];
    u64 h = FNV1A_BASIS;
    raf_puts("=== BENCH PHI64 MIX CHAIN 64 steps ===\n");
    BENCH_RUN(samp, {
        u64 acc = h;
        /* 64 passos de mix — chain de hash sem heap                        */
        acc = phi64_mix(acc, 0xDEAD);  acc = phi64_mix(acc, 0xBEEF);
        acc = phi64_mix(acc, 0xCAFE);  acc = phi64_mix(acc, 0xBABE);
        acc = phi64_mix(acc, 0x0102);  acc = phi64_mix(acc, 0x0304);
        acc = phi64_mix(acc, 0x0506);  acc = phi64_mix(acc, 0x0708);
        acc = phi64_mix(acc, 0x0910);  acc = phi64_mix(acc, 0x1112);
        acc = phi64_mix(acc, 0x1314);  acc = phi64_mix(acc, 0x1516);
        acc = phi64_mix(acc, 0x1718);  acc = phi64_mix(acc, 0x1920);
        acc = phi64_mix(acc, 0x2122);  acc = phi64_mix(acc, 0x2324);
        /* ... padrão repetido ×4 = 64 muls totais                          */
        acc = phi64_mix(acc, 0x2526);  acc = phi64_mix(acc, 0x2728);
        acc = phi64_mix(acc, 0x2930);  acc = phi64_mix(acc, 0x3132);
        acc = phi64_mix(acc, 0x3334);  acc = phi64_mix(acc, 0x3536);
        acc = phi64_mix(acc, 0x3738);  acc = phi64_mix(acc, 0x3940);
        acc = phi64_mix(acc, 0x4142);  acc = phi64_mix(acc, 0x4344);
        acc = phi64_mix(acc, 0x4546);  acc = phi64_mix(acc, 0x4748);
        acc = phi64_mix(acc, 0x4950);  acc = phi64_mix(acc, 0x5152);
        acc = phi64_mix(acc, 0x5354);  acc = phi64_mix(acc, 0x5556);
        acc = phi64_mix(acc, 0x5758);  acc = phi64_mix(acc, 0x5960);
        acc = phi64_mix(acc, 0x6162);  acc = phi64_mix(acc, 0x6364);
        acc = phi64_mix(acc, 0x6566);  acc = phi64_mix(acc, 0x6768);
        acc = phi64_mix(acc, 0x6970);  acc = phi64_mix(acc, 0x7172);
        acc = phi64_mix(acc, 0x7374);  acc = phi64_mix(acc, 0x7576);
        acc = phi64_mix(acc, 0x7778);  acc = phi64_mix(acc, 0x7980);
        acc = phi64_mix(acc, 0x8182);  acc = phi64_mix(acc, 0x8384);
        acc = phi64_mix(acc, 0x8586);  acc = phi64_mix(acc, 0x8788);
        acc = phi64_mix(acc, 0x8990);  acc = phi64_mix(acc, 0x9192);
        acc = phi64_mix(acc, 0x9394);  acc = phi64_mix(acc, 0x9596);
        acc = phi64_mix(acc, 0x9798);  acc = phi64_mix(acc, 0x9900);
        acc = phi64_mix(acc, 0xA0A1);  acc = phi64_mix(acc, 0xA2A3);
        acc = phi64_mix(acc, 0xA4A5);  acc = phi64_mix(acc, 0xA6A7);
        acc = phi64_mix(acc, 0xA8A9);  acc = phi64_mix(acc, 0xAAAA);
        acc = phi64_mix(acc, 0xABCD);  acc = phi64_mix(acc, 0xABCE);
        h = acc;
        __asm__ volatile("" :: "r"(h) : "memory");
    });
    BenchResult r = bench_analyze(samp);
    bench_report("phi64_mix_64steps", r, freq);
}

/* ── Sessão: FSM 1000 transições + Lyapunov ─────────────────────────────── */
static void run_bench_fsm(u64 freq) {
    static u64 samp[BENCH_K];
    FSMState fsm;
    raf_puts("=== BENCH FSM 1000 steps + Lyapunov Q16 ===\n");
    BENCH_RUN(samp, {
        fsm_init(&fsm);
        /* 1000 transições com input derivado do step — pseudo-random        */
        for (u32 k = 0; k < 1000; k++) {
            u32 inp = (k ^ (k >> 3) ^ (k << 2)) & (FSM_IN - 1);
            fsm_step(&fsm, inp);
        }
        __asm__ volatile("" :: "r"(fsm.lambda) : "memory");
    });
    BenchResult r = bench_analyze(samp);
    bench_report("fsm_1000steps_lyap", r, freq);
    raf_puts("  final_state=");
    raf_puts(fsm_domain_name(fsm.state));
    raf_puts("\n  attractor=");
    raf_puts(attractor_name(fsm_classify(fsm.lambda)));
    raf_puts("\n  lambda_q16=");
    raf_putu64((u64)(u32)(s32)fsm.lambda);
    raf_puts("  acc_weight_q16=");
    raf_putu64((u64)(u32)(s32)fsm.acc_weight);
}

/* ── Sessão: T^7 toroide 1000 steps ────────────────────────────────────── */
static void run_bench_toroid(u64 freq) {
    static u64 samp[BENCH_K];
    T7State t7;
    raf_puts("=== BENCH T^7 TOROID 1000 steps IIR ===\n");
    BENCH_RUN(samp, {
        t7_init(&t7);
        for (u32 k = 0; k < 1000; k++) {
            T7Input xi;
            xi.data_hash = k ^ 0xDEADBEEF;
            xi.entropy   = (q16_t)((k * 137) & 0xFFFF);
            xi.hw_state  = k ^ (k >> 4);
            t7_map_input(&t7, &xi);
            t7_step(&t7,
                (q16_t)((k * 41) & 0xFFFF),
                (q16_t)((k * 73) & 0xFFFF));
        }
        __asm__ volatile("" :: "r"(t7.phi) : "memory");
    });
    BenchResult r = bench_analyze(samp);
    bench_report("t7_toroid_1000steps", r, freq);
    raf_puts("  attractor_idx=");  raf_putu64(t7.attractor);
    raf_puts("  coherence_q16=");  raf_putu64((u64)(u32)t7_coherence(&t7));
    raf_puts("  phi_ethica_q16="); raf_putu64((u64)(u32)t7.phi);
}

/* ── Sessão: spiral + fibonacci-rafael ──────────────────────────────────── */
static void run_bench_spiral(u64 freq) {
    static u64 samp[BENCH_K];
    q16_t fraf = Q16_ONE;
    raf_puts("=== BENCH FIBONACCI-RAFAEL 1000 steps Q16 ===\n");
    BENCH_RUN(samp, {
        fraf = Q16_ONE;
        for (u32 k = 0; k < 1000; k++) {
            fraf = q16_fraf_next(fraf);
        }
        __asm__ volatile("" :: "r"(fraf) : "memory");
    });
    BenchResult r = bench_analyze(samp);
    bench_report("fraf_1000steps_q16", r, freq);
    raf_puts("  fraf_q16="); raf_putu64((u64)(u32)fraf);
}

/* ── Sessão: Merkle chain ───────────────────────────────────────────────── */
static void run_bench_merkle(u64 freq) {
    static u64 samp[BENCH_K];
    MerkleChain mc;
    HashVivo hv;
    raf_puts("=== BENCH MERKLE CHAIN + HASHVIVO 100 feeds ===\n");
    BENCH_RUN(samp, {
        merkle_init(&mc);
        hashvivo_init(&hv);
        for (u32 k = 0; k < 100; k++) {
            u64 dh = phi64_mix((u64)k, (u64)k * 0xDEAD);
            merkle_feed(&mc, dh);
            hashvivo_feed(&hv, dh);
        }
        u64 root   = merkle_root(&mc);
        u64 digest = hashvivo_digest(&hv);
        __asm__ volatile("" :: "r"(root),"r"(digest) : "memory");
    });
    BenchResult r = bench_analyze(samp);
    bench_report("merkle_hashvivo_100", r, freq);
    /* Recalcula fora do bench para imprimir */
    merkle_init(&mc); hashvivo_init(&hv);
    for (u32 k = 0; k < 100; k++) {
        u64 dh = phi64_mix((u64)k, (u64)k * 0xDEAD);
        merkle_feed(&mc, dh); hashvivo_feed(&hv, dh);
    }
    raf_puts("  merkle_root=");  raf_putu64(merkle_root(&mc));
    raf_puts("  hashvivo=");     raf_putu64(hashvivo_digest(&hv));
}

/* ── ABI report em runtime ──────────────────────────────────────────────── */
static void print_abi_report(u64 freq) {
    raf_puts("\n=== ABI & HARDWARE REPORT ===\n");
#ifdef RAF_ARCH_A64
    raf_puts("  arch=ARM64 ABI=AAPCS64\n");
    raf_puts("  regs: x0-x7=args x8=syscall/indirect x19-x28=callee-saved\n");
    raf_puts("  sp=16byte_aligned x30=LR NEON=v0-v31\n");
    raf_puts("  crc32c=hw(crc32cx) spiral=fmla tsc=cntvct_el0\n");
#endif
#ifdef RAF_ARCH_X64
    raf_puts("  arch=x86-64 ABI=SysV_AMD64\n");
    raf_puts("  regs: rdi,rsi,rdx,rcx,r8,r9=args rax=ret\n");
    raf_puts("  rbx,rbp,r12-r15=callee-saved rsp=16B_before_call\n");
    raf_puts("  crc32c=hw(crc32q,SSE4.2) tsc=rdtsc+lfence\n");
#endif
#ifdef RAF_ARCH_A32
    raf_puts("  arch=ARM32 ABI=AAPCS\n");
    raf_puts("  regs: r0-r3=args r4-r11=callee-saved r13=sp r14=lr r15=pc\n");
    raf_puts("  crc32c=software(poly=0x82F63B78) timer=clock_gettime(raw syscall)\n");
#endif
    raf_puts("  arena_cap="); raf_putu64(ARENA_CAP);
    raf_puts("bytes  bench_k="); raf_putu64(BENCH_K);
    raf_puts("  timer_freq_hz_or_zero_raw_ticks="); raf_putu64(freq);
}

/* ── Execution/evidence boundary summary ───────────────────────────────── */
static void print_summary(void) {
    raf_puts("\n=== RAFAELIA BENCHMARK EXECUTION BOUNDARY ===\n");
    raf_puts("  measured_values     | emitted above | current process only\n");
    raf_puts("  dynamic_heap_calls  | 0 in this harness | static arena\n");
    raf_puts("  GC_runtime          | none | no GC dependency\n");
#ifdef RAF_ARCH_A32
    raf_puts("  timer               | clock_gettime | raw Linux syscall; ns\n");
#endif
#ifdef RAF_ARCH_A64
    raf_puts("  timer               | cntvct_el0 | counter + cntfrq_el0\n");
#endif
#ifdef RAF_ARCH_X64
    raf_puts("  timer               | rdtsc+lfence | raw ticks; not calibrated ns\n");
#endif
    raf_puts("  I/O + exit          | raw Linux syscalls | OS-bound userspace\n");
    raf_puts("  physical bare-metal | TOKEN_VAZIO (CLOSURE_L12) | requires board/startup/device receipt\n");
}

/* ── Entrada principal — sem argc/argv no modo nolibc ───────────────────── */
void _start(void) {
    u64 freq = raf_tsc_freq();
    raf_puts("\nRAFAELIA ENTERPRISE NO-LIBC USERSPACE v1.1\n");
    raf_puts("Linux/Android raw-syscall | zero-malloc | Q16 | ARM64/x86-64/ARM32\n");
    raf_puts("Omega=Amor | bitraf64 | hashchain RAFCODE-PHI\n\n");
    print_abi_report(freq);
    raf_puts("\n");
    ARENA_RESET();
    run_bench_crc32c(freq);
    run_bench_arena(freq);
    run_bench_hash(freq);
    run_bench_fsm(freq);
    run_bench_toroid(freq);
    run_bench_spiral(freq);
    run_bench_merkle(freq);
    print_summary();
    raf_puts("\nΣΩΔΦ RAFCODE-Phi-DeltaRafaelVerboOmega\n");
    raf_exit(0);
}
