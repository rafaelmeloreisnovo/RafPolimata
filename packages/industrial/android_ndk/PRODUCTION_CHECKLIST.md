# Android NDK — Production Checklist RafPolimata

Checklist de producao para usar os metodos M036-M047 no Android NDK.
Arquivos de referencia na raiz do repositorio: `RAF_036_*.c` a `RAF_047_*.c`.
Cabecalho base: `RAF_rafaelia_common.h`.

---

## ABI Split obrigatorio

Todo APK de producao deve incluir ambas as ABIs para cobertura maxima de
dispositivos. Configuracao minima em `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.18)
project(rafnative C)

# Alvo minimo: API 21 (Android 5.0) — minSDK para arm64-v8a
set(CMAKE_C_STANDARD 11)

add_library(rafnative SHARED
    # Metodos M041-M047 que compoem o core nativo:
    ${CMAKE_SOURCE_DIR}/../../../../RAF_041_jni_bridge_minimo.c
    ${CMAKE_SOURCE_DIR}/../../../../RAF_045_deteccao_de_abi_em_runtime.c
    ${CMAKE_SOURCE_DIR}/../../../../RAF_046_syscall_direta_quando_fizer_sentido.c
    ${CMAKE_SOURCE_DIR}/../../../../RAF_047_ring_buffer_nativo_exposto_ao_kotlin_java.c
    ${CMAKE_SOURCE_DIR}/../../../../RAF_rafaelia_common.h
)

target_include_directories(rafnative PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../../
)
```

`build.gradle` (modulo app):
```groovy
android {
    defaultConfig {
        ndk {
            // ABI split obrigatorio — nao usar abiFilters com apenas uma ABI em producao
            abiFilters "arm64-v8a", "armeabi-v7a"
        }
    }
    splits {
        abi {
            enable true
            reset()
            include "arm64-v8a", "armeabi-v7a"
            universalApk false  // APK universal aumenta tamanho, evitar em producao
        }
    }
}
```

Build via CLI:
```bash
# arm64-v8a:
cmake -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 \
      -DANDROID_NDK=$NDK_PATH \
      -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake \
      -B build_arm64 -S .
cmake --build build_arm64

# armeabi-v7a:
cmake -DANDROID_ABI=armeabi-v7a \
      -DANDROID_PLATFORM=android-21 \
      -DANDROID_NDK=$NDK_PATH \
      -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake \
      -B build_arm32 -S .
cmake --build build_arm32
```

Referencia de metodos de build: `RAF_042_cmake_separado_por_abi.c`,
`RAF_043_build_arm64_v8a.c`, `RAF_044_build_armeabi_v7a.c`.

---

## JNI bridge pattern (M041)

Arquivo: `RAF_041_jni_bridge_minimo.c`

Regras:
- O nome da funcao JNI exportada deve seguir o padrao:
  `Java_<pacote_underscored>_<Classe>_<metodo>`
- O tipo de retorno para valores de 64 bits e `jlong` (int64_t),
  nunca `long` Java diretamente no C — a ABI JNI e diferente entre plataformas.
- Nao usar tipos Java (`jstring`, `jarray`) no hot path — cada conversao
  implica uma chamada ao JNIEnv vtable (indirection via ponteiro de funcao).
- Para valores primitivos (`jlong`, `jint`, `jfloat`): chamada direta, sem overhead.

```c
// Correto — de RAF_041_jni_bridge_minimo.c:
jlong rafaelia_m041_jni_impl(JNIEnv *env, jobject obj) {
    (void)env;
    (void)obj;
    return (jlong)42;
}

// Errado em hot path — conversao de string no caminho critico:
// jstring result = (*env)->NewStringUTF(env, buf);  // aloca heap Java
```

Kotlin/Java — carregamento:
```kotlin
class RafNative {
    external fun nativeGetValue(): Long

    companion object {
        init {
            System.loadLibrary("rafnative")
        }
    }
}
```

O auto-test de `rafaelia_m041_jni_bridge_minimo()` chama a funcao JNI com
`env=NULL, obj=NULL` e verifica que o retorno e 42 — garante que o corpo
da funcao nao depende do JNIEnv para o caminho de execucao nominal.

---

## Ring buffer JNI (M047)

Arquivo: `RAF_047_ring_buffer_nativo_exposto_ao_kotlin_java.c`

Especificacoes do ring buffer de producao:
- Capacidade: 64 entradas (`M047_CAPACITY 64`) — pode ser ajustado para potencia de 2.
- Tipo de entrada: `jlong` (int64_t, 8 bytes) — footprint total: 512 bytes + 12 bytes de controle.
- Acesso single-producer single-consumer (SPSC): sem lock necessario.
- Acesso multi-producer ou multi-consumer: requer lock externo (`pthread_mutex`
  ou `std::atomic` em C++).
- Sem heap: `static rafaelia_ring_m047_t _m047_ring` — alocado em BSS.

Restricoes SPSC que garantem lock-free:
- Um unico thread escreve via `rafaelia_m047_push()`.
- Um unico thread le via `rafaelia_m047_pop()`.
- `head` e `tail` sao `jint` (nao volatile nesta implementacao) — em C11
  puro com SPSC, adicionar `_Atomic` se compilador nao garantir visibilidade
  entre threads sem barreiras explicitamente inseridas.

```c
// Expor push/pop ao Kotlin via JNI:
JNIEXPORT jint JNICALL
Java_com_rafael_RafNative_nativePush(JNIEnv *env, jobject obj, jlong value) {
    (void)env; (void)obj;
    return (jint)rafaelia_m047_push(value);  // -1 se cheio, 0 se ok
}

JNIEXPORT jlong JNICALL
Java_com_rafael_RafNative_nativePop(JNIEnv *env, jobject obj) {
    (void)env; (void)obj;
    jlong v = -1L;
    rafaelia_m047_pop(&v);  // -1L se vazio
    return v;
}
```

---

## Syscall direta (M046)

Arquivo: `RAF_046_syscall_direta_quando_fizer_sentido.c`

Caso de uso: obter TID do thread corrente sem passar pela libc wrapper.
`syscall(SYS_gettid)` tem overhead ~1.2x a 5x menor que `gettid()` libc
em alguns cenarios (evita branch dentro do wrapper VDSO/libc).

```c
// De RAF_046_syscall_direta_quando_fizer_sentido.c:
#include <sys/syscall.h>
#include <unistd.h>

long tid = syscall(SYS_gettid);
// usar para thread tagging em loggers binarios (M048) ou
// para associar prioridade SCHED_FIFO a um TID especifico.
```

Quando NAO usar syscall direta:
- Quando a libc wrapper ja usa VDSO (ex: `clock_gettime` em ARM64 — VDSO
  elimina a transicao de contexto completamente).
- Quando portabilidade entre ARMv7/ARM64/x86 e necessaria — `SYS_gettid`
  tem o mesmo numero em todas, mas outros syscalls divergem.
- Em codigo freestanding Apkc (`Apkc/sys.h`) o padrao e `svc #0` /
  `swi #0` direto — sem libc, sem wrapper.

---

## Afinidade de thread (M036)

Arquivo: `RAF_036_afinidade_de_thread_em_linux_android.c`

```c
// De RAF_036_afinidade_de_thread_em_linux_android.c:
cpu_set_t cpuset;
CPU_ZERO(&cpuset);
CPU_SET(0, &cpuset);
int rc = sched_setaffinity(0, sizeof(cpuset), &cpuset);
// rc == -1 com errno == EPERM em CI/containers — TOKEN_VAZIO, nao falha.
(void)rc;
return 0;
```

Em device real Android:
- `sched_setaffinity` requer `CAP_SYS_NICE` ou root, ou que o processo
  ja esteja rodando no nucleo-alvo como thread principal.
- Em producao NDK: usar `pthread_setaffinity_np` (disponivel no NDK r17+)
  ou `sched_setaffinity` em thread de benchmark dedicado.
- Sempre verificar disponibilidade de CPUs: `sysconf(_SC_NPROCESSORS_ONLN)`.
- EPERM em CI e esperado — testar em device fisico para resultado real.

Prioridade de thread (M037 — `RAF_037_prioridade_de_thread_para_benchmark.c`):
```c
struct sched_param param = { .sched_priority = 1 };
// SCHED_FIFO prio=1 requer CAP_SYS_NICE ou root no Android.
// Em apps comuns: usar Process.setThreadPriority(-19) via Java.
sched_setscheduler(0, SCHED_FIFO, &param);
```

---

## ABI detection em runtime (M045)

Arquivo: `RAF_045_deteccao_de_abi_em_runtime.c`

Verificar `__aarch64__` vs `__arm__` em compile-time para selecionar
caminho otimizado. Em runtime, checar `android_getCpuFamily()` (NDK) ou
ler `/proc/cpuinfo` para fallback seguro:

```c
// De RAF_045_deteccao_de_abi_em_runtime.c — pattern de deteccao:
#if defined(__aarch64__)
    // Usar instrucoes ARM64: cntvct_el0, CRC32CX, PMULL, NEON 128-bit
    return RAF_ABI_ARM64;
#elif defined(__arm__)
    // Usar instrucoes ARM32: NEON 64-bit, CRC32 software
    return RAF_ABI_ARM32;
#else
    return RAF_ABI_UNKNOWN;
#endif
```

Em runtime (quando ABI pode ser determinada apenas em execucao):
```c
#include <cpu-features.h>   // NDK cpu-features
AndroidCpuFamily family = android_getCpuFamily();
uint64_t features = android_getCpuFeatures();
int has_neon = (features & ANDROID_CPU_ARM_FEATURE_NEON) != 0;
```

---

## Restricoes de producao

- **Sem malloc nos hot paths**: conforme invariante do projeto documentado
  em `CLAUDE.md`. Buffers no NDK devem ser estaticos ou stack-allocated.
  O ring buffer M047 (`static rafaelia_ring_m047_t _m047_ring`) demonstra
  o padrao correto.
- **Sem stdio em codigo freestanding NDK**: `printf` de libc nao e disponivel
  em `-nostdlib`. Usar `__android_log_write` (log binario) ou M048
  (`RAF_048_log_binario_em_vez_de_log_textual_pesado.c`).
- **Sem stack overflow**: limites de stack em threads Android variam por
  dispositivo (tipicamente 256 KB-8 MB). Buffers > 64 KB no hot path
  devem ser `static` ou alocados no heap do processo (unica excecao ao
  regra de sem malloc — fora do hot path).
- **Minimo NDK API**: usar `android-21` (Android 5.0) para cobertura maxima.
  `SYS_gettid` e `sched_setaffinity` disponiveis desde API 14.
  `pthread_setaffinity_np` disponivel desde NDK r17 (API 21+).

---

## Checklist rapido pre-release

- [ ] ABI split: `arm64-v8a` e `armeabi-v7a` ambos presentes no APK
- [ ] JNI: funcoes exportadas com nome correto `Java_<pkg>_<cls>_<met>`
- [ ] JNI: retornos de 64 bits usam `jlong`, nao `long`
- [ ] Ring buffer M047: capacidade e potencia de 2; SPSC verificado
- [ ] M046: `SYS_gettid` retorna > 0 em device real
- [ ] M036: EPERM em CI e TOKEN_VAZIO; testar em device fisico
- [ ] M045: ABI detection cobre `__aarch64__` e `__arm__` com fallback
- [ ] Sem `malloc`/`free` no hot path de nenhum metodo M036-M047
- [ ] Build limpo com `-Wall -Wextra` sem warnings em ambas ABIs
- [ ] `RAF_host_syntax_check.sh` passa antes do push
