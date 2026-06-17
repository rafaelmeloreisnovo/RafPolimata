#include "RAF_rafaelia_common.h"

/*
 * Método M047: Ring buffer nativo exposto ao Kotlin/Java
 * Alvo: Android NDK
 * Domínio: Buffer
 * Ganho estimado: baixa cópia
 *
 * Implementa ring buffer de 64 entradas (jlong) com push/pop sem heap.
 * Projetado para exposição via JNI: jlong/jint usam tipos locais quando
 * fora do Android NDK.
 *
 * Status: implementação real ring buffer estático com self-test push/pop.
 */

#if defined(__ANDROID__)
#include <jni.h>
typedef jlong raf_jlong;
typedef jint  raf_jint;
#else
typedef long  raf_jlong;
typedef int   raf_jint;
#endif

#define M047_CAPACITY 64

typedef struct {
    raf_jlong buf[M047_CAPACITY];
    raf_jint  head;
    raf_jint  tail;
    raf_jint  count;
} rafaelia_ring_m047_t;

static rafaelia_ring_m047_t _m047_ring = {.head=0, .tail=0, .count=0};

static int rafaelia_m047_push(raf_jlong v) {
    if (_m047_ring.count >= M047_CAPACITY) return -1;
    _m047_ring.buf[_m047_ring.head % M047_CAPACITY] = v;
    _m047_ring.head++;
    _m047_ring.count++;
    return 0;
}

static int rafaelia_m047_pop(raf_jlong *v) {
    if (_m047_ring.count == 0) return -1;
    *v = _m047_ring.buf[_m047_ring.tail % M047_CAPACITY];
    _m047_ring.tail++;
    _m047_ring.count--;
    return 0;
}

int rafaelia_m047_ring_buffer_nativo_exposto_ao_kotlin_java(void) {
    /* Reset state */
    _m047_ring.head  = 0;
    _m047_ring.tail  = 0;
    _m047_ring.count = 0;

    /* Push two sentinel values */
    rafaelia_m047_push((raf_jlong)0xCAFEL);
    rafaelia_m047_push((raf_jlong)0xBEEFL);

    /* Pop and verify FIFO order */
    raf_jlong v = 0;
    if (rafaelia_m047_pop(&v) != 0 || v != (raf_jlong)0xCAFEL) return -1;
    if (rafaelia_m047_pop(&v) != 0 || v != (raf_jlong)0xBEEFL) return -1;

    /* Buffer must now be empty */
    if (rafaelia_m047_pop(&v) != -1) return -1;

    return 0;
}
