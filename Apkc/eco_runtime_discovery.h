/* eco_runtime_discovery.h — Runtime Service Discovery (Stage 12.4)
 *
 * Service registry: locate services at runtime.
 * Service capabilities: what operations each service supports.
 * Service health checks: verify service availability.
 * Service routing: select appropriate service for operation.
 * Dynamic binding: late-binding to service implementations.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_ECO_RUNTIME_DISCOVERY_H
#define APKC_ECO_RUNTIME_DISCOVERY_H 1

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Runtime discovery status */
enum DiscoveryStatus {
	DISCOVERY_OK = 0,        /* Service found and healthy */
	DISCOVERY_NOT_FOUND = 1, /* Service not registered */
	DISCOVERY_UNHEALTHY = 2, /* Service registered but unhealthy */
	DISCOVERY_UNAVAILABLE = 3, /* Service capacity exceeded */
	DISCOVERY_VERSION_CONFLICT = 4, /* Service version mismatch */
	DISCOVERY_PERMISSION = 5 /* Access denied to service */
};

/* Service health status */
enum HealthStatus {
	HEALTH_UNKNOWN = 0,
	HEALTH_HEALTHY = 1,
	HEALTH_DEGRADED = 2,
	HEALTH_UNHEALTHY = 3
};

/* Service capability descriptor */
struct ServiceCapability {
	const char *operation;     /* Operation name (compile, optimize, etc.) */
	const char *input_type;    /* Expected input type */
	const char *output_type;   /* Expected output type */
	u32 timeout_ms;            /* Operation timeout */
	u8 supported;              /* 1 if capability supported */
};

/* Service version */
struct ServiceVersion {
	u16 major;
	u16 minor;
	u32 patch;
};

/* Registered service */
struct RegisteredService {
	const char *name;          /* Service name */
	const char *endpoint;      /* Service location/identifier */
	struct ServiceVersion version; /* Service version */
	u8 language;               /* Primary language (if applicable) */
	struct ServiceCapability *capabilities; /* Supported operations */
	u32 capability_count;
	u32 load;                  /* Current load (0-100) */
	u8 health;                 /* HealthStatus */
	u32 last_check_time;       /* Unix timestamp of last health check */
	u64 invocation_count;      /* Total successful invocations */
};

/* Service registry */
struct ServiceRegistry {
	struct RegisteredService services[32];  /* Up to 32 services */
	u32 service_count;
	u32 last_discovery_time;   /* Last registry update time */
};

/* Service request */
struct ServiceRequest {
	const char *operation;     /* Operation to perform */
	const char *language;      /* Target language (if applicable) */
	const u8 *input_data;      /* Input buffer */
	u32 input_size;
	u8 *output_data;           /* Output buffer */
	u32 output_size;
};

/* Service result */
struct ServiceResult {
	u8 status;                 /* DiscoveryStatus */
	const char *service_name;  /* Which service handled request */
	u32 execution_time_ms;     /* Time to execute operation */
	u64 result_size;           /* Bytes written to output */
};

/* ============================================================ */
/* SERVICE REGISTRY MANAGEMENT */
/* ============================================================ */

/* Initialize service registry */
static inline void discovery_init(struct ServiceRegistry *sr) {
	if (!sr) return;
	sr->service_count = 0;
	sr->last_discovery_time = 0;
}

/* Register service */
static inline u8 discovery_register_service(
	struct ServiceRegistry *sr,
	const struct RegisteredService *service) {

	if (!sr || !service || !service->name) return DISCOVERY_NOT_FOUND;
	if (sr->service_count >= 32) return DISCOVERY_UNAVAILABLE;

	struct RegisteredService *svc = &sr->services[sr->service_count];
	*svc = *service;
	svc->health = HEALTH_HEALTHY;
	svc->invocation_count = 0;

	sr->service_count++;
	return DISCOVERY_OK;
}

/* Find service by name */
static inline struct RegisteredService *discovery_find_service(
	struct ServiceRegistry *sr,
	const char *name) {

	if (!sr || !name) return 0;

	u32 i;
	for (i = 0; i < sr->service_count; i++) {
		struct RegisteredService *svc = &sr->services[i];
		const char *svc_name = svc->name;

		/* String compare */
		u32 j = 0;
		while (name[j] && svc_name[j] && name[j] == svc_name[j]) j++;

		if (name[j] == 0 && svc_name[j] == 0) {
			return svc;
		}
	}

	return 0;
}

/* Find service by language */
static inline struct RegisteredService *discovery_find_by_language(
	struct ServiceRegistry *sr,
	u8 language) {

	if (!sr) return 0;

	u32 i;
	for (i = 0; i < sr->service_count; i++) {
		struct RegisteredService *svc = &sr->services[i];
		if (svc->language == language && svc->health == HEALTH_HEALTHY) {
			return svc;
		}
	}

	return 0;
}

/* ============================================================ */
/* SERVICE HEALTH CHECKS */
/* ============================================================ */

/* Check service health */
static inline u8 discovery_health_check(struct RegisteredService *svc) {
	if (!svc) return DISCOVERY_NOT_FOUND;

	/* Simple health check: load < 90% and recent check */
	if (svc->load >= 90) {
		svc->health = HEALTH_DEGRADED;
	}

	return (svc->health == HEALTH_HEALTHY) ? DISCOVERY_OK : DISCOVERY_UNHEALTHY;
}

/* Mark service as unhealthy */
static inline void discovery_mark_unhealthy(struct RegisteredService *svc) {
	if (svc) svc->health = HEALTH_UNHEALTHY;
}

/* Mark service as healthy */
static inline void discovery_mark_healthy(struct RegisteredService *svc) {
	if (svc) svc->health = HEALTH_HEALTHY;
}

/* Update service load */
static inline void discovery_update_load(
	struct RegisteredService *svc,
	u32 new_load) {

	if (!svc) return;
	svc->load = (new_load > 100) ? 100 : new_load;
}

/* ============================================================ */
/* SERVICE CAPABILITY MATCHING */
/* ============================================================ */

/* Check if service supports operation */
static inline u8 discovery_has_capability(
	struct RegisteredService *svc,
	const char *operation) {

	if (!svc || !operation) return 0;

	u32 i;
	for (i = 0; i < svc->capability_count; i++) {
		struct ServiceCapability *cap = &svc->capabilities[i];
		const char *cap_op = cap->operation;

		/* String compare */
		u32 j = 0;
		while (operation[j] && cap_op[j] && operation[j] == cap_op[j]) j++;

		if (operation[j] == 0 && cap_op[j] == 0) {
			return cap->supported;
		}
	}

	return 0;
}

/* Find best service for operation */
static inline struct RegisteredService *discovery_find_for_operation(
	struct ServiceRegistry *sr,
	const char *operation) {

	if (!sr || !operation) return 0;

	struct RegisteredService *best = 0;
	u32 best_score = 0;

	u32 i;
	for (i = 0; i < sr->service_count; i++) {
		struct RegisteredService *svc = &sr->services[i];

		if (svc->health != HEALTH_HEALTHY) continue;
		if (!discovery_has_capability(svc, operation)) continue;

		/* Score: prefer lower load */
		u32 score = 100 - svc->load;
		if (score > best_score) {
			best = svc;
			best_score = score;
		}
	}

	return best;
}

/* ============================================================ */
/* SERVICE INVOCATION & ROUTING */
/* ============================================================ */

/* Route request to best service */
static inline u8 discovery_route_request(
	struct ServiceRegistry *sr,
	struct ServiceRequest *request,
	struct ServiceResult *result) {

	if (!sr || !request || !result) return DISCOVERY_NOT_FOUND;

	struct RegisteredService *svc = discovery_find_for_operation(
		sr, request->operation);

	if (!svc) return DISCOVERY_NOT_FOUND;

	/* Check version compatibility */
	if (svc->version.major == 0) {
		return DISCOVERY_VERSION_CONFLICT;
	}

	/* Would invoke service here in real implementation */
	result->service_name = svc->name;
	result->status = DISCOVERY_OK;
	result->execution_time_ms = 0;
	result->result_size = 0;

	svc->invocation_count++;
	return DISCOVERY_OK;
}

/* ============================================================ */
/* REGISTRY QUERIES & STATISTICS */
/* ============================================================ */

/* Count registered services */
static inline u32 discovery_count_services(struct ServiceRegistry *sr) {
	if (!sr) return 0;
	return sr->service_count;
}

/* Count healthy services */
static inline u32 discovery_count_healthy(struct ServiceRegistry *sr) {
	if (!sr) return 0;

	u32 count = 0;
	u32 i;
	for (i = 0; i < sr->service_count; i++) {
		if (sr->services[i].health == HEALTH_HEALTHY) count++;
	}
	return count;
}

/* Get service by index */
static inline struct RegisteredService *discovery_get_service(
	struct ServiceRegistry *sr,
	u32 index) {

	if (!sr || index >= sr->service_count) return 0;
	return &sr->services[index];
}

/* Get total invocations across all services */
static inline u64 discovery_total_invocations(struct ServiceRegistry *sr) {
	if (!sr) return 0;

	u64 total = 0;
	u32 i;
	for (i = 0; i < sr->service_count; i++) {
		total += sr->services[i].invocation_count;
	}
	return total;
}

/* Get average service load */
static inline u32 discovery_average_load(struct ServiceRegistry *sr) {
	if (!sr || sr->service_count == 0) return 0;

	u64 total_load = 0;
	u32 i;
	for (i = 0; i < sr->service_count; i++) {
		total_load += sr->services[i].load;
	}

	return (u32)(total_load / sr->service_count);
}

#endif /* APKC_ECO_RUNTIME_DISCOVERY_H */
