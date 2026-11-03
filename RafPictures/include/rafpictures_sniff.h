/* RafPictures V1 — freestanding signature detector (no libc, no heap, no I/O).
 * Source type != decoder != runtime gate.
 * A ZIP signature alone NEVER establishes RafPictures identity;
 * higher-level adapter must validate the first "mimetype" entry and manifest.
 */
#ifndef RAFPICTURES_SNIFF_H
#define RAFPICTURES_SNIFF_H

enum rafp_type {
    RAFP_UNKNOWN = 0, RAFP_PNG = 1, RAFP_JPEG = 2,
    RAFP_GIF = 3, RAFP_WEBP = 4, RAFP_TIFF = 5,
    RAFP_BMP = 6, RAFP_EXR = 7, RAFP_QOI = 8,
    RAFP_JXL = 9, RAFP_AVIF = 10, RAFP_HEIC = 11,
    RAFP_HEIF = 12, RAFP_ZIP_CANDIDATE = 255
};

static unsigned rafp_eq4(const unsigned char *p,
                         unsigned char a, unsigned char b,
                         unsigned char c, unsigned char d) {
    return (unsigned)(p[0] == a && p[1] == b &&
                      p[2] == c && p[3] == d);
}

static enum rafp_type rafp_detect(const unsigned char *p, unsigned long n) {
    if (p == (const unsigned char *)0 || n < 2UL)
        return RAFP_UNKNOWN;
    if (n >= 8UL && rafp_eq4(p,137,80,78,71) &&
        rafp_eq4(p+4,13,10,26,10)) return RAFP_PNG;
    if (n >= 3UL && p[0] == 255 && p[1] == 216 &&
        p[2] == 255) return RAFP_JPEG;
    if (n >= 6UL && p[0] == 71 && p[1] == 73 && p[2] == 70 &&
        p[3] == 56 && (p[4] == 55 || p[4] == 57) &&
        p[5] == 97) return RAFP_GIF;
    if (n >= 12UL && rafp_eq4(p,82,73,70,70) &&
        rafp_eq4(p+8,87,69,66,80)) return RAFP_WEBP;
    if (n >= 4UL && (rafp_eq4(p,73,73,42,0) ||
        rafp_eq4(p,77,77,0,42))) return RAFP_TIFF;
    if (p[0] == 66 && p[1] == 77) return RAFP_BMP;
    if (n >= 4UL && rafp_eq4(p,118,47,49,1)) return RAFP_EXR;
    if (n >= 4UL && rafp_eq4(p,113,111,105,102)) return RAFP_QOI;
    if (p[0] == 255 && p[1] == 10) return RAFP_JXL;
    if (n >= 12UL && rafp_eq4(p+4,74,88,76,32) &&
        rafp_eq4(p+8,13,10,135,10)) return RAFP_JXL;
    if (n >= 12UL && rafp_eq4(p+4,102,116,121,112)) {
        if (rafp_eq4(p+8,97,118,105,102) ||
            rafp_eq4(p+8,97,118,105,115)) return RAFP_AVIF;
        if (rafp_eq4(p+8,104,101,105,99) ||
            rafp_eq4(p+8,104,101,105,120) ||
            rafp_eq4(p+8,104,101,118,99) ||
            rafp_eq4(p+8,104,101,118,120)) return RAFP_HEIC;
        if (rafp_eq4(p+8,109,105,102,49) ||
            rafp_eq4(p+8,109,115,102,49)) return RAFP_HEIF;
    }
    if (n >= 4UL && rafp_eq4(p,80,75,3,4))
        return RAFP_ZIP_CANDIDATE;
    return RAFP_UNKNOWN;
}
#endif
