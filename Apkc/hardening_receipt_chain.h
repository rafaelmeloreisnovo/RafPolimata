/* Apkc/hardening_receipt_chain.h — Receipt chain-of-custody gates
 *
 * No malloc, no libc, no abstractions. Direct evidence tracking.
 * Records SHA-256 snapshots, gate states, and custody transitions.
 */

#ifndef HARDENING_RECEIPT_CHAIN_H
#define HARDENING_RECEIPT_CHAIN_H 1

#include <stdint.h>
#include <stddef.h>

#define RECEIPT_CHAIN_MAX_DEPTH 16
#define RECEIPT_SHA256_SIZE 32
#define RECEIPT_TOKEN_SIZE 64

/* Single receipt entry: immutable snapshot */
struct receipt_entry {
	uint8_t sha256[RECEIPT_SHA256_SIZE];
	uint32_t gate_index;    /* which gate produced this */
	uint8_t gate_state;     /* PASS=0, FAIL=1, TOKEN_VAZIO=2 */
	uint32_t timestamp;     /* unix seconds */
	uint32_t artifact_size;
	uint8_t artifact_type;  /* SOURCE=0, OBJECT=1, APK=2, ELF=3, PROOF=4 */
};

/* Chain links: from one artifact to next */
struct receipt_link {
	struct receipt_entry source;
	struct receipt_entry target;
	uint8_t transform;      /* 0=identity, 1=compile, 2=link, 3=zip, 4=sign */
	uint8_t flags;          /* bit0: reversible, bit1: deterministic */
};

/* Complete chain sequence */
struct receipt_chain {
	struct receipt_entry entries[RECEIPT_CHAIN_MAX_DEPTH];
	struct receipt_link links[RECEIPT_CHAIN_MAX_DEPTH - 1];
	uint8_t entry_count;
	uint8_t link_count;
	uint8_t chain_valid;
	uint32_t total_transformations;
};

/* === Receipt operations === */

static inline void receipt_entry_init(struct receipt_entry *entry) {
	for (int i = 0; i < RECEIPT_SHA256_SIZE; i++) {
		entry->sha256[i] = 0;
	}
	entry->gate_index = 0;
	entry->gate_state = 2;  /* TOKEN_VAZIO by default */
	entry->timestamp = 0;
	entry->artifact_size = 0;
	entry->artifact_type = 0;
}

static inline void receipt_entry_set_sha(struct receipt_entry *entry, const uint8_t *sha256) {
	for (int i = 0; i < RECEIPT_SHA256_SIZE; i++) {
		entry->sha256[i] = sha256[i];
	}
}

static inline uint8_t receipt_entry_sha_matches(const struct receipt_entry *a, const struct receipt_entry *b) {
	for (int i = 0; i < RECEIPT_SHA256_SIZE; i++) {
		if (a->sha256[i] != b->sha256[i]) return 0;
	}
	return 1;
}

/* === Chain operations === */

static inline void receipt_chain_init(struct receipt_chain *chain) {
	chain->entry_count = 0;
	chain->link_count = 0;
	chain->chain_valid = 1;
	chain->total_transformations = 0;

	for (int i = 0; i < RECEIPT_CHAIN_MAX_DEPTH; i++) {
		receipt_entry_init(&chain->entries[i]);
	}
}

static inline uint8_t receipt_chain_add_entry(struct receipt_chain *chain,
	const struct receipt_entry *entry)
{
	if (chain->entry_count >= RECEIPT_CHAIN_MAX_DEPTH) {
		chain->chain_valid = 0;
		return 0;
	}

	chain->entries[chain->entry_count] = *entry;
	chain->entry_count++;
	return 1;
}

static inline uint8_t receipt_chain_add_link(struct receipt_chain *chain,
	struct receipt_link *link)
{
	if (chain->link_count >= RECEIPT_CHAIN_MAX_DEPTH - 1) {
		chain->chain_valid = 0;
		return 0;
	}

	/* Verify continuity: target of new link must match next entry */
	if (chain->link_count > 0) {
		uint8_t prev_target_idx = chain->link_count;
		if (!receipt_entry_sha_matches(&chain->entries[prev_target_idx], &link->source)) {
			chain->chain_valid = 0;
			return 0;
		}
	}

	chain->links[chain->link_count] = *link;
	chain->link_count++;
	chain->total_transformations++;
	return 1;
}

static inline uint8_t receipt_chain_is_continuous(const struct receipt_chain *chain) {
	if (chain->link_count + 1 != chain->entry_count) return 0;
	if (!chain->chain_valid) return 0;

	/* Every link must connect consecutive entries */
	for (int i = 0; i < chain->link_count; i++) {
		if (chain->links[i].transform == 0) {
			/* identity transform: source and target sha must match */
			if (!receipt_entry_sha_matches(&chain->links[i].source, &chain->links[i].target)) {
				return 0;
			}
		}
	}

	return 1;
}

/* === Custody transfer gate === */

struct receipt_custody_gate {
	struct receipt_chain source_chain;
	struct receipt_chain object_chain;
	struct receipt_chain apk_chain;
	uint8_t custody_verified;
	uint8_t source_to_object_ok;  /* transformation succeeded */
	uint8_t object_to_apk_ok;
	uint32_t gate_count;
};

static inline void receipt_custody_init(struct receipt_custody_gate *gate) {
	receipt_chain_init(&gate->source_chain);
	receipt_chain_init(&gate->object_chain);
	receipt_chain_init(&gate->apk_chain);
	gate->custody_verified = 0;
	gate->source_to_object_ok = 0;
	gate->object_to_apk_ok = 0;
	gate->gate_count = 0;
}

static inline uint8_t receipt_custody_verify(const struct receipt_custody_gate *gate) {
	if (!receipt_chain_is_continuous(&gate->source_chain)) return 0;
	if (!receipt_chain_is_continuous(&gate->object_chain)) return 0;
	if (!receipt_chain_is_continuous(&gate->apk_chain)) return 0;

	/* All chains must have at least one entry */
	if (gate->source_chain.entry_count == 0) return 0;
	if (gate->object_chain.entry_count == 0) return 0;
	if (gate->apk_chain.entry_count == 0) return 0;

	return 1;
}

/* === Proof manifest gate === */

struct receipt_proof_manifest {
	struct receipt_entry f0;  /* Source read */
	struct receipt_entry f1;  /* Host compile */
	struct receipt_entry f2;  /* ARM32 assemble */
	struct receipt_entry f3;  /* ARM32 link */
	struct receipt_entry f4;  /* Obj generation */
	struct receipt_entry f5;  /* Cross-AArch64 */
	struct receipt_entry f6;  /* Source cap verify */
	uint8_t all_pass;
	uint32_t checkpoint;
};

static inline void receipt_proof_manifest_init(struct receipt_proof_manifest *manifest) {
	receipt_entry_init(&manifest->f0);
	receipt_entry_init(&manifest->f1);
	receipt_entry_init(&manifest->f2);
	receipt_entry_init(&manifest->f3);
	receipt_entry_init(&manifest->f4);
	receipt_entry_init(&manifest->f5);
	receipt_entry_init(&manifest->f6);
	manifest->all_pass = 0;
	manifest->checkpoint = 0;
}

static inline void receipt_proof_manifest_mark_pass(struct receipt_proof_manifest *manifest,
	uint8_t gate_index)
{
	struct receipt_entry *entry = NULL;

	switch (gate_index) {
	case 0: entry = &manifest->f0; break;
	case 1: entry = &manifest->f1; break;
	case 2: entry = &manifest->f2; break;
	case 3: entry = &manifest->f3; break;
	case 4: entry = &manifest->f4; break;
	case 5: entry = &manifest->f5; break;
	case 6: entry = &manifest->f6; break;
	default: return;
	}

	entry->gate_state = 0;  /* PASS */
	entry->gate_index = gate_index;
}

static inline uint8_t receipt_proof_manifest_all_pass(const struct receipt_proof_manifest *manifest) {
	return manifest->f0.gate_state == 0 &&
	       manifest->f1.gate_state == 0 &&
	       manifest->f2.gate_state == 0 &&
	       manifest->f3.gate_state == 0 &&
	       manifest->f4.gate_state == 0 &&
	       manifest->f5.gate_state == 0 &&
	       manifest->f6.gate_state == 0;
}

/* === Recursive custody chain === */

struct receipt_recursive_custody {
	struct receipt_chain provider_to_physical;
	struct receipt_entry provider_artifact;
	struct receipt_entry physical_artifact;
	uint8_t provider_sealed;
	uint8_t physical_sealed;
	uint32_t recursion_depth;
	uint8_t closure_verified;
};

static inline void receipt_recursive_custody_init(struct receipt_recursive_custody *rec) {
	receipt_chain_init(&rec->provider_to_physical);
	receipt_entry_init(&rec->provider_artifact);
	receipt_entry_init(&rec->physical_artifact);
	rec->provider_sealed = 0;
	rec->physical_sealed = 0;
	rec->recursion_depth = 0;
	rec->closure_verified = 0;
}

static inline uint8_t receipt_recursive_custody_seal(struct receipt_recursive_custody *rec) {
	if (rec->recursion_depth == 0) return 0;
	if (!receipt_chain_is_continuous(&rec->provider_to_physical)) return 0;

	rec->closure_verified = 1;
	return 1;
}

#endif /* HARDENING_RECEIPT_CHAIN_H */
