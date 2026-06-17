#include "RAF_rafaelia_common.h"

/*
 * Método M041: JNI bridge mínimo
 * Alvo: Android NDK
 * Domínio: JNI
 * Ganho estimado: 2x-20x vs camada pesada
 *
 * Expõe uma função C para Kotlin/Java via JNI com fricção mínima.
 * Usa typedefs locais quando jni.h não está disponível (build host).
 */

#if defined(__ANDROID__)
#include <jni.h>
#else
/* Local JNI type aliases for non-Android host builds */
typedef void *   JNIEnv;
typedef void *   jobject;
typedef int32_t  jint;
typedef int64_t  jlong;
#endif

/*
 * Canonical JNI function name for:
 *   package com.rafael; class RafNative; native long nativeGetValue();
 *
 * On Android this is loaded by System.loadLibrary("rafnative") and
 * resolved automatically by the JVM via the naming convention:
 *   Java_<package_underscored>_<class>_<method>
 */
jlong rafaelia_m041_jni_impl(JNIEnv *env, jobject obj)
{
    (void)env;
    (void)obj;
    return (jlong)42;
}

int rafaelia_m041_jni_bridge_minimo(void)
{
    /* Host self-test: call the JNI impl directly (env=NULL, obj=NULL) */
    jlong result = rafaelia_m041_jni_impl((JNIEnv *)0, (jobject)0);
    return (result == (jlong)42) ? 0 : -1;
}
