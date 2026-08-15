/* test_phase12_ecosystem.c — Phase 12 Ecosystem Integration Tests
 *
 * Comprehensive test suite for:
 * - Stage 12.1: Plugin System
 * - Stage 12.2: Package Manager
 * - Stage 12.3: Cross-Language Resolution
 * - Stage 12.4: Runtime Discovery
 *
 * FREESTANDING: No malloc, no libc.
 */

#include <stdio.h>
#include <string.h>

/* Include Phase 12 headers */
#include "Apkc/eco_plugin_system.h"
#include "Apkc/eco_package_manager.h"
#include "Apkc/eco_cross_lang_resolution.h"
#include "Apkc/eco_runtime_discovery.h"

/* ============================================================ */
/* STAGE 12.1: PLUGIN SYSTEM TESTS */
/* ============================================================ */

/* Mock plugin functions */
static u8 mock_compile_kotlin(const u8 *src, u32 src_len, u8 *out, u32 *out_len) {
	if (!src || !out || !out_len) return 1;
	*out_len = 0;
	return 0;
}

static u8 mock_validate_kotlin(const u8 *code, u32 code_len) {
	if (!code) return 1;
	return 0;
}

static int test_plugin_registry_init(void) {
	struct PluginRegistry pr = {0};
	plugin_registry_init(&pr);

	if (pr.plugin_count != 0) {
		printf("❌ test_plugin_registry_init: count not zeroed\n");
		return 1;
	}

	printf("✓ test_plugin_registry_init\n");
	return 0;
}

static int test_plugin_register(void) {
	struct PluginRegistry pr = {0};
	plugin_registry_init(&pr);

	struct PluginMetadata metadata = {
		.name = "kotlin_v2",
		.language = "kotlin",
		.version = {2, 0, 0},
		.capabilities = PLUGIN_COMPILE | PLUGIN_VALIDATE
	};

	struct PluginInterface interface = {
		.compile = mock_compile_kotlin,
		.validate = mock_validate_kotlin
	};

	if (plugin_register(&pr, &metadata, &interface) != PLUGIN_OK) {
		printf("❌ test_plugin_register: registration failed\n");
		return 1;
	}

	if (pr.plugin_count != 1) {
		printf("❌ test_plugin_register: count not incremented\n");
		return 1;
	}

	printf("✓ test_plugin_register\n");
	return 0;
}

static int test_plugin_find_by_language(void) {
	struct PluginRegistry pr = {0};
	plugin_registry_init(&pr);

	struct PluginMetadata metadata = {
		.name = "kotlin_v2",
		.language = "kotlin",
		.version = {2, 0, 0},
		.capabilities = PLUGIN_COMPILE
	};

	struct PluginInterface interface = {
		.compile = mock_compile_kotlin
	};

	plugin_register(&pr, &metadata, &interface);

	struct Plugin *found = plugin_find_by_language(&pr, "kotlin");
	if (!found) {
		printf("❌ test_plugin_find_by_language: plugin not found\n");
		return 1;
	}

	printf("✓ test_plugin_find_by_language\n");
	return 0;
}

static int test_plugin_capability_check(void) {
	struct PluginRegistry pr = {0};
	plugin_registry_init(&pr);

	struct PluginMetadata metadata = {
		.name = "kotlin_v2",
		.language = "kotlin",
		.version = {2, 0, 0},
		.capabilities = PLUGIN_COMPILE | PLUGIN_VALIDATE
	};

	struct PluginInterface interface = {0};
	plugin_register(&pr, &metadata, &interface);

	struct Plugin *plugin = plugin_find_by_language(&pr, "kotlin");
	if (!plugin_has_capability(plugin, PLUGIN_COMPILE)) {
		printf("❌ test_plugin_capability_check: capability not detected\n");
		return 1;
	}

	if (plugin_has_capability(plugin, PLUGIN_OPTIMIZE)) {
		printf("❌ test_plugin_capability_check: false positive\n");
		return 1;
	}

	printf("✓ test_plugin_capability_check\n");
	return 0;
}

/* ============================================================ */
/* STAGE 12.2: PACKAGE MANAGER TESTS */
/* ============================================================ */

static int test_package_registry_init(void) {
	struct PackageRegistry pr = {0};
	package_registry_init(&pr);

	if (pr.package_count != 0) {
		printf("❌ test_package_registry_init: count not zeroed\n");
		return 1;
	}

	printf("✓ test_package_registry_init\n");
	return 0;
}

static int test_package_register(void) {
	struct PackageRegistry pr = {0};
	package_registry_init(&pr);

	struct PackageMetadata metadata = {
		.name = "kotlinx-coroutines",
		.language = "kotlin",
		.version = {1, 6, 0},
		.platform = PLATFORM_ARM64,
		.size_bytes = 128000
	};

	if (package_register(&pr, &metadata, "/opt/packages/kotlinx") != PKG_OK) {
		printf("❌ test_package_register: registration failed\n");
		return 1;
	}

	if (pr.package_count != 1) {
		printf("❌ test_package_register: count not incremented\n");
		return 1;
	}

	printf("✓ test_package_register\n");
	return 0;
}

static int test_version_compare(void) {
	struct PackageVersion v1 = {1, 0, 0};
	struct PackageVersion v2 = {2, 0, 0};

	if (version_compare(v1, v2) >= 0) {
		printf("❌ test_version_compare: comparison failed\n");
		return 1;
	}

	if (version_compare(v2, v1) <= 0) {
		printf("❌ test_version_compare: comparison reversed\n");
		return 1;
	}

	struct PackageVersion v3 = {1, 0, 0};
	if (version_compare(v1, v3) != 0) {
		printf("❌ test_version_compare: equal versions not equal\n");
		return 1;
	}

	printf("✓ test_version_compare\n");
	return 0;
}

static int test_version_satisfies(void) {
	struct PackageVersion actual = {1, 5, 2};
	struct PackageVersion required = {1, 5, 0};

	if (!version_satisfies(actual, required, VER_MIN)) {
		printf("❌ test_version_satisfies: MIN constraint failed\n");
		return 1;
	}

	if (version_satisfies(actual, required, VER_EXACT)) {
		printf("❌ test_version_satisfies: EXACT false positive\n");
		return 1;
	}

	printf("✓ test_version_satisfies\n");
	return 0;
}

/* ============================================================ */
/* STAGE 12.3: CROSS-LANGUAGE RESOLUTION TESTS */
/* ============================================================ */

static int test_resolver_init(void) {
	struct CrossLangResolver clr = {0};
	resolver_init(&clr);

	if (clr.node_count != 0) {
		printf("❌ test_resolver_init: node count not zeroed\n");
		return 1;
	}

	printf("✓ test_resolver_init\n");
	return 0;
}

static int test_parse_python_import(void) {
	struct ImportStatement import = {0};
	const char *python_import = "import requests";

	if (parse_python_import(python_import, &import) != RESOLVE_OK) {
		printf("❌ test_parse_python_import: parsing failed\n");
		return 1;
	}

	if (import.language != IMPORT_PYTHON) {
		printf("❌ test_parse_python_import: language not set\n");
		return 1;
	}

	printf("✓ test_parse_python_import\n");
	return 0;
}

static int test_parse_go_import(void) {
	struct ImportStatement import = {0};
	const char *go_import = "import \"fmt\"";

	if (parse_go_import(go_import, &import) != RESOLVE_OK) {
		printf("❌ test_parse_go_import: parsing failed\n");
		return 1;
	}

	if (import.language != IMPORT_GO) {
		printf("❌ test_parse_go_import: language not set\n");
		return 1;
	}

	printf("✓ test_parse_go_import\n");
	return 0;
}

static int test_resolver_detect_cycle(void) {
	struct CrossLangResolver clr = {0};
	resolver_init(&clr);

	const char *deps1[] = {"pkg2", 0};
	const char *deps2[] = {"pkg1", 0};

	resolver_add_node(&clr, "pkg1", deps1, 1);
	resolver_add_node(&clr, "pkg2", deps2, 1);

	if (resolver_detect_cycle(&clr, "pkg1", 0) != RESOLVE_CIRCULAR) {
		printf("❌ test_resolver_detect_cycle: cycle not detected\n");
		return 1;
	}

	printf("✓ test_resolver_detect_cycle\n");
	return 0;
}

/* ============================================================ */
/* STAGE 12.4: RUNTIME DISCOVERY TESTS */
/* ============================================================ */

static int test_discovery_init(void) {
	struct ServiceRegistry sr = {0};
	discovery_init(&sr);

	if (sr.service_count != 0) {
		printf("❌ test_discovery_init: service count not zeroed\n");
		return 1;
	}

	printf("✓ test_discovery_init\n");
	return 0;
}

static int test_discovery_register_service(void) {
	struct ServiceRegistry sr = {0};
	discovery_init(&sr);

	struct ServiceCapability caps[] = {
		{.operation = "compile", .supported = 1}
	};

	struct RegisteredService service = {
		.name = "kotlin-compiler",
		.endpoint = "localhost:8080",
		.version = {1, 0, 0},
		.capabilities = caps,
		.capability_count = 1,
		.load = 50
	};

	if (discovery_register_service(&sr, &service) != DISCOVERY_OK) {
		printf("❌ test_discovery_register_service: registration failed\n");
		return 1;
	}

	if (sr.service_count != 1) {
		printf("❌ test_discovery_register_service: count not incremented\n");
		return 1;
	}

	printf("✓ test_discovery_register_service\n");
	return 0;
}

static int test_discovery_find_service(void) {
	struct ServiceRegistry sr = {0};
	discovery_init(&sr);

	struct RegisteredService service = {
		.name = "rust-compiler",
		.endpoint = "localhost:9090",
		.version = {1, 0, 0}
	};

	discovery_register_service(&sr, &service);

	struct RegisteredService *found = discovery_find_service(&sr, "rust-compiler");
	if (!found) {
		printf("❌ test_discovery_find_service: service not found\n");
		return 1;
	}

	printf("✓ test_discovery_find_service\n");
	return 0;
}

static int test_discovery_health_check(void) {
	struct ServiceRegistry sr = {0};
	discovery_init(&sr);

	struct RegisteredService service = {
		.name = "go-compiler",
		.version = {1, 0, 0},
		.load = 30
	};

	discovery_register_service(&sr, &service);

	struct RegisteredService *svc = discovery_find_service(&sr, "go-compiler");
	if (discovery_health_check(svc) != DISCOVERY_OK) {
		printf("❌ test_discovery_health_check: health check failed\n");
		return 1;
	}

	printf("✓ test_discovery_health_check\n");
	return 0;
}

static int test_discovery_count_services(void) {
	struct ServiceRegistry sr = {0};
	discovery_init(&sr);

	struct RegisteredService service1 = {.name = "service1", .version = {1, 0, 0}};
	struct RegisteredService service2 = {.name = "service2", .version = {1, 0, 0}};

	discovery_register_service(&sr, &service1);
	discovery_register_service(&sr, &service2);

	if (discovery_count_services(&sr) != 2) {
		printf("❌ test_discovery_count_services: count incorrect\n");
		return 1;
	}

	printf("✓ test_discovery_count_services\n");
	return 0;
}

/* ============================================================ */
/* MAIN TEST RUNNER */
/* ============================================================ */

int main(void) {
	printf("=== Phase 12: Ecosystem Integration Tests ===\n\n");

	int failed = 0;

	printf("Stage 12.1: Plugin System\n");
	failed += test_plugin_registry_init();
	failed += test_plugin_register();
	failed += test_plugin_find_by_language();
	failed += test_plugin_capability_check();

	printf("\nStage 12.2: Package Manager\n");
	failed += test_package_registry_init();
	failed += test_package_register();
	failed += test_version_compare();
	failed += test_version_satisfies();

	printf("\nStage 12.3: Cross-Language Resolution\n");
	failed += test_resolver_init();
	failed += test_parse_python_import();
	failed += test_parse_go_import();
	failed += test_resolver_detect_cycle();

	printf("\nStage 12.4: Runtime Discovery\n");
	failed += test_discovery_init();
	failed += test_discovery_register_service();
	failed += test_discovery_find_service();
	failed += test_discovery_health_check();
	failed += test_discovery_count_services();

	printf("\n=== All Phase 12 tests completed ===\n");
	return failed;
}
