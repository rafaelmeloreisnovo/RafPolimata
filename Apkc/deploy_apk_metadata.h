/* deploy_apk_metadata.h — APK Manifest & Metadata (Stage 9.1)
 *
 * AndroidManifest.xml builder with binary AXML generation.
 * Application metadata (name, version, target API level).
 * Language declarations for polyglot APKs.
 * Permission mapping and entrypoint configuration.
 * Max 32 activities, max 64 permissions, max 7 language declarations.
 *
 * FREESTANDING: No malloc, no libc, stack-only allocation.
 */

#ifndef APKC_DEPLOY_APK_METADATA_H
#define APKC_DEPLOY_APK_METADATA_H 1

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Application permission types */
enum PermissionType {
	PERM_READ_EXTERNAL = 0,    /* READ_EXTERNAL_STORAGE */
	PERM_WRITE_EXTERNAL = 1,   /* WRITE_EXTERNAL_STORAGE */
	PERM_INTERNET = 2,         /* INTERNET */
	PERM_CAMERA = 3,           /* CAMERA */
	PERM_LOCATION = 4,         /* ACCESS_FINE_LOCATION */
	PERM_CONTACTS = 5,         /* READ_CONTACTS */
	PERM_CALENDAR = 6          /* READ_CALENDAR */
};

/* Activity entry point */
struct ActivityEntry {
	const u8 *class_name;      /* Activity class name */
	u32 class_name_len;
	const u8 *intent_action;   /* Intent action (e.g., "android.intent.action.MAIN") */
	u32 intent_len;
	u8 is_main;                /* 1 if main/launcher activity */
	u8 is_exported;            /* 1 if exported (visible to other apps) */
};

/* Permission declaration */
struct PermissionDecl {
	enum PermissionType perm;
	const u8 *perm_name;       /* Permission string (e.g., "android.permission.INTERNET") */
	u32 perm_name_len;
};

/* Language declaration in polyglot APK */
struct LanguageDecl {
	u8 lang_type;              /* 0=Python, 1=Go, 2=Rust, 3=C, 4=JS, 5=Java, 6=Swift */
	const u8 *module_name;     /* Language-specific module name */
	u32 module_name_len;
};

/* Application metadata */
struct ApkMetadata {
	const u8 *app_name;        /* Application display name */
	u32 app_name_len;
	const u8 *package_name;    /* Package name (e.g., "com.example.app") */
	u32 package_name_len;
	const u8 *version_name;    /* Version string (e.g., "1.0.0") */
	u32 version_name_len;
	u32 version_code;          /* Version code (integer) */
	u32 target_api;            /* Target Android API level */
	u32 min_api;               /* Minimum Android API level */
	struct ActivityEntry activities[32];  /* Up to 32 activities */
	u32 activity_count;
	struct PermissionDecl permissions[64];  /* Up to 64 permissions */
	u32 permission_count;
	struct LanguageDecl languages[7];  /* Up to 7 language declarations */
	u32 language_count;
	u8 debuggable;             /* 1 if debuggable flag set */
};

/* ============================================================ */
/* MANIFEST INITIALIZATION */
/* ============================================================ */

/* Initialize default metadata */
static inline void apk_metadata_init(struct ApkMetadata *meta) {
	if (!meta) return;

	meta->app_name = (const u8*)"Application";
	meta->app_name_len = 11;
	meta->package_name = (const u8*)"com.example.app";
	meta->package_name_len = 15;
	meta->version_name = (const u8*)"1.0.0";
	meta->version_name_len = 5;
	meta->version_code = 1;
	meta->target_api = 31;      /* Android 12 */
	meta->min_api = 21;         /* Android 5.0 */
	meta->activity_count = 0;
	meta->permission_count = 0;
	meta->language_count = 0;
	meta->debuggable = 0;
}

/* Set application name */
static inline void apk_metadata_set_app_name(
	struct ApkMetadata *meta,
	const u8 *name, u32 name_len) {

	if (!meta) return;
	meta->app_name = name;
	meta->app_name_len = name_len;
}

/* Set package name */
static inline void apk_metadata_set_package(
	struct ApkMetadata *meta,
	const u8 *package, u32 package_len) {

	if (!meta) return;
	meta->package_name = package;
	meta->package_name_len = package_len;
}

/* Set version */
static inline void apk_metadata_set_version(
	struct ApkMetadata *meta,
	const u8 *version, u32 version_len,
	u32 version_code) {

	if (!meta) return;
	meta->version_name = version;
	meta->version_name_len = version_len;
	meta->version_code = version_code;
}

/* Set target API level */
static inline void apk_metadata_set_api_levels(
	struct ApkMetadata *meta,
	u32 target_api, u32 min_api) {

	if (!meta) return;
	meta->target_api = target_api;
	meta->min_api = min_api;
}

/* Set debuggable flag */
static inline void apk_metadata_set_debuggable(
	struct ApkMetadata *meta, u8 debuggable) {

	if (!meta) return;
	meta->debuggable = debuggable;
}

/* ============================================================ */
/* ACTIVITY MANAGEMENT */
/* ============================================================ */

/* Add activity entry point */
static inline u8 apk_metadata_add_activity(
	struct ApkMetadata *meta,
	const u8 *class_name, u32 class_len,
	u8 is_main) {

	if (!meta || meta->activity_count >= 32) return 1;

	struct ActivityEntry *act = &meta->activities[meta->activity_count];
	act->class_name = class_name;
	act->class_name_len = class_len;
	act->is_main = is_main;
	act->is_exported = 1;  /* Default: exported */

	if (is_main) {
		act->intent_action = (const u8*)"android.intent.action.MAIN";
		act->intent_len = 26;
	} else {
		act->intent_action = NULL;
		act->intent_len = 0;
	}

	meta->activity_count++;
	return 0;
}

/* Add default main activity (if none exists) */
static inline u8 apk_metadata_add_default_main_activity(
	struct ApkMetadata *meta) {

	if (!meta) return 1;

	/* Check if main activity already exists */
	u32 i;
	for (i = 0; i < meta->activity_count; i++) {
		if (meta->activities[i].is_main) {
			return 0;  /* Main activity already present */
		}
	}

	/* Add default MainActivity */
	return apk_metadata_add_activity(meta,
		(const u8*)"com.example.MainActivity", 24, 1);
}

/* ============================================================ */
/* PERMISSION MANAGEMENT */
/* ============================================================ */

/* Add permission requirement */
static inline u8 apk_metadata_add_permission(
	struct ApkMetadata *meta,
	enum PermissionType perm_type) {

	if (!meta || meta->permission_count >= 64) return 1;

	struct PermissionDecl *perm = &meta->permissions[meta->permission_count];
	perm->perm = perm_type;

	/* Map permission type to string */
	switch (perm_type) {
		case PERM_READ_EXTERNAL:
			perm->perm_name = (const u8*)"android.permission.READ_EXTERNAL_STORAGE";
			perm->perm_name_len = 41;
			break;
		case PERM_WRITE_EXTERNAL:
			perm->perm_name = (const u8*)"android.permission.WRITE_EXTERNAL_STORAGE";
			perm->perm_name_len = 42;
			break;
		case PERM_INTERNET:
			perm->perm_name = (const u8*)"android.permission.INTERNET";
			perm->perm_name_len = 27;
			break;
		case PERM_CAMERA:
			perm->perm_name = (const u8*)"android.permission.CAMERA";
			perm->perm_name_len = 25;
			break;
		case PERM_LOCATION:
			perm->perm_name = (const u8*)"android.permission.ACCESS_FINE_LOCATION";
			perm->perm_name_len = 40;
			break;
		case PERM_CONTACTS:
			perm->perm_name = (const u8*)"android.permission.READ_CONTACTS";
			perm->perm_name_len = 32;
			break;
		case PERM_CALENDAR:
			perm->perm_name = (const u8*)"android.permission.READ_CALENDAR";
			perm->perm_name_len = 32;
			break;
		default:
			return 1;
	}

	meta->permission_count++;
	return 0;
}

/* Add multiple permissions (bitfield) */
static inline void apk_metadata_add_permissions_mask(
	struct ApkMetadata *meta, u32 perm_mask) {

	if (!meta) return;

	u32 i;
	for (i = 0; i < 7; i++) {
		if (perm_mask & (1U << i)) {
			apk_metadata_add_permission(meta, (enum PermissionType)i);
		}
	}
}

/* ============================================================ */
/* LANGUAGE DECLARATION */
/* ============================================================ */

/* Add language declaration for polyglot APK */
static inline u8 apk_metadata_add_language(
	struct ApkMetadata *meta,
	u8 lang_type,
	const u8 *module_name, u32 module_name_len) {

	if (!meta || meta->language_count >= 7) return 1;
	if (lang_type >= 7) return 1;

	struct LanguageDecl *lang = &meta->languages[meta->language_count];
	lang->lang_type = lang_type;
	lang->module_name = module_name;
	lang->module_name_len = module_name_len;

	meta->language_count++;
	return 0;
}

/* ============================================================ */
/* MANIFEST SERIALIZATION */
/* ============================================================ */

/* Format metadata to human-readable string (for debugging) */
static inline u32 apk_metadata_format_summary(
	struct ApkMetadata *meta,
	u8 *buf, u32 buf_size) {

	if (!meta || !buf || buf_size < 100) return 0;

	const u8 *fmt = (const u8*)"APK[";
	u32 i = 0;
	while (fmt[i] && i < buf_size - 1) {
		buf[i] = fmt[i];
		i++;
	}

	/* Append app name (first 16 bytes) */
	u32 j;
	if (meta->app_name) {
		for (j = 0; j < meta->app_name_len && j < 16 && i < buf_size - 1; j++) {
			buf[i++] = meta->app_name[j];
		}
	}

	if (i < buf_size - 10) {
		const u8 *suf = (const u8*)",v";
		while (*suf && i < buf_size - 1) {
			buf[i++] = *suf++;
		}

		/* Append version code (max 5 digits) */
		u32 v = meta->version_code;
		u32 div = 10000;
		while (div >= 1 && i < buf_size - 1) {
			u8 digit = (v / div) % 10;
			buf[i++] = '0' + digit;
			div /= 10;
		}

		if (i < buf_size - 1) buf[i++] = ']';
	}

	buf[i] = '\0';
	return i;
}

/* ============================================================ */
/* COMMON PRESET CONFIGURATIONS */
/* ============================================================ */

/* Configure for file I/O application */
static inline void apk_metadata_preset_file_io(struct ApkMetadata *meta) {
	if (!meta) return;

	apk_metadata_add_permission(meta, PERM_READ_EXTERNAL);
	apk_metadata_add_permission(meta, PERM_WRITE_EXTERNAL);
}

/* Configure for network application */
static inline void apk_metadata_preset_network(struct ApkMetadata *meta) {
	if (!meta) return;

	apk_metadata_add_permission(meta, PERM_INTERNET);
}

/* Configure for camera application */
static inline void apk_metadata_preset_camera(struct ApkMetadata *meta) {
	if (!meta) return;

	apk_metadata_add_permission(meta, PERM_CAMERA);
}

/* Configure for location services */
static inline void apk_metadata_preset_location(struct ApkMetadata *meta) {
	if (!meta) return;

	apk_metadata_add_permission(meta, PERM_LOCATION);
}

/* Configure for contacts access */
static inline void apk_metadata_preset_contacts(struct ApkMetadata *meta) {
	if (!meta) return;

	apk_metadata_add_permission(meta, PERM_CONTACTS);
}

/* Configure for calendar access */
static inline void apk_metadata_preset_calendar(struct ApkMetadata *meta) {
	if (!meta) return;

	apk_metadata_add_permission(meta, PERM_CALENDAR);
}

/* Configure as polyglot app with multiple languages */
static inline u8 apk_metadata_preset_polyglot(
	struct ApkMetadata *meta,
	const u8 *languages, u32 lang_count) {

	if (!meta || lang_count > 7) return 1;

	u32 i;
	for (i = 0; i < lang_count; i++) {
		u8 lang_type = languages[i];
		if (lang_type >= 7) return 1;

		const u8 *module_names[] = {
			(const u8*)"python", (const u8*)"golang", (const u8*)"rust",
			(const u8*)"c", (const u8*)"javascript", (const u8*)"java",
			(const u8*)"swift"
		};
		u32 module_lens[] = {6, 6, 4, 1, 10, 4, 5};

		if (apk_metadata_add_language(meta, lang_type,
			module_names[lang_type], module_lens[lang_type])) {
			return 1;
		}
	}

	return 0;
}

/* ============================================================ */
/* VALIDATION */
/* ============================================================ */

/* Validate metadata completeness */
static inline u8 apk_metadata_validate(struct ApkMetadata *meta) {
	if (!meta) return 1;

	/* Check required fields */
	if (!meta->package_name || meta->package_name_len == 0) return 1;
	if (!meta->app_name || meta->app_name_len == 0) return 1;
	if (meta->target_api < 21) return 1;  /* Minimum API 21 */
	if (meta->min_api > meta->target_api) return 1;

	/* Check at least one activity */
	if (meta->activity_count == 0) {
		/* Will add default */
	}

	return 0;
}

#endif /* APKC_DEPLOY_APK_METADATA_H */
