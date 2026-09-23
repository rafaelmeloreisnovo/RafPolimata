# Linguagens — contrato, fonte presente e fronteira de prova

**Governance binding: CLOSURE_L11** — explicit unknown-state markers are governed by the operational gap topology closure; the binding does not promote the underlying gap.

**Base:** main@f22efc099ac530d946ff2ec34954455f75632e92  
**Contrato legível por máquina:** ci/contracts/rafaelia_language_completion_v1.tsv  
**Dispatch:** Apkc/lang_profile.h

> [!IMPORTANT]
> Perfil reconhecido ≠ frontend completo ≠ artefato estrito ≠ execução.

## 1. Matriz canônica de 23 perfis

| ID | Linguagem/perfil | Pipeline declarado | Build tool | Rota estrita | Classe final |
|---:|---|---|---|---|---|
| 0 | ASM | ASM_INTERNAL | none | DIRECT_ISA | STANDALONE |
| 1 | C | NATIVE_SO | clang | NATIVE_SUBSET | STANDALONE |
| 2 | C++ | NATIVE_SO | clang++ | NATIVE_SUBSET | CONDITIONAL |
| 3 | Rust | NATIVE_SO | rustc | NATIVE_SUBSET | CONDITIONAL |
| 4 | Kotlin | DEX | kotlinc+d8 | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 5 | Java | DEX | javac+d8 | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 6 | Python | SCRIPT_BOOTSTRAP | python3 | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 7 | Shell | SCRIPT_BOOTSTRAP | sh | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 8 | Perl | SCRIPT_BOOTSTRAP | perl | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 9 | JavaScript | SCRIPT_BOOTSTRAP | node | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 10 | PHP | SCRIPT_BOOTSTRAP | php | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 11 | JSX | JSX_BOOTSTRAP | npx+babel+node | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 12 | Go | NATIVE_SO | go | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 13 | Ruby | SCRIPT_BOOTSTRAP | ruby | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 14 | Swift | NATIVE_SO | swiftc | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 15 | Groovy | DEX | groovyc+d8 | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 16 | Clojure | SCRIPT_BOOTSTRAP | clojure | LOWER_TO_RAF_IR | REQUIRES_LOWERING |
| 17 | GLSL | GPU_SPV | glslc | DEVICE_KERNEL | DEVICE_ONLY |
| 18 | OpenCL | GPU_CL | device_driver | DEVICE_KERNEL | DEVICE_ONLY |
| 19 | HLSL | GPU_SPV | glslc | DEVICE_KERNEL | DEVICE_ONLY |
| 20 | WGSL | GPU_WGSL | device_driver | DEVICE_KERNEL | DEVICE_ONLY |
| 21 | DSP | DSP | hexagon-clang | DEVICE_KERNEL | CONDITIONAL |
| 22 | TFLite | NPU_MODEL | model_blob | MODEL_TO_INTERNAL_KERNELS | DATA_ONLY |

## 2. Linguagens pedidas — presença física atual

| Linguagem | Fonte encontrada no tree | Leitura correta |
|---|---:|---|
| C | 176 arquivos .c | corpo nativo amplo; ainda depende de gate por componente |
| C++ | 2 arquivos .cpp | implementação real existe, porém corpus pequeno |
| Rust | 1 arquivo .rs | implementação real existe no RAF Hash Fabric; sem Cargo workspace observado |
| Python | 214 arquivos .py | forte presença em tooling, validação, pesquisa e orquestração |
| Kotlin | 0 arquivos .kt | dispatch/rota DEX existe; fonte Kotlin corrente não foi observada |
| Java | 0 arquivos .java | rota Java/DEX e provas históricas existem; fonte Java corrente não foi observada |

### C++

Fonte observada:
- native/raf_hash_fabric_v1/cpp/raf_hash_fabric.cpp;
- tests/fixtures/strict_kernel.cpp.

O perfil estrito desliga exceptions, RTTI, thread-safe statics e cxa atexit. C++ continua condicional até símbolo/runtime passarem o gate.

### Rust

Fonte observada:
- native/raf_hash_fabric_v1/rust/lib.rs.

O Makefile desse módulo usa rustc diretamente com panic=abort. Não foi observado Cargo.toml; portanto a documentação não chama isso de workspace Cargo.

O contrato estrito exige:
- no_std;
- sem alloc;
- panic=abort;
- runtime externo ausente no artefato final;
- gate de símbolos.

### Python

Python é a principal linguagem de automação observada. A rota estrita, contudo, classifica Python como frontend que precisa lowering quando o objetivo é um artefato final freestanding sem interpretador.

Python de tooling e Python como linguagem de entrada são papéis diferentes.

### Kotlin e Java

Apkc/lang_profile.h possui perfis DEX para Kotlin e Java. O contrato marca ambos como REQUIRES_LOWERING para o final estrito.

O repositório contém scripts, documentação e provas históricas de Java/DEX, mas o tree corrente não contém arquivos fonte .kt ou .java. Isso deve permanecer explícito até uma fixture ou fonte ser versionada.

## 3. RAF Hash Fabric — exceção útil de implementação multilíngue

native/raf_hash_fabric_v1 documenta implementações em:
- C11;
- C++20 subset;
- Rust no_std;
- Zig;
- D betterC;
- Forth;
- Assembly.

O estado do README desse módulo é IMPLEMENTED source only; runtime evidence continua TOKEN_VAZIO. A licença PolyForm ali é de escopo do módulo e não deve ser extrapolada para o repositório inteiro.

## 4. Semântica comum

A rota arquitetural pretendida para subconjuntos suportados é:

~~~text
surface syntax
-> typed operation
-> canonical semantics
-> RAF Semantic IR
-> strict kernel
-> target backend
-> artifact
-> execution/evidence
~~~

A equivalência deve ser provada por vetores e resultado, não por semelhança sintática.

## 5. Evidência histórica relevante

docs/RAF_SEMANTIC_LANGUAGE_SEVEN_ARCH_V2.md registra um passe local histórico com:
- 10 semantic frontends;
- 4 vectors;
- semantic_equivalence PASS;
- freestanding C object PASS;
- claim_allowed=false.

Esse snapshot continua válido no escopo da revisão em que foi produzido; não substitui um rerun no main atual.

R3 = ⟨F_ok: contrato de 23 perfis separado da presença real; F_gap: Kotlin/Java sem fonte corrente e Rust sem Cargo workspace; F_next: adicionar fixtures/gates apenas onde houver necessidade de implementação e evidência, sem fabricar cobertura⟩.
