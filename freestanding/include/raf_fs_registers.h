#ifndef RAF_FS_REGISTERS_H
#define RAF_FS_REGISTERS_H

/*
 * RAFAELIA-L0-FILE-CONTRACT
 * PURPOSE: Zero-runtime register-class topology metadata for scheduling/codegen decisions.
 * SCOPE: Architectural class/count metadata only; no privileged register access and no OS ABI.
 * PRECONDITIONS: Compiler target macros identify the selected ISA/profile.
 * REGISTER_OWNERSHIP: Metadata only; no architectural register is read or written here.
 * CLOBBERS: None.
 * MEMORY_ORDER: None.
 * TAIL_SHADOW: No data path, no tail, no shadow state.
 * EVIDENCE: Source metadata; executor support is tracked separately under CLOSURE_L11/CLOSURE_L12.
 */

#include "raf_fs_abi.h"

#define RAF_FS_REGCLASS_GPR        (1u << 0)
#define RAF_FS_REGCLASS_PC         (1u << 1)
#define RAF_FS_REGCLASS_STATUS     (1u << 2)
#define RAF_FS_REGCLASS_FP         (1u << 3)
#define RAF_FS_REGCLASS_SIMD       (1u << 4)
#define RAF_FS_REGCLASS_VECTOR     (1u << 5)
#define RAF_FS_REGCLASS_PREDICATE  (1u << 6)
#define RAF_FS_REGCLASS_MATRIX     (1u << 7)
#define RAF_FS_REGCLASS_CONTROL    (1u << 8)
#define RAF_FS_REGCLASS_DEBUG      (1u << 9)
#define RAF_FS_REGCLASS_PMU        (1u << 10)
#define RAF_FS_REGCLASS_VIRT       (1u << 11)
#define RAF_FS_REGCLASS_SECURITY   (1u << 12)

#if defined(__x86_64__)
# define RAF_FS_REGCLASS_MASK (RAF_FS_REGCLASS_GPR | RAF_FS_REGCLASS_PC | RAF_FS_REGCLASS_STATUS | RAF_FS_REGCLASS_FP | RAF_FS_REGCLASS_SIMD | RAF_FS_REGCLASS_CONTROL | RAF_FS_REGCLASS_DEBUG | RAF_FS_REGCLASS_PMU | RAF_FS_REGCLASS_VIRT | RAF_FS_REGCLASS_SECURITY | ((RAF_FS_ABI_PRED_REGS) ? RAF_FS_REGCLASS_PREDICATE : 0u) | ((RAF_FS_ABI_MATRIX_REGS) ? RAF_FS_REGCLASS_MATRIX : 0u))
# define RAF_FS_FP_REGS 8u       /* x87 architectural stack registers. */
# define RAF_FS_STATUS_REGS 2u   /* RIP/RFLAGS modeled as control/status classes, not GPRs. */
# define RAF_FS_CONTROL_FAMILY 1u /* CR*, XCR*, MXCSR, segment/system families; privileged access excluded here. */

#elif defined(__i386__)
# define RAF_FS_REGCLASS_MASK (RAF_FS_REGCLASS_GPR | RAF_FS_REGCLASS_PC | RAF_FS_REGCLASS_STATUS | RAF_FS_REGCLASS_FP | RAF_FS_REGCLASS_SIMD | RAF_FS_REGCLASS_CONTROL | RAF_FS_REGCLASS_DEBUG | RAF_FS_REGCLASS_PMU | RAF_FS_REGCLASS_SECURITY)
# define RAF_FS_FP_REGS 8u
# define RAF_FS_STATUS_REGS 2u
# define RAF_FS_CONTROL_FAMILY 1u

#elif defined(__aarch64__)
# define RAF_FS_REGCLASS_MASK (RAF_FS_REGCLASS_GPR | RAF_FS_REGCLASS_PC | RAF_FS_REGCLASS_STATUS | RAF_FS_REGCLASS_FP | RAF_FS_REGCLASS_SIMD | RAF_FS_REGCLASS_VECTOR | RAF_FS_REGCLASS_CONTROL | RAF_FS_REGCLASS_DEBUG | RAF_FS_REGCLASS_PMU | RAF_FS_REGCLASS_VIRT | RAF_FS_REGCLASS_SECURITY | ((RAF_FS_ABI_PRED_REGS) ? RAF_FS_REGCLASS_PREDICATE : 0u) | ((RAF_FS_ABI_MATRIX_REGS) ? RAF_FS_REGCLASS_MATRIX : 0u))
# define RAF_FS_FP_REGS 32u      /* V0..V31 views include scalar FP lanes. */
# define RAF_FS_STATUS_REGS 3u   /* NZCV/FPCR/FPSR primary directly relevant state. */
# define RAF_FS_CONTROL_FAMILY 2u /* PSTATE/system-register families; privileged access excluded here. */

#elif defined(__arm__) && (__ARM_ARCH >= 7)
# define RAF_FS_REGCLASS_MASK (RAF_FS_REGCLASS_GPR | RAF_FS_REGCLASS_PC | RAF_FS_REGCLASS_STATUS | RAF_FS_REGCLASS_FP | RAF_FS_REGCLASS_SIMD | RAF_FS_REGCLASS_CONTROL | RAF_FS_REGCLASS_DEBUG | RAF_FS_REGCLASS_PMU | RAF_FS_REGCLASS_SECURITY)
# define RAF_FS_FP_REGS 32u      /* S0..S31 scalar views; D/Q aliases are profile-dependent. */
# define RAF_FS_STATUS_REGS 2u   /* APSR/CPSR plus FPSCR family. */
# define RAF_FS_CONTROL_FAMILY 3u /* CP15/system families excluded from ordinary L0 access. */

#elif defined(__riscv)
# define RAF_FS_REGCLASS_MASK (RAF_FS_REGCLASS_GPR | RAF_FS_REGCLASS_PC | RAF_FS_REGCLASS_STATUS | RAF_FS_REGCLASS_FP | RAF_FS_REGCLASS_VECTOR | RAF_FS_REGCLASS_CONTROL | RAF_FS_REGCLASS_DEBUG | RAF_FS_REGCLASS_PMU | RAF_FS_REGCLASS_VIRT | RAF_FS_REGCLASS_SECURITY | ((RAF_FS_ABI_PRED_REGS) ? RAF_FS_REGCLASS_PREDICATE : 0u))
# if defined(__riscv_flen)
#  define RAF_FS_FP_REGS 32u
# else
#  define RAF_FS_FP_REGS 0u
# endif
# define RAF_FS_STATUS_REGS 1u   /* pc is separate; CSR families carry status/control. */
# define RAF_FS_CONTROL_FAMILY 4u /* CSR families; privilege/extension determines accessibility. */
#else
# error "Unsupported RAFAELIA register topology target"
#endif

/* Scheduling geometry: storage aliases are not counted as independent physical state. */
#define RAF_FS_REG_GPR_COUNT       RAF_FS_ABI_GPR_COUNT
#define RAF_FS_REG_VECTOR_COUNT    RAF_FS_ABI_VECTOR_REGS
#define RAF_FS_REG_PRED_COUNT      RAF_FS_ABI_PRED_REGS
#define RAF_FS_REG_MATRIX_COUNT    RAF_FS_ABI_MATRIX_REGS
#define RAF_FS_REG_VECTOR_BITS     RAF_FS_ABI_VECTOR_BITS

#endif
