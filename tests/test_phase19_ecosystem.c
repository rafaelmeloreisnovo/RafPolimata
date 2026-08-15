/* test_phase19_ecosystem.c — Phase 19 Testing (Stages 19.1–19.4)
 *
 * Comprehensive tests for ecosystem analysis and package management.
 *
 * Build: gcc -std=c99 -Wall -O2 -I. -I Apkc tests/test_phase19_ecosystem.c -o test_phase19 && ./test_phase19
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Apkc/eco_module_graph.h"
#include "Apkc/eco_package_registry.h"
#include "Apkc/eco_plugin_discovery.h"
#include "Apkc/eco_ecosystem_health.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

static u32 tests_passed = 0;
static u32 tests_failed = 0;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { \
		printf("FAIL: %s\n", msg); \
		tests_failed++; \
		return 0; \
	} \
} while (0)

#define PASS(msg) do { \
	printf("PASS: %s\n", msg); \
	tests_passed++; \
	return 1; \
} while (0)

static u8 test_graph_init(void) {
	struct ModuleGraph g = {0};
	graph_init(&g);
	ASSERT(g.module_count == 0, "init ok");
	PASS("graph_init");
}

static u8 test_graph_add_module(void) {
	struct ModuleGraph g = {0};
	graph_init(&g);
	u8 r = graph_add_module(&g, "mod_a");
	ASSERT(r && g.module_count == 1, "add ok");
	PASS("graph_add_module");
}

static u8 test_graph_add_dependency(void) {
	struct ModuleGraph g = {0};
	graph_init(&g);
	graph_add_module(&g, "a");
	graph_add_module(&g, "b");
	u8 r = graph_add_dependency(&g, 0, 1);
	ASSERT(r && g.total_edges == 1, "dep ok");
	PASS("graph_add_dependency");
}

static u8 test_graph_has_cycle(void) {
	struct ModuleGraph g = {0};
	graph_init(&g);
	graph_add_module(&g, "a");
	graph_add_module(&g, "b");
	graph_add_dependency(&g, 0, 1);
	graph_add_dependency(&g, 1, 0);
	u8 has_cycle = graph_has_cycle_at_module(&g, 0);
	ASSERT(has_cycle == 1, "cycle detected");
	PASS("graph_has_cycle");
}

static u8 test_registry_init(void) {
	struct PackageRegistry r = {0};
	registry_init(&r);
	ASSERT(r.package_count == 0, "init ok");
	PASS("registry_init");
}

static u8 test_registry_add_package(void) {
	struct PackageRegistry r = {0};
	registry_init(&r);
	u8 res = registry_add_package(&r, "pkg", "1.0.0", "author", "MIT", "desc");
	ASSERT(res && r.package_count == 1, "add ok");
	PASS("registry_add_package");
}

static u8 test_registry_lookup(void) {
	struct PackageRegistry r = {0};
	registry_init(&r);
	registry_add_package(&r, "openssl", "1.1.1", "openssl", "Apache", "ssl");
	struct PackageMetadata *p = registry_lookup_package(&r, "openssl");
	ASSERT(p != 0, "lookup ok");
	PASS("registry_lookup");
}

static u8 test_registry_download(void) {
	struct PackageRegistry r = {0};
	registry_init(&r);
	registry_add_package(&r, "pkg", "1.0", "a", "MIT", "d");
	registry_record_download(&r, 0);
	ASSERT(r.packages[0].download_count == 1, "download ok");
	PASS("registry_download");
}

static u8 test_plugreg_init(void) {
	struct PluginRegistry p = {0};
	plugreg_init(&p);
	ASSERT(p.plugin_count == 0, "init ok");
	PASS("plugreg_init");
}

static u8 test_plugreg_discover(void) {
	struct PluginRegistry p = {0};
	plugreg_init(&p);
	u8 r = plugreg_discover_plugin(&p, "gcc", "/path", "9.0", 1);
	ASSERT(r && p.plugin_count == 1, "discover ok");
	PASS("plugreg_discover");
}

static u8 test_plugreg_load(void) {
	struct PluginRegistry p = {0};
	plugreg_init(&p);
	plugreg_discover_plugin(&p, "gcc", "/path", "9.0", 1);
	u8 r = plugreg_load_plugin(&p, 0);
	ASSERT(r && p.plugins[0].is_loaded, "load ok");
	PASS("plugreg_load");
}

static u8 test_ecohealth_init(void) {
	struct EcosystemHealth h = {0};
	ecohealth_init(&h);
	ASSERT(h.overall_score == 0, "init ok");
	PASS("ecohealth_init");
}

static u8 test_ecohealth_update(void) {
	struct EcosystemHealth h = {0};
	ecohealth_init(&h);
	ecohealth_update_metrics(&h, 100, 80, 5, 1);
	ASSERT(h.metrics.total_packages == 100, "update ok");
	PASS("ecohealth_update");
}

static u8 test_ecohealth_score(void) {
	struct EcosystemHealth h = {0};
	ecohealth_init(&h);
	ecohealth_update_metrics(&h, 100, 90, 5, 0);
	u32 score = ecohealth_calculate_score(&h);
	ASSERT(score > 50, "score ok");
	PASS("ecohealth_score");
}

int main(void) {
	printf("=== Phase 19: Ecosystem & Package Management Tests ===\n\n");

	printf("Stage 19.1: Module Graph\n");
	test_graph_init();
	test_graph_add_module();
	test_graph_add_dependency();
	test_graph_has_cycle();

	printf("\nStage 19.2: Package Registry\n");
	test_registry_init();
	test_registry_add_package();
	test_registry_lookup();
	test_registry_download();

	printf("\nStage 19.3: Plugin Discovery\n");
	test_plugreg_init();
	test_plugreg_discover();
	test_plugreg_load();

	printf("\nStage 19.4: Ecosystem Health\n");
	test_ecohealth_init();
	test_ecohealth_update();
	test_ecohealth_score();

	printf("\n=== Test Results ===\n");
	printf("Passed: %u\n", tests_passed);
	printf("Failed: %u\n", tests_failed);

	if (tests_failed == 0) {
		printf("\nAll tests PASSED! ✓\n");
		return 0;
	}
	return 1;
}
