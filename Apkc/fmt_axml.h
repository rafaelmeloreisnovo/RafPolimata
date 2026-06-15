/* fmt_axml.h — Android Binary XML (AXML) encoder for AndroidManifest.xml.
 * Produces a NativeActivity manifest with optional uses-permission and
 * uses-feature elements. No heap. Writes into caller-supplied buffer.
 *
 * Static string pool indices (SI_*) — indices 0..33 are fixed.
 * Dynamic strings (permission names, feature names) follow at indices 34+.
 *
 * String pool layout:
 *   0  ""                  20  "category"
 *   1  "android"           21  PKG_NAME  (caller)
 *   2  NS URI              22  "1.0"
 *   3  "versionCode"       23  APP_LABEL (caller)
 *   4  "versionName"       24  "false"
 *   5  "package"           25  "android.app.NativeActivity"
 *   6  "minSdkVersion"     26  "android.app.lib_name"
 *   7  "targetSdkVersion"  27  LIB_NAME  (caller)
 *   8  "label"             28  "android.intent.action.MAIN"
 *   9  "hasCode"           29  "android.intent.category.LAUNCHER"
 *  10  "name"              30  "uses-permission"
 *  11  "exported"          31  "uses-feature"
 *  12  "value"             32  "required"
 *  13  "manifest"          33  "true"
 *  14  "uses-sdk"          34+ permission names (caller)
 *  15  "application"       34+nperms+ feature names (caller)
 *  16  "activity"
 *  17  "meta-data"
 *  18  "intent-filter"
 *  19  "action"
 */
#pragma once
#include "mem.h"

/* ── String-pool index constants ─────────────────────────────────────────── */
#define SI_EMPTY    0u
#define SI_AND_PFX  1u
#define SI_AND_URI  2u
#define SI_VCODE    3u
#define SI_VNAME    4u
#define SI_PKG      5u
#define SI_MINSDK   6u
#define SI_TGTSDK   7u
#define SI_LABEL    8u
#define SI_HASCODE  9u
#define SI_NAME    10u
#define SI_EXPORTED 11u
#define SI_VALUE   12u
#define SI_EL_MAN  13u
#define SI_EL_SDK  14u
#define SI_EL_APP  15u
#define SI_EL_ACT  16u
#define SI_EL_META 17u
#define SI_EL_FILT 18u
#define SI_EL_ACN  19u
#define SI_EL_CAT  20u
#define SI_PKG_VAL 21u
#define SI_VN_VAL  22u
#define SI_LBL_VAL 23u
#define SI_FALSE   24u
#define SI_NACT    25u
#define SI_LIBKEY  26u
#define SI_LIB_VAL 27u
#define SI_ACTMAIN 28u
#define SI_CATLNCH 29u
#define SI_EL_PERM 30u  /* "uses-permission" */
#define SI_EL_FEAT 31u  /* "uses-feature"    */
#define SI_REQUIRED 32u /* "required"        */
#define SI_TRUE    33u  /* "true"            */
#define SI_COUNT_EX 34u /* total static pool entries */
#define AX_MAX_EXTRA 16u
#define AX_MAX_STRS (SI_COUNT_EX + AX_MAX_EXTRA) /* 50 */

/* Resource IDs for android-namespace attribute names (indices 0..32).
 * Index 32 = SI_REQUIRED = "required" → 0x0101021E.
 * All element-name and value string indices have resID = 0. */
#define AX_RESID_COUNT 33u
static const u32 _ax_resid[AX_RESID_COUNT] = {
    0u, 0u, 0u,              /* 0-2  */
    0x0101021Bu,             /* 3  versionCode  */
    0x0101021Cu,             /* 4  versionName  */
    0u,                      /* 5  package      */
    0x0101020Cu,             /* 6  minSdkVersion */
    0x01010270u,             /* 7  targetSdkVersion */
    0x01010001u,             /* 8  label        */
    0x0101000Cu,             /* 9  hasCode      */
    0x01010003u,             /* 10 name         */
    0x01010010u,             /* 11 exported     */
    0x01010024u,             /* 12 value        */
    /* 13-31: element names and value strings — no resID */
    0u,0u,0u,0u,0u,0u,0u,0u, /* 13-20 */
    0u,0u,0u,0u,0u,0u,0u,0u,0u, /* 21-29 */
    0u,0u,                   /* 30-31 (uses-permission, uses-feature) */
    0x0101021Eu,             /* 32 required */
};

/* Fixed string literals for static pool */
static const char *const _ax_lit[SI_COUNT_EX] = {
    "",
    "android",
    "http://schemas.android.com/apk/res/android",
    "versionCode", "versionName", "package",
    "minSdkVersion", "targetSdkVersion",
    "label", "hasCode", "name", "exported", "value",
    "manifest", "uses-sdk", "application", "activity",
    "meta-data", "intent-filter", "action", "category",
    NULL,         /* SI_PKG_VAL  — caller fills */
    "1.0",
    NULL,         /* SI_LBL_VAL  — caller fills */
    "false",
    "android.app.NativeActivity",
    "android.app.lib_name",
    NULL,         /* SI_LIB_VAL  — caller fills */
    "android.intent.action.MAIN",
    "android.intent.category.LAUNCHER",
    "uses-permission",
    "uses-feature",
    "required",
    "true",
};

/* ── AXML writer ─────────────────────────────────────────────────────────── */
typedef struct { u8 *buf; sz cap; sz pos; } AxWr;

static inline void _ax16(AxWr *a, u16 v){ u8 b[2]; w16(b,v); m_cpy(a->buf+a->pos,b,2); a->pos+=2; }
static inline void _ax32(AxWr *a, u32 v){ u8 b[4]; w32(b,v); m_cpy(a->buf+a->pos,b,4); a->pos+=4; }
static inline void _axb (AxWr *a, const void *src, sz n){ m_cpy(a->buf+a->pos,src,n); a->pos+=n; }
static inline void _ax_patch32(AxWr *a, sz off, u32 v){ w32(a->buf+off,v); }

static inline u32 _ax_utf16(u8 *out, const char *s) {
    u32 n = (u32)s_len(s);
    w16(out, (u16)n);
    for (u32 i = 0; i < n; i++) w16(out+2+i*2, (u16)(u8)s[i]);
    w16(out+2+n*2, 0u);
    return 2u + n*2u + 2u;
}

/* ── Public API ──────────────────────────────────────────────────────────── */
/*
 * axml_build() — generate NativeActivity AndroidManifest.xml (AXML).
 *   perms[0..nperms-1] : android permission strings, e.g. "android.permission.INTERNET"
 *   feats[0..nfeats-1] : android feature strings, e.g. "android.hardware.camera"
 *   NULL/0 for perms or feats = none.  Max 8 of each.
 *   Buffer should be at least 12 KiB when using many permissions.
 */
static sz axml_build(const char *pkg, const char *label,
                     const char *libname, u32 min_sdk, u32 tgt_sdk,
                     const char *const *perms, int nperms,
                     const char *const *feats, int nfeats,
                     u8 *out, sz cap)
{
    AxWr A; A.buf = out; A.cap = cap; A.pos = 0;

    if (nperms < 0) nperms = 0;
    if (nfeats < 0) nfeats = 0;
    if (nperms > 8) nperms = 8;
    if (nfeats > 8) nfeats = 8;

    u32 total_strs = SI_COUNT_EX + (u32)nperms + (u32)nfeats;

    /* Build extended string value array on stack */
    const char *sv[AX_MAX_STRS];
    for (u32 i = 0; i < SI_COUNT_EX; i++) sv[i] = _ax_lit[i];
    sv[SI_PKG_VAL] = pkg;
    sv[SI_LBL_VAL] = label;
    sv[SI_LIB_VAL] = libname;
    for (int i = 0; i < nperms; i++) sv[SI_COUNT_EX + (u32)i] = perms[i];
    for (int i = 0; i < nfeats; i++) sv[SI_COUNT_EX + (u32)nperms + (u32)i] = feats[i];

    /* Compute string pool sizes */
    u32 str_off[AX_MAX_STRS];
    u32 str_data_sz = 0;
    for (u32 i = 0; i < total_strs; i++) {
        str_off[i] = str_data_sz;
        const char *s = sv[i] ? sv[i] : "";
        u32 n = (u32)s_len(s);
        str_data_sz += 2u + n*2u + 2u;
    }
    str_data_sz = u32_aln(str_data_sz, 4u);

    u32 str_off_arr   = 0x1Cu;                          /* pool headerSize = 28 */
    u32 strings_start = str_off_arr + total_strs * 4u;
    u32 sp_chunk_sz   = strings_start + str_data_sz;
    u32 rm_chunk_sz   = 8u + AX_RESID_COUNT * 4u;      /* resource map */

    /* ---- File header ---- */
    sz file_hdr_off = A.pos;
    _ax16(&A, 0x0003u); _ax16(&A, 8u); _ax32(&A, 0u); /* total patched at end */

    /* ---- String pool chunk ---- */
    _ax16(&A, 0x0001u); _ax16(&A, 0x001Cu); _ax32(&A, sp_chunk_sz);
    _ax32(&A, total_strs); _ax32(&A, 0u); _ax32(&A, 0u);
    _ax32(&A, strings_start); _ax32(&A, 0u);
    for (u32 i = 0; i < total_strs; i++) _ax32(&A, str_off[i]);
    sz str_data_start = A.pos;
    for (u32 i = 0; i < total_strs; i++) {
        const char *s = sv[i] ? sv[i] : "";
        u32 n = _ax_utf16(A.buf + A.pos, s);
        A.pos += n;
    }
    while ((A.pos - str_data_start) < str_data_sz) A.buf[A.pos++] = 0;

    /* ---- Resource map chunk ---- */
    _ax16(&A, 0x0180u); _ax16(&A, 8u); _ax32(&A, rm_chunk_sz);
    for (u32 i = 0; i < AX_RESID_COUNT; i++) _ax32(&A, _ax_resid[i]);

    /* ---- XML nodes ---- */
#define _LINE  1u
#define _CMT   0xFFFFFFFFu
#define _NONS  0xFFFFFFFFu

    /* Start namespace */
    _ax16(&A,0x0100u);_ax16(&A,16u);_ax32(&A,24u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,SI_AND_PFX);_ax32(&A,SI_AND_URI);

#define _ATTR(ns,nm,rv,dt,data) do { \
    _ax32(&A,(u32)(ns)); _ax32(&A,(u32)(nm)); _ax32(&A,(u32)(rv)); \
    _ax16(&A,8u); A.buf[A.pos++]=0; A.buf[A.pos++]=(u8)(dt); \
    _ax32(&A,(u32)(data)); \
} while(0)

#define _END(ns,nm) do { \
    _ax16(&A,0x0103u);_ax16(&A,16u);_ax32(&A,24u); \
    _ax32(&A,_LINE);_ax32(&A,_CMT); \
    _ax32(&A,(u32)(ns));_ax32(&A,(u32)(nm)); \
} while(0)

    /* <manifest package="PKG" android:versionCode="1" android:versionName="1.0"> */
    _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,96u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,_NONS);_ax32(&A,SI_EL_MAN);
    _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,3u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);
    _ATTR(_NONS,     SI_PKG,   SI_PKG_VAL, 0x03u, SI_PKG_VAL);
    _ATTR(SI_AND_URI,SI_VCODE, _CMT,       0x10u, 1u);
    _ATTR(SI_AND_URI,SI_VNAME, SI_VN_VAL,  0x03u, SI_VN_VAL);

    /* <uses-sdk android:minSdkVersion="N" android:targetSdkVersion="N"/> */
    _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,76u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,_NONS);_ax32(&A,SI_EL_SDK);
    _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,2u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);
    _ATTR(SI_AND_URI,SI_MINSDK,_CMT,0x10u,min_sdk);
    _ATTR(SI_AND_URI,SI_TGTSDK,_CMT,0x10u,tgt_sdk);
    _END(_NONS,SI_EL_SDK);

    /* <uses-permission android:name="PERM"/> for each perm */
    for (int i = 0; i < nperms; i++) {
        u32 psi = SI_COUNT_EX + (u32)i; /* string pool index for perm name */
        _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,56u);
        _ax32(&A,_LINE);_ax32(&A,_CMT);
        _ax32(&A,_NONS);_ax32(&A,SI_EL_PERM);
        _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,1u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);
        _ATTR(SI_AND_URI,SI_NAME,psi,0x03u,psi);
        _END(_NONS,SI_EL_PERM);
    }

    /* <uses-feature android:name="FEAT" android:required="true"/> for each feat */
    for (int i = 0; i < nfeats; i++) {
        u32 fsi = SI_COUNT_EX + (u32)nperms + (u32)i;
        _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,76u);
        _ax32(&A,_LINE);_ax32(&A,_CMT);
        _ax32(&A,_NONS);_ax32(&A,SI_EL_FEAT);
        _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,2u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);
        _ATTR(SI_AND_URI,SI_NAME,    fsi,      0x03u, fsi);
        _ATTR(SI_AND_URI,SI_REQUIRED,SI_TRUE,  0x12u, 1u); /* required=true */
        _END(_NONS,SI_EL_FEAT);
    }

    /* <application android:label=".." android:hasCode="false"> */
    _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,76u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,_NONS);_ax32(&A,SI_EL_APP);
    _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,2u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);
    _ATTR(SI_AND_URI,SI_LABEL,  SI_LBL_VAL,0x03u,SI_LBL_VAL);
    _ATTR(SI_AND_URI,SI_HASCODE,SI_FALSE,  0x12u,0u);

    /* <activity android:name="NativeActivity" android:label=".." android:exported="true"> */
    _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,96u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,_NONS);_ax32(&A,SI_EL_ACT);
    _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,3u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);
    _ATTR(SI_AND_URI,SI_NAME,    SI_NACT,    0x03u,SI_NACT);
    _ATTR(SI_AND_URI,SI_LABEL,   SI_LBL_VAL, 0x03u,SI_LBL_VAL);
    _ATTR(SI_AND_URI,SI_EXPORTED,_CMT,       0x12u,1u);

    /* <meta-data android:name="android.app.lib_name" android:value="LIBNAME"/> */
    _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,76u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,_NONS);_ax32(&A,SI_EL_META);
    _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,2u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);
    _ATTR(SI_AND_URI,SI_NAME, SI_LIBKEY, 0x03u,SI_LIBKEY);
    _ATTR(SI_AND_URI,SI_VALUE,SI_LIB_VAL,0x03u,SI_LIB_VAL);
    _END(_NONS,SI_EL_META);

    /* <intent-filter> */
    _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,36u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,_NONS);_ax32(&A,SI_EL_FILT);
    _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);

    /* <action android:name="android.intent.action.MAIN"/> */
    _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,56u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,_NONS);_ax32(&A,SI_EL_ACN);
    _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,1u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);
    _ATTR(SI_AND_URI,SI_NAME,SI_ACTMAIN,0x03u,SI_ACTMAIN);
    _END(_NONS,SI_EL_ACN);

    /* <category android:name="android.intent.category.LAUNCHER"/> */
    _ax16(&A,0x0102u);_ax16(&A,16u);_ax32(&A,56u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,_NONS);_ax32(&A,SI_EL_CAT);
    _ax16(&A,0x14u);_ax16(&A,0x14u);_ax16(&A,1u);_ax16(&A,0u);_ax16(&A,0u);_ax16(&A,0u);
    _ATTR(SI_AND_URI,SI_NAME,SI_CATLNCH,0x03u,SI_CATLNCH);
    _END(_NONS,SI_EL_CAT);

    _END(_NONS,SI_EL_FILT);
    _END(_NONS,SI_EL_ACT);
    _END(_NONS,SI_EL_APP);
    _END(_NONS,SI_EL_MAN);

    /* End namespace */
    _ax16(&A,0x0101u);_ax16(&A,16u);_ax32(&A,24u);
    _ax32(&A,_LINE);_ax32(&A,_CMT);
    _ax32(&A,SI_AND_PFX);_ax32(&A,SI_AND_URI);

    /* Patch total file size */
    _ax_patch32(&A, file_hdr_off+4, (u32)A.pos);

#undef _LINE
#undef _CMT
#undef _NONS
#undef _ATTR
#undef _END

    return A.pos;
}
