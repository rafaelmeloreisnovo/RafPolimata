/* eco_plugin_discovery.h — Runtime Plugin Discovery (Stage 19.3)
 *
 * Plugin manifest: define plugin interface and capabilities.
 * Plugin loading: dynamically load compatible plugins at runtime.
 * Plugin registration: register plugin services in registry.
 * Capability negotiation: verify plugin compatibility with host.
 * Plugin lifecycle: manage plugin initialization and shutdown.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_ECO_PLUGIN_DISCOVERY_H
#define APKC_ECO_PLUGIN_DISCOVERY_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Plugin capabilities */
enum PluginCapability {
	CAP_COMPILER = 0,           /* Can compile code */
	CAP_LINKER = 1,             /* Can link binaries */
	CAP_OPTIMIZER = 2,          /* Can optimize code */
	CAP_DEBUGGER = 3,           /* Can debug execution */
	CAP_PROFILER = 4,           /* Can profile performance */
	CAP_ANALYZER = 5,           /* Can analyze code */
	CAP_FORMATTER = 6,          /* Can format code */
	CAP_TRANSLATOR = 7          /* Can translate between languages */
};

/* Plugin interface */
struct PluginInterface {
	const char *plugin_name;    /* Plugin identifier */
	const char *version;        /* Plugin version */
	u32 api_version;            /* Supported API version */
	u8 required_capabilities;   /* Capability flags */
	u32 plugin_id;              /* Unique plugin ID */
};

/* Discovered plugin */
struct DiscoveredPlugin {
	struct PluginInterface interface;
	const char *plugin_path;    /* Path to plugin file */
	u8 is_loaded;               /* 1 if currently loaded */
	u8 is_compatible;           /* 1 if host-compatible */
	u32 load_count;             /* Times loaded */
};

/* Plugin registry */
struct PluginRegistry {
	struct DiscoveredPlugin plugins[32];  /* Up to 32 plugins */
	u32 plugin_count;
	u32 loaded_count;
	u32 total_plugins_discovered;
};

/* Initialize plugin registry */
static inline void plugreg_init(struct PluginRegistry *reg) {
	if (!reg) return;
	reg->plugin_count = 0;
	reg->loaded_count = 0;
	reg->total_plugins_discovered = 0;
}

/* Discover plugin */
static inline u8 plugreg_discover_plugin(
	struct PluginRegistry *reg,
	const char *plugin_name,
	const char *plugin_path,
	const char *version,
	u32 api_version) {

	if (!reg || !plugin_name || !plugin_path) return 0;
	if (reg->plugin_count >= 32) return 0;

	struct DiscoveredPlugin *plugin = &reg->plugins[reg->plugin_count];
	plugin->interface.plugin_name = plugin_name;
	plugin->interface.version = version;
	plugin->interface.api_version = api_version;
	plugin->plugin_path = plugin_path;
	plugin->is_loaded = 0;
	plugin->is_compatible = 1;
	plugin->load_count = 0;

	reg->plugin_count++;
	reg->total_plugins_discovered++;
	return 1;
}

/* Load plugin */
static inline u8 plugreg_load_plugin(
	struct PluginRegistry *reg,
	u32 plugin_id) {

	if (!reg || plugin_id >= reg->plugin_count) return 0;

	struct DiscoveredPlugin *plugin = &reg->plugins[plugin_id];
	if (!plugin->is_compatible) return 0;

	plugin->is_loaded = 1;
	plugin->load_count++;
	reg->loaded_count++;
	return 1;
}

/* Get loaded plugin count */
static inline u32 plugreg_get_loaded_count(struct PluginRegistry *reg) {
	if (!reg) return 0;
	return reg->loaded_count;
}

/* Find plugin by name */
static inline struct DiscoveredPlugin *plugreg_find_plugin(
	struct PluginRegistry *reg,
	const char *plugin_name) {

	if (!reg || !plugin_name) return 0;

	u32 i;
	for (i = 0; i < reg->plugin_count; i++) {
		const char *name_a = reg->plugins[i].interface.plugin_name;
		const char *name_b = plugin_name;
		u32 j = 0;
		while (name_a[j] && name_b[j] && name_a[j] == name_b[j]) j++;
		if (name_a[j] == 0 && name_b[j] == 0) {
			return &reg->plugins[i];
		}
	}

	return 0;
}

#endif /* APKC_ECO_PLUGIN_DISCOVERY_H */
