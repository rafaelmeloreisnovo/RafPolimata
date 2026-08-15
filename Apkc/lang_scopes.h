/* lang_scopes.h — Variable scoping and block management
 *
 * Tracks variable scopes, block depth, lifetime
 * Implements LIFO scope stack for nested blocks
 */

#ifndef APKC_LANG_SCOPES_H
#define APKC_LANG_SCOPES_H 1

typedef unsigned char u8;
typedef unsigned int u32;

/* Scope level info */
struct ScopeLevel {
	u32 var_start;       /* index of first var in this scope */
	u32 var_count;       /* number of vars in this scope */
	u8 depth;            /* nesting depth (0=global, 1+=local) */
};

/* Scope stack: manages variable visibility */
struct ScopeStack {
	struct ScopeLevel levels[16];  /* up to 16 nested levels */
	u8 level_count;                /* current level count */
};

/* Initialize scope stack */
static inline void scope_init(struct ScopeStack *ss) {
	ss->level_count = 0;
	ss->levels[0].var_start = 0;
	ss->levels[0].var_count = 0;
	ss->levels[0].depth = 0;
}

/* Enter new scope (opening brace) */
static inline u8 scope_enter(struct ScopeStack *ss, u32 var_base) {
	if (ss->level_count >= 15) return 1;  /* too deep */

	ss->level_count++;
	u8 level = ss->level_count;
	ss->levels[level].var_start = var_base;
	ss->levels[level].var_count = 0;
	ss->levels[level].depth = level;
	return 0;
}

/* Exit scope (closing brace) */
static inline u32 scope_exit(struct ScopeStack *ss) {
	if (ss->level_count == 0) return 0;

	u32 freed_vars = ss->levels[ss->level_count].var_count;
	ss->level_count--;
	return freed_vars;
}

/* Get current scope depth */
static inline u8 scope_depth(struct ScopeStack *ss) {
	return ss->level_count;
}

#endif /* APKC_LANG_SCOPES_H */
