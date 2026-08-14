/* Apkc/hardening_armv7_assembler.h — ARM32 cross-assembler validation gates
 *
 * No malloc, no libc, no abstractions. Direct syscalls for assembly verification.
 * Validates ARMv7 EABI instruction encoding and binary object production.
 * Freestanding: no stdint.h, no stdlib.h, no external includes.
 */

#ifndef HARDENING_ARMV7_ASSEMBLER_H
#define HARDENING_ARMV7_ASSEMBLER_H 1

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef int int32_t;

#define ARM32_INSTRUCTION_SIZE 4
#define ARM32_MAX_SECTION_SIZE 0x100000UL  /* 1 MiB per section */
#define ARM32_MAX_SYMBOLS 256
#define ARM32_MAX_RELOCATIONS 512

/* ARM32 instruction class validation */
enum arm32_instr_class {
	ARM32_ALU = 0,        /* ADD, SUB, ORR, AND, XOR */
	ARM32_LOAD_STORE = 1, /* LDR, STR, LDRB, STRB */
	ARM32_BRANCH = 2,     /* B, BL, BX */
	ARM32_NEON = 3,       /* SIMD instructions */
	ARM32_SYSTEM = 4,     /* MRS, MSR, SWI */
	ARM32_UNDEFINED = 255,
};

/* Single assembler pass result */
struct armv7_assembly_result {
	uint32_t instruction_count;
	uint32_t section_size;
	uint32_t symbol_count;
	uint32_t relocation_count;
	uint8_t has_text_section;
	uint8_t has_rodata_section;
	uint8_t has_data_section;
	uint8_t elf_class;  /* 32 or 64 */
};

/* Assembly cross-check gate: no abstractions, just flags */
struct armv7_cross_assembler {
	uint8_t pass_count;      /* 0 to 9 */
	uint8_t fail_count;
	uint32_t total_instructions;
	uint32_t total_size_bytes;
	struct armv7_assembly_result passes[9];
	uint8_t flags;  /* bit 0: verified, bit 1: relocatable, bit 2: linked */
};

/* Instruction encoding validator: no callbacks, pure bit checks */

static inline uint8_t armv7_decode_class(uint32_t instr) {
	/* Bare ARM32/Thumb instruction classification by opcode bits */
	uint32_t cond = (instr >> 28) & 0xF;
	if (cond == 0xF) return ARM32_UNDEFINED;

	uint32_t op = (instr >> 26) & 0x3;
	switch (op) {
	case 0: /* 00 - ALU/load-store */
		{
			uint32_t bit25 = (instr >> 25) & 1;
			uint32_t bit24_20 = (instr >> 20) & 0x1F;

			if (bit25 == 0 && bit24_20 == 0x2) return ARM32_LOAD_STORE; /* Reg shift */
			if (bit24_20 >= 0x10 && bit24_20 <= 0x1F) return ARM32_LOAD_STORE;
			return ARM32_ALU;
		}
	case 1: /* 01 - Load/Store immediate */
		return ARM32_LOAD_STORE;
	case 2: /* 10 - Branch */
		return ARM32_BRANCH;
	case 3: /* 11 - Coprocessor / NEON */
		{
			uint32_t bit24 = (instr >> 24) & 1;
			if (bit24 == 0) return ARM32_NEON;
			return ARM32_SYSTEM;
		}
	}
	return ARM32_UNDEFINED;
}

/* Simple pass-through validation: instruction at offset is valid encoding */

static inline uint8_t armv7_instr_valid(uint32_t instr) {
	/* Undefined encodings in ARM32: certain coprocessor forms, reserved bits */
	uint32_t cond = (instr >> 28) & 0xF;
	uint32_t op1 = (instr >> 25) & 0x7;
	uint32_t op2 = (instr >> 4) & 0xF;

	/* Most 32-bit patterns are valid ARM32 instructions, but some are reserved */
	if (cond == 0xF && op1 == 0x7 && op2 == 0xF) return 0;  /* Reserved pattern */

	return 1;
}

/* === Assembly result tracking === */

static inline void armv7_result_init(struct armv7_assembly_result *res) {
	res->instruction_count = 0;
	res->section_size = 0;
	res->symbol_count = 0;
	res->relocation_count = 0;
	res->has_text_section = 0;
	res->has_rodata_section = 0;
	res->has_data_section = 0;
	res->elf_class = 32;
}

static inline void armv7_result_add_instruction(struct armv7_assembly_result *res, uint32_t instr) {
	if (res->section_size < ARM32_MAX_SECTION_SIZE) {
		if (armv7_instr_valid(instr)) {
			res->instruction_count++;
			res->section_size += ARM32_INSTRUCTION_SIZE;
		}
	}
}

static inline void armv7_result_add_symbol(struct armv7_assembly_result *res) {
	if (res->symbol_count < ARM32_MAX_SYMBOLS) {
		res->symbol_count++;
	}
}

static inline void armv7_result_add_relocation(struct armv7_assembly_result *res) {
	if (res->relocation_count < ARM32_MAX_RELOCATIONS) {
		res->relocation_count++;
	}
}

/* === Cross-assembler gate === */

static inline void armv7_cross_init(struct armv7_cross_assembler *cross) {
	cross->pass_count = 0;
	cross->fail_count = 0;
	cross->total_instructions = 0;
	cross->total_size_bytes = 0;
	cross->flags = 0;

	for (int i = 0; i < 9; i++) {
		armv7_result_init(&cross->passes[i]);
	}
}

static inline void armv7_cross_record_pass(struct armv7_cross_assembler *cross,
	const struct armv7_assembly_result *result)
{
	if (cross->pass_count < 9) {
		cross->passes[cross->pass_count] = *result;
		cross->total_instructions += result->instruction_count;
		cross->total_size_bytes += result->section_size;
		cross->pass_count++;
	}
}

static inline void armv7_cross_record_fail(struct armv7_cross_assembler *cross) {
	cross->fail_count++;
}

static inline uint8_t armv7_cross_all_pass(const struct armv7_cross_assembler *cross) {
	return cross->pass_count == 9 && cross->fail_count == 0;
}

/* === Batch validation: verify 9 passes sequentially === */

struct armv7_batch_validator {
	struct armv7_cross_assembler cross;
	uint32_t batch_size;
	uint32_t bytes_processed;
	uint8_t all_validated;
};

static inline void armv7_batch_init(struct armv7_batch_validator *batch, uint32_t size) {
	armv7_cross_init(&batch->cross);
	batch->batch_size = size;
	batch->bytes_processed = 0;
	batch->all_validated = 0;
}

static inline void armv7_batch_process(struct armv7_batch_validator *batch,
	const uint8_t *data, uint32_t len)
{
	for (uint32_t i = 0; i + 3 < len; i += 4) {
		uint32_t instr = (uint32_t)data[i] |
		                 ((uint32_t)data[i+1] << 8) |
		                 ((uint32_t)data[i+2] << 16) |
		                 ((uint32_t)data[i+3] << 24);

		if (armv7_instr_valid(instr)) {
			batch->bytes_processed += 4;
		}
	}
}

static inline uint8_t armv7_batch_verify(struct armv7_batch_validator *batch) {
	/* All instructions must decode without error */
	batch->all_validated = (batch->bytes_processed == batch->batch_size);
	return batch->all_validated;
}

/* === Symbol and relocation tracking === */

struct armv7_symbol_entry {
	uint32_t offset;
	uint32_t size;
	uint8_t bind;   /* local=0, global=1, weak=2 */
	uint8_t type;   /* object=0, func=1, section=2 */
};

struct armv7_relocation_entry {
	uint32_t offset;
	uint8_t type;   /* ABS=0, REL=1, PLT=2 */
	uint32_t symbol_index;
	int32_t addend;
};

#endif /* HARDENING_ARMV7_ASSEMBLER_H */
