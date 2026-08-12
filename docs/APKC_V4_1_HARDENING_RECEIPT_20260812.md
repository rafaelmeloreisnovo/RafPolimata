# ApkC / RAFAELIA COMPLETE v4.1 HARDENED — Receipt 2026-08-12

**Área:** ApkC / baixo nível Android / evidência operacional  
**Responsável lógico:** subsistema `Apkc/` + qualidade/evidência  
**Ciclo de vida:** `AUDIT / VERIFIED_LIMITED`  
**Entrada relacionada:** `docs/INDEX.md` §4; `docs/AGENTES.md`; `Apkc/PROTOCOL.md`  
**Branch de implantação:** `audit/apkc-v4-1-hardened-20260812`  
**Base canônica do repositório no início:** `a68aa9093e35f9ed2e332501425b2e0f5a33d99b`  
**claim_allowed:** `false`

## 1. Escopo e regra de não regressão

Este receipt registra o hardening de um snapshot externo/standalone chamado `RAFAELIA_COMPLETE_v4.zip` sem substituir nem rebaixar o ApkC canônico já existente neste repositório.

A implantação é **append-only de evidência**. O conteúdo atual de `Apkc/` continua sendo a linha de evolução canônica. Arquivos do snapshot V4 corrigido **não devem ser copiados por cima de `Apkc/`** sem comparação semântica, gates e revisão dedicada, porque o repositório atual contém uma arquitetura mais evoluída (perfis de linguagem, encoders separados, formatos e CI específicos).

## 2. Âncoras criptográficas

| Artefato | SHA-256 | Estado |
|---|---|---|
| `RAFAELIA_COMPLETE_v4.zip` | `d71656be2e649f4344ebebf1e1fdeee75052f3ed9db51551bbb51fefa5cbe454` | base imutável auditada |
| `RAFAELIA_COMPLETE_v4_1_HARDENED.zip` | `168543d2aa76fc083cec8b87e2d7b840230282e247b9f1bdf576da9046f4172c` | release hardened produzido |
| `apkc.o` recebido | `109f2305dda97d70e2feb2f513655522510b193b0a580cbf1ff4062d868b21bd` | AArch64 relocatable |
| `apkc-aarch64` linkado na auditoria | `e2e47a1eb131cfa27ce00e066c237c9ba5ba9212c293cb8f26b9c38fe8b87e6f` | AArch64 static EXEC |
| `hello.apk` histórico | `a331d0248d01d8e7030291e93905c2e2f046cf7cb5ba4ecaf02609cec273c024` | evidência histórica separada |
| `hello-signed.apk` histórico | `063c1b61c35e45f3cf253d42c99bfcd58910162c46ba6c5160846b56651dcc28` | evidência histórica separada |

A relação criptográfica `apkc.o atual -> hello.apk histórico` permanece `TOKEN_VAZIO`: há evidência válida dos dois lados, mas não há build receipt comum que feche identidade causal byte-a-byte.

## 3. Gates executados no ambiente de auditoria

Ambiente: Linux x86_64, Clang 17, Python 3.13.5. Execução ARM física não é inferida a partir de cross-build.

| Gate | Estado | Evidência resumida |
|---|---|---|
| integridade ZIP base | `PASS` | arquivo íntegro e hash congelado |
| C host compile/run | `PASS` | 5/5 sob ASan + UBSan |
| commit gate | `PASS` | `COMMITS=42`, `ROLLBACKS=0` |
| orchestrator | `PASS` | `OK=42`, `CRC_ERR=0` |
| baremetal C object | `PASS` | Clang C11 |
| Java bridge | `PASS` | `javac` |
| ARM32 assemble | `PASS` | 9/9 |
| ARM32 static link | `PASS` | 9/9 ELF32 ARM EABI5 |
| ARM32 physical run | `TOKEN_VAZIO` | device/userspace ARM requerido |
| ApkC object, undefined symbols | `PASS` | 0 undefined; `_start` presente |
| ApkC AArch64 static link | `PASS` | ELF64 AArch64 EXEC |
| ApkC AArch64 physical run | `TOKEN_VAZIO` | host de auditoria não executa AArch64 |
| APK histórico assinado | `PASS_HISTORICAL` | v1/v2/v3 true + pacote instalado |
| current ApkC -> historical APK chain | `TOKEN_VAZIO` | receipt comum ausente |
| NDK JNI `.so` | `TOKEN_VAZIO` | NDK ausente no host de auditoria |
| GPU compute real | `TOKEN_VAZIO` | probe/fallback não equivale a kernel executado |

## 4. Correções materiais no snapshot V4.1

1. Commit gate corrigido para sequência `LOAD|PROC|VERIFY -> COMMIT`.
2. Overflow de 1 byte no orchestrator removido; CRC inicial das camadas e normalização Q16 corrigidos.
3. `baremetal_nomalloc.c`: header, AVX2 preprocessor, `O_CLOEXEC` e include NEON condicionados.
4. `RafaeliaCore.java`: inicialização de `_libLoaded` corrigida.
5. Macro inválida em `rafaelia_jni_direct.c` removida.
6. B1..B8 + `raf_asm_b1.S`: imediatos e formas ARM/NEON corrigidos até 9/9 assemble+link.
7. B5 usa ARMv7VE por `UDIV`; escrita low/high explicitada.
8. Sigma-Ω: índices 0..6 corrigidos.
9. Android NDK: executáveis freestanding `_start` separados da JNI shared library; helper ASM ausente não é anunciado.
10. `build_all.sh` e geradores endurecidos para comportamento fail-closed em gates bloqueantes.

## 5. Evidência histórica preservada

O checkpoint de 2026-06-14 registrava inicialmente apenas compilação do `apkc.o` e deixava a geração do APK como `TOKEN_VAZIO`. Um receipt histórico posterior registra `hello.apk`, `hello-signed.apk`, assinatura v1/v2/v3 e pacote `com.rafael.teste` instalado.

Esses estados não foram fundidos retroativamente. A cadeia temporal é preservada:

```text
checkpoint inicial
  F0/F1 PASS
  F2..F6 TOKEN_VAZIO
        |
        v
receipt histórico posterior
  APK + assinatura + instalação PASS_HISTORICAL
        |
        v
hardening 2026-08-12
  objeto/link/cross-build PASS
  causalidade exata objeto->APK histórico TOKEN_VAZIO
```

## 6. TOKEN_VAZIO ainda aberto

- execução física dos 9 ELFs ARM32;
- execução física do `apkc-aarch64` atual;
- build receipt comum ligando o objeto atual ao APK histórico;
- build NDK atual de `librafaelia_core.so` no snapshot;
- kernel OpenCL/Vulkan computacional real no snapshot;
- golden vectors canônicos de todas as constantes Q16 do snapshot;
- commit canônico de origem do `RAFAELIA_COMPLETE_v4.zip`;
- harmonização de licença top-level do snapshot.

Nenhum desses itens recebe PASS por inferência.

## 7. Implantação Termux / próximo gate físico

No dispositivo, a linha canônica do RafPolimata deve continuar usando a rota já governada do repositório:

```sh
sh scripts/apkc_termux_hermetic_build.sh --abi both
```

O pacote standalone V4.1 também contém um `deploy_termux.sh` fail-closed para reproduzir seus gates próprios. Essa rota é evidência complementar e não substitui o executor canônico acima.

Promoção mínima seguinte:

```text
source
 -> build hermético
 -> APK
 -> unzip/estrutura
 -> readelf ABI(s)
 -> assinatura
 -> instalação
 -> NativeActivity runtime
 -> hashes + receipt
```

## 8. Rollback

Esta implantação não altera `Apkc/` nem remove/move arquivos. Rollback é simplesmente retirar este receipt/entrada de índice na branch/PR; nenhuma implementação canônica precisa ser revertida.

## 9. Fechamento

```text
F_ok   = hardening standalone produzido; 5/5 C sanitizado; 9/9 ARM32 assemble+link; ApkC AArch64 link; prova APK histórica preservada.
F_gap  = gates físicos e elo causal byte-a-byte entre objeto atual e APK histórico.
F_next = executar o build hermético canônico no Termux real e congelar receipt do source até runtime, sem promover TOKEN_VAZIO por inferência.
```
