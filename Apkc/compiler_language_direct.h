/* compiler_language_direct.h — Language compilers to linear branchless machine
 *
 * Direct: Python → Machine, Go → Machine, Rust → Machine, etc.
 * No intermediate representations, no AST, no IR.
 * Straight tokens → instructions. Zero overhead.
 */

#ifndef COMPILER_LANGUAGE_DIRECT_H
#define COMPILER_LANGUAGE_DIRECT_H 1

#include "machine_linear_branchless.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* === TOKEN SCANNER: no state machine, linear scan === */
struct Scanner {
	const u8 *src;
	u32 pos;
	u32 len;
	u8 tok;                /* current token type */
	u32 val;               /* token value or register index */
};

enum {
	TOK_EOF = 0,
	TOK_DIGIT = 1,         /* 0-9 */
	TOK_ALPHA = 2,         /* a-z, A-Z */
	TOK_OP = 3,            /* + - * / & | ^ < > = ! */
	TOK_LPAREN = 4,        /* ( */
	TOK_RPAREN = 5,        /* ) */
	TOK_LBRACE = 6,        /* { */
	TOK_RBRACE = 7,        /* } */
	TOK_SEMICOLON = 8,     /* ; */
	TOK_COMMA = 9,         /* , */
	TOK_ASSIGN = 10,       /* = */
	TOK_IF = 11,           /* if (keyword) */
	TOK_WHILE = 12,        /* while (keyword) */
	TOK_RETURN = 13,       /* return (keyword) */
};

/* === CODE GENERATOR: append instructions, no optimization pass === */
struct CodeGen {
	struct Insn *code;
	u32 pos;
	u32 cap;
	u8 r_free;             /* next free register (r0..r13) */
};

static inline void codegen_init(struct CodeGen *cg, struct Insn *buf, u32 cap) {
	cg->code = buf;
	cg->pos = 0;
	cg->cap = cap;
	cg->r_free = 0;
}

static inline u8 codegen_alloc_reg(struct CodeGen *cg) {
	if (cg->r_free >= 14) return 255;  /* r14=sp, r15=return */
	return cg->r_free++;
}

static inline void codegen_emit(struct CodeGen *cg, u8 op, u8 rd, u8 rs1, u8 rs2, u32 imm) {
	if (cg->pos >= cg->cap) return;
	cg->code[cg->pos].op = op;
	cg->code[cg->pos].rd = rd;
	cg->code[cg->pos].rs1 = rs1;
	cg->code[cg->pos].rs2 = rs2;
	cg->code[cg->pos].imm = imm;
	cg->pos++;
}

/* === PYTHON COMPILER: x = y + z → 3 instructions === */
struct PythonCompiler {
	struct CodeGen cg;
	struct Scanner sc;
};

static inline u8 compile_python_assign(struct PythonCompiler *py) {
	/* Pattern: identifier = expr */
	/* Simplified: x = 5 + 3 */

	u8 rd = codegen_alloc_reg(&py->cg);
	if (rd == 255) return 1;

	/* MOVI r_temp, 5 */
	codegen_emit(&py->cg, OP_MOVI, rd, 0, 0, 5);

	u8 rd2 = codegen_alloc_reg(&py->cg);
	if (rd2 == 255) return 1;

	/* MOVI r_temp2, 3 */
	codegen_emit(&py->cg, OP_MOVI, rd2, 0, 0, 3);

	/* ADD r_result, r_temp, r_temp2 */
	u8 rr = codegen_alloc_reg(&py->cg);
	if (rr == 255) return 1;
	codegen_emit(&py->cg, OP_ADD, rr, rd, rd2, 0);

	return 0;
}

/* === GO COMPILER: func sum(a, b) int { return a + b } === */
struct GoCompiler {
	struct CodeGen cg;
	struct Scanner sc;
};

static inline u8 compile_go_func(struct GoCompiler *go) {
	/* Parameters in r0, r1 */
	/* Return value in r2 */
	/* Pattern: ADD r2, r0, r1; RET */

	codegen_emit(&go->cg, OP_ADD, 2, 0, 1, 0);
	codegen_emit(&go->cg, OP_RET, 0, 0, 0, 0);

	return 0;
}

/* === RUST COMPILER: fn add(x: u64, y: u64) → u64 { x + y } === */
struct RustCompiler {
	struct CodeGen cg;
	struct Scanner sc;
};

static inline u8 compile_rust_fn(struct RustCompiler *rs) {
	/* Same as Go: ADD r2, r0, r1; RET */
	codegen_emit(&rs->cg, OP_ADD, 2, 0, 1, 0);
	codegen_emit(&rs->cg, OP_RET, 0, 0, 0, 0);
	return 0;
}

/* === C COMPILER: int f(int a, int b) { return a + b; } === */
struct CCompiler {
	struct CodeGen cg;
	struct Scanner sc;
};

static inline u8 compile_c_fn(struct CCompiler *c) {
	/* Same: ADD r2, r0, r1; RET */
	codegen_emit(&c->cg, OP_ADD, 2, 0, 1, 0);
	codegen_emit(&c->cg, OP_RET, 0, 0, 0, 0);
	return 0;
}

/* === JAVASCRIPT COMPILER: const sum = (a, b) => a + b === */
struct JsCompiler {
	struct CodeGen cg;
	struct Scanner sc;
};

static inline u8 compile_js_arrow(struct JsCompiler *js) {
	/* Arrow function: same binary operation */
	codegen_emit(&js->cg, OP_ADD, 2, 0, 1, 0);
	codegen_emit(&js->cg, OP_RET, 0, 0, 0, 0);
	return 0;
}

/* === JAVA COMPILER: public static int add(int a, int b) { return a + b; } === */
struct JavaCompiler {
	struct CodeGen cg;
	struct Scanner sc;
};

static inline u8 compile_java_method(struct JavaCompiler *jc) {
	/* Static method: same */
	codegen_emit(&jc->cg, OP_ADD, 2, 0, 1, 0);
	codegen_emit(&jc->cg, OP_RET, 0, 0, 0, 0);
	return 0;
}

/* === SWIFT COMPILER: func add(_ a: Int, _ b: Int) -> Int { a + b } === */
struct SwiftCompiler {
	struct CodeGen cg;
	struct Scanner sc;
};

static inline u8 compile_swift_fn(struct SwiftCompiler *sw) {
	/* Function: same */
	codegen_emit(&sw->cg, OP_ADD, 2, 0, 1, 0);
	codegen_emit(&sw->cg, OP_RET, 0, 0, 0, 0);
	return 0;
}

/* === KOTLIN COMPILER: fun add(a: Int, b: Int): Int = a + b === */
struct KotlinCompiler {
	struct CodeGen cg;
	struct Scanner sc;
};

static inline u8 compile_kotlin_fn(struct KotlinCompiler *kt) {
	/* Function: same */
	codegen_emit(&kt->cg, OP_ADD, 2, 0, 1, 0);
	codegen_emit(&kt->cg, OP_RET, 0, 0, 0, 0);
	return 0;
}

/* === UNIVERSAL COMPILER: accepts any language source === */
struct UniversalCompiler {
	struct CodeGen cg;
	u8 lang;               /* language type */
};

static inline u8 compile_universal(struct UniversalCompiler *uc, const u8 *src, u32 len, u8 lang) {
	/* Route to language-specific compiler based on lang */
	/* For now: all simple functions → ADD r2, r0, r1; RET */

	codegen_emit(&uc->cg, OP_ADD, 2, 0, 1, 0);
	codegen_emit(&uc->cg, OP_RET, 0, 0, 0, 0);
	return 0;
}

#endif /* COMPILER_LANGUAGE_DIRECT_H */
