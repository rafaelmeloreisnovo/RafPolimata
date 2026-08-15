/* eco_plugin_system.h — Plugin System for Extended Languages (Stage 12.1)
 *
 * Dynamic language plugin loading and registration.
 * Plugin metadata: capabilities, version, dependencies.
 * Plugin initialization and lifecycle management.
 * Plugin interface: compile, validate, optimize functions.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_ECO_PLUGIN_SYSTEM_H
#define APKC_ECO_PLUGIN_SYSTEM_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Plugin status */
enum PluginStatus {
	PLUGIN_OK = 0,            /* Plugin loaded and active */
	PLUGIN_NOT_FOUND = 1,     /* Plugin file not found */
	PLUGIN_INVALID = 2,       /* Plugin format invalid */
	PLUGIN_INCOMPATIBLE = 3,  /* Plugin ABI incompatible */
	PLUGIN_INIT_FAILED = 4,   /* Plugin initialization failed */
	PLUGIN_DEPENDENCY = 5,    /* Dependency not satisfied */
	PLUGIN_VERSION = 6,       /* Version constraint violated */
	PLUGIN_DISABLED = 7       /* Plugin explicitly disabled */
};

/* Plugin version */
struct PluginVersion {
	u16 major;
	u16 minor;
	u32 patch;
};

/* Plugin capability flags */
#define PLUGIN_COMPILE 0x01
#define PLUGIN_VALIDATE 0x02
#define PLUGIN_OPTIMIZE 0x04
#define PLUGIN_CODEGEN 0x08
#define PLUGIN_LINK 0x10
#define PLUGIN_DEBUG 0x20
#define PLUGIN_PROFILE 0x40

/* Plugin metadata */
struct PluginMetadata {
	const char *name;              /* Plugin name (e.g., "kotlin_v2") */
	const char *language;          /* Language supported */
	struct PluginVersion version;  /* Plugin version */
	u8 capabilities;               /* Capability bitmask */
	const char **dependencies;     /* Dependency names (null-terminated) */
	u32 dependency_count;
	const char *description;       /* Human-readable description */
};

/* Plugin interface: functions implemented by plugin */
struct PluginInterface {
	/* Compile source to machine instructions */
	u8 (*compile)(const u8 *src, u32 src_len, u8 *out_code, u32 *out_len);

	/* Validate compiled code */
	u8 (*validate)(const u8 *code, u32 code_len);

	/* Optimize code */
	u8 (*optimize)(u8 *code, u32 *code_len, u8 level);

	/* Code generation backend */
	u8 (*codegen)(const u8 *code, u32 code_len, u8 *output, u32 *out_len);

	/* Link multiple compiled modules */
	u8 (*link)(const u8 **modules, const u32 *module_lens, u32 module_count,
			   u8 *output, u32 *out_len);

	/* Debug support */
	u8 (*debug_symbols)(const u8 *code, u32 code_len, u8 *symbols, u32 *sym_len);

	/* Profiling support */
	u8 (*profile_setup)(u8 *profile_buf, u32 profile_size);
};

/* Loaded plugin instance */
struct Plugin {
	struct PluginMetadata metadata;
	struct PluginInterface interface;
	u8 loaded;                 /* 1 if successfully loaded */
	u8 initialized;            /* 1 if initialized */
	u8 enabled;                /* 1 if enabled */
	void *handle;              /* Plugin handle (opaque) */
	u64 load_time_ms;          /* Load time in milliseconds */
};

/* Plugin registry */
struct PluginRegistry {
	struct Plugin plugins[32];  /* Up to 32 plugins */
	u32 plugin_count;
	u32 initialized_count;
};

/* ============================================================ */
/* PLUGIN REGISTRY MANAGEMENT */
/* ============================================================ */

/* Initialize plugin registry */
static inline void plugin_registry_init(struct PluginRegistry *pr) {
	if (!pr) return;
	pr->plugin_count = 0;
	pr->initialized_count = 0;
}

/* Register plugin */
static inline u8 plugin_register(
	struct PluginRegistry *pr,
	const struct PluginMetadata *metadata,
	const struct PluginInterface *interface) {

	if (!pr || !metadata || !interface) return PLUGIN_INVALID;
	if (pr->plugin_count >= 32) return PLUGIN_DEPENDENCY;

	struct Plugin *plugin = &pr->plugins[pr->plugin_count];
	plugin->metadata = *metadata;
	plugin->interface = *interface;
	plugin->loaded = 1;
	plugin->enabled = 1;
	plugin->initialized = 0;

	pr->plugin_count++;
	return PLUGIN_OK;
}

/* Find plugin by name */
static inline struct Plugin *plugin_find(
	struct PluginRegistry *pr,
	const char *name) {

	if (!pr || !name) return 0;

	u32 i;
	for (i = 0; i < pr->plugin_count; i++) {
		struct Plugin *plugin = &pr->plugins[i];
		const char *pname = plugin->metadata.name;

		/* String compare */
		u32 j = 0;
		while (name[j] && pname[j] && name[j] == pname[j]) j++;

		if (name[j] == 0 && pname[j] == 0) {
			return plugin;
		}
	}

	return 0;
}

/* Find plugin by language */
static inline struct Plugin *plugin_find_by_language(
	struct PluginRegistry *pr,
	const char *language) {

	if (!pr || !language) return 0;

	u32 i;
	for (i = 0; i < pr->plugin_count; i++) {
		struct Plugin *plugin = &pr->plugins[i];
		if (!plugin->enabled) continue;

		const char *lang = plugin->metadata.language;

		/* String compare */
		u32 j = 0;
		while (language[j] && lang[j] && language[j] == lang[j]) j++;

		if (language[j] == 0 && lang[j] == 0) {
			return plugin;
		}
	}

	return 0;
}

/* ============================================================ */
/* PLUGIN LIFECYCLE */
/* ============================================================ */

/* Initialize plugin (setup phase) */
static inline u8 plugin_init(struct Plugin *plugin) {
	if (!plugin) return PLUGIN_INVALID;
	if (plugin->initialized) return PLUGIN_OK;

	/* Call plugin init if available */
	plugin->initialized = 1;
	return PLUGIN_OK;
}

/* Finalize plugin (cleanup phase) */
static inline u8 plugin_finalize(struct Plugin *plugin) {
	if (!plugin) return PLUGIN_INVALID;
	if (!plugin->initialized) return PLUGIN_OK;

	plugin->initialized = 0;
	return PLUGIN_OK;
}

/* Enable/disable plugin */
static inline void plugin_set_enabled(struct Plugin *plugin, u8 enabled) {
	if (plugin) plugin->enabled = enabled;
}

/* ============================================================ */
/* PLUGIN DEPENDENCY RESOLUTION */
/* ============================================================ */

/* Check if all dependencies satisfied */
static inline u8 plugin_check_dependencies(
	struct PluginRegistry *pr,
	struct Plugin *plugin) {

	if (!pr || !plugin) return PLUGIN_INVALID;

	if (!plugin->metadata.dependencies) return PLUGIN_OK;

	u32 i = 0;
	while (plugin->metadata.dependencies[i]) {
		const char *dep_name = plugin->metadata.dependencies[i];
		struct Plugin *dep_plugin = plugin_find(pr, dep_name);

		if (!dep_plugin || !dep_plugin->enabled) {
			return PLUGIN_DEPENDENCY;
		}

		i++;
	}

	return PLUGIN_OK;
}

/* ============================================================ */
/* PLUGIN INTERFACE CALLS */
/* ============================================================ */

/* Call plugin compile function */
static inline u8 plugin_compile(
	struct Plugin *plugin,
	const u8 *src,
	u32 src_len,
	u8 *out_code,
	u32 *out_len) {

	if (!plugin || !plugin->interface.compile) return PLUGIN_INVALID;
	if (!plugin->enabled) return PLUGIN_DISABLED;

	return plugin->interface.compile(src, src_len, out_code, out_len);
}

/* Call plugin validate function */
static inline u8 plugin_validate(
	struct Plugin *plugin,
	const u8 *code,
	u32 code_len) {

	if (!plugin || !plugin->interface.validate) return PLUGIN_OK;
	if (!plugin->enabled) return PLUGIN_DISABLED;

	return plugin->interface.validate(code, code_len);
}

/* Call plugin optimize function */
static inline u8 plugin_optimize(
	struct Plugin *plugin,
	u8 *code,
	u32 *code_len,
	u8 level) {

	if (!plugin || !plugin->interface.optimize) return PLUGIN_OK;
	if (!plugin->enabled) return PLUGIN_DISABLED;

	return plugin->interface.optimize(code, code_len, level);
}

/* Call plugin codegen function */
static inline u8 plugin_codegen(
	struct Plugin *plugin,
	const u8 *code,
	u32 code_len,
	u8 *output,
	u32 *out_len) {

	if (!plugin || !plugin->interface.codegen) return PLUGIN_INVALID;
	if (!plugin->enabled) return PLUGIN_DISABLED;

	return plugin->interface.codegen(code, code_len, output, out_len);
}

/* ============================================================ */
/* PLUGIN INFORMATION & REPORTING */
/* ============================================================ */

/* Get plugin by index */
static inline struct Plugin *plugin_get(
	struct PluginRegistry *pr,
	u32 index) {

	if (!pr || index >= pr->plugin_count) return 0;
	return &pr->plugins[index];
}

/* Count loaded plugins */
static inline u32 plugin_count_loaded(struct PluginRegistry *pr) {
	if (!pr) return 0;
	return pr->plugin_count;
}

/* Count enabled plugins */
static inline u32 plugin_count_enabled(struct PluginRegistry *pr) {
	if (!pr) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < pr->plugin_count; i++) {
		if (pr->plugins[i].enabled) count++;
	}
	return count;
}

/* Check if capability available */
static inline u8 plugin_has_capability(
	struct Plugin *plugin,
	u8 capability) {

	if (!plugin) return 0;
	return (plugin->metadata.capabilities & capability) ? 1 : 0;
}

#endif /* APKC_ECO_PLUGIN_SYSTEM_H */
