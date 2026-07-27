# M063 — Metodologia RAFAELIA para completar linguagens e assimilar bibliotecas

**Estado:** `FORMULATED / STATIC_CORE_PASS_LOCAL / LANGUAGE_FRONTENDS_INCREMENTAL`  
**Escopo:** `RafPolimata`, `ApkC`, bibliotecas externas e kernels para ARM32/ARM64/NEON/GPU/DSP/NPU.  
**Regra:** uma linguagem reconhecida pelo ApkC não é automaticamente uma linguagem freestanding.

## 1. Invariante

Toda linguagem ou biblioteca deve convergir para o mesmo contrato final:

```text
fonte estrangeira
→ inventário de runtime
→ seleção de kernels puros
→ lowering para RAF IR / C estrito / ASM
→ deduplicação de função
→ buffers estáticos ou do caller
→ patch seletivo de bits
→ plano de até 16 lanes independentes
→ código por arquitetura
→ gate de símbolo/relocação/runtime
→ binário mínimo + prova por ciclos
```

O compilador externo pode existir no **plano de construção**, desde que nenhuma dependência invisível seja carregada para o **plano de execução**. A ferramenta usada para traduzir não recebe autoridade sobre o binário final.

## 2. Contrato do binário estrito

O perfil estrito exige, conforme o tipo de artefato:

- freestanding;
- zero `malloc`, `calloc`, `realloc` e `free`;
- zero heap e zero GC;
- zero interpretador ou runtime gerenciado no artefato final;
- zero libc ou biblioteca nativa externa não explicitamente assimilada;
- zero exceções, RTTI, reflexão, TLS e inicializadores ocultos;
- zero tail-call optimization no caminho auditado;
- zero variável sombreada aceita pelo compilador (`-Wshadow -Werror`);
- zero unwind e metadados dispensáveis;
- símbolos ocultos e remoção das seções não alcançáveis;
- ausência de build-id quando a reprodutibilidade bit a bit exigir;
- zero símbolo indefinido no núcleo final;
- memória estática ou buffer fornecido pelo caller;
- nenhuma função duplicada para a mesma operação canônica;
- decisão e benchmark por **ciclos**, não por alegação de clock.

`branchless` é requisito de hot path quando preserva a semântica e reduz dependência de controle. Não é aplicado cegamente: uma transformação só entra quando a equivalência é bit-exata e a contagem de ciclos não piora no alvo real.

## 3. O que “completar uma linguagem” significa

Completar não significa empacotar o interpretador, a VM ou a biblioteca padrão inteira. Significa criar uma rota verificável para o subconjunto necessário:

```text
sintaxe aceita
→ AST/ops conhecidos
→ tipos de largura fixa
→ ownership explícito
→ erro por código/flags
→ operações canônicas únicas
→ RAF IR
→ backend ASM/ISA
```

Recursos que dependem de runtime permanecem rejeitados ou `TOKEN_VAZIO` até lowering próprio.

## 4. Classificação das 23 entradas atuais

| Classe | Linguagens/perfis | Regra estrita |
|---|---|---|
| Direto | ASM | Encoder interno; nenhuma ferramenta ou runtime final |
| Nativo estrito | C | Subset freestanding; syscalls/ISA próprios; zero libc |
| Nativo condicional | C++, Rust | Só após remover exceptions/RTTI/alloc/runtime e passar o gate de símbolos |
| Lowering obrigatório | Kotlin, Java, Python, Shell, Perl, JavaScript, PHP, JSX, Go, Ruby, Swift, Groovy, Clojure | O pipeline hospedado continua útil como referência, mas o final estrito deve baixar para RAF IR/C/ASM |
| Kernel de dispositivo | GLSL, OpenCL, HLSL, WGSL, DSP | O kernel pode ser estrito; driver/loader do host fica fora do claim |
| Dados/modelo | TFLite | O arquivo é modelo, não linguagem; operadores devem ser convertidos em kernels internos |

A matriz legível por máquina está em `ci/contracts/rafaelia_language_completion_v1.tsv`. A política C está em `Apkc/lang_freestanding_policy.h`.

## 5. Assimilação de bibliotecas externas

Uma biblioteca externa não é vinculada integralmente por conveniência. Ela é decomposta:

1. **Proveniência:** commit, hash, licença, API e arquitetura.
2. **Inventário:** heap, GC, exceções, threads, TLS, I/O, syscalls, reflexão e dynamic loading.
3. **Seleção:** somente kernels puros ou trechos cuja equivalência possa ser testada.
4. **Normalização:** inteiros de largura fixa, endianness explícita e estado passado por parâmetro.
5. **Memória:** trocar alocação por buffer do caller, arena estática ou streaming.
6. **Erro:** trocar throw/panic/exceção por status, máscara ou `TOKEN_VAZIO` tipado.
7. **Deduplicação:** uma operação recebe um `canonical_kernel_id`; wrappers sobrepostos apontam para a mesma implementação.
8. **Lowering:** converter para RAF IR, C estrito ou ASM, sem trazer o runtime de origem.
9. **Equivalência:** vetores fixos com comparação bit a bit entre referência e kernel assimilado.
10. **Gate final:** símbolo, dependência, relocação, seção, tamanho, hash e ciclos.

O manifesto de entrada está em `docs/templates/RAFAELIA_LIBRARY_ASSIMILATION_MANIFEST.yaml`.

## 6. Sem reescrever blocos inteiros

A operação canônica de patch é:

\[
novo = atual \oplus ((atual \oplus valor) \land máscara).
\]

Somente os bits selecionados pela máscara mudam semanticamente. A implementação está em `rafaelia/raf_lane16_patch.h`.

Limite físico: memória comum normalmente grava byte, palavra, linha de cache, página ou bloco de storage. Portanto, “trocar um bit” significa **read-modify-write mascarado** no menor grão suportado. Para MMIO, flash e storage, o backend deve declarar a granularidade física e o custo real; não se afirma escrita física de um único bit quando o hardware regrava uma palavra ou página.

## 7. Sobreposição sem duplicação

Funções sobrepostas são tratadas por identidade semântica:

```text
assinatura de entrada
+ largura dos tipos
+ operação
+ política de overflow
+ endianness
= canonical_kernel_id
```

Dois wrappers com o mesmo ID apontam para um único kernel. Especializações de ARM32, ARM64, NEON, GPU ou DSP são backends da mesma operação, não novas definições da função.

## 8. Plano determinístico de até 16 lanes

O núcleo `rafaelia/raf_lane16_patch.h` reserva dezesseis domínios:

```text
CPU0 CPU1 CPU2 CPU3 CPU4 CPU5 CPU6 CPU7
SIMD NEON GPU CRC32_SW CACHE_L1 CACHE_L2 RAM_BUFFER STORAGE
```

Cada lane declara dependências e recurso. Lanes independentes podem ser emitidas em paralelo ou sobrepostas pelo backend. O plano não promete que uma CPU execute dezesseis instruções no mesmo ciclo: issue width, portas, hazards, cache misses e driver são medidos no hardware.

A regra é:

```text
independência comprovada → paralelismo permitido
colisão de dados/recurso → ordem determinística
capacidade não provada → execução serial ou TOKEN_VAZIO
```

## 9. Orquestração de memória

- **L1:** dados quentes, pequenos e reutilizados imediatamente;
- **L2:** tabelas compartilhadas e kernels já normalizados;
- **buffer/RAM:** staging estático com capacidade fixa;
- **storage:** commit por patch/registro quando o meio permitir; caso contrário, regravação mínima alinhada ao bloco físico;
- **CRC32 software/hardware:** lane própria, sem misturar verificação com transformação;
- **GPU/SIMD/NEON:** somente dados independentes, alinhados e com fallback bit-exato.

Prefetch, cache hint e unrolling não são ativados por estética. Entram quando o contador de ciclos confirma redução de fricção.

## 10. Flags canônicas

### C estrito

```text
-ffreestanding -fno-builtin -nostdlib -nostartfiles -nodefaultlibs
-fno-stack-protector -fno-ident
-fno-unwind-tables -fno-asynchronous-unwind-tables
-fno-optimize-sibling-calls
-fvisibility=hidden -ffunction-sections -fdata-sections
-Wall -Wextra -Werror -Wshadow
-Wl,--gc-sections -Wl,--strip-all -Wl,--build-id=none
```

O desligamento de stack protector é um perfil mínimo para evitar helper externo. Um perfil endurecido pode manter proteção somente se `__stack_chk_guard` e `__stack_chk_fail` forem internos e auditados.

### C++ estrito

Além do perfil C:

```text
-fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit
```

`new`, `delete`, containers alocadores, iostream, locale e inicialização global não entram.

### Rust estrito

```text
#![no_std]
sem alloc
panic=abort
LTO + codegen-units=1
```

A saída só é aceita depois de provar zero runtime externo, zero símbolo indefinido e helpers internos para o alvo.

## 11. Gates de artefato

Para ELF executável estrito, o contrato final procura:

- nenhum `PT_INTERP`;
- nenhum `PT_DYNAMIC` ou `DT_NEEDED` quando o alvo é completamente estático;
- nenhum símbolo `UND`;
- nenhuma relocação residual quando o formato escolhido exige imagem fechada;
- nenhuma `.eh_frame`, `.gcc_except_table`, `.init_array` não autorizada;
- entrypoint explícito;
- símbolos públicos somente por allowlist;
- hash reprodutível e tamanho registrado;
- saída equivalente antes/depois de strip;
- ciclos p50/p95/p99 e jitter por alvo.

DEX, GPU, DSP e NPU têm gates próprios. Não recebem o selo ELF por analogia.

## 12. Estados epistêmicos

```text
DIRECT_PASS       rota direta e artefato final provado
SUBSET_PASS       subset da linguagem provado
LOWERED_PASS      fonte estrangeira baixada para kernel interno equivalente
DEVICE_PASS       kernel provado; host/driver fora do claim
DATA_ONLY         modelo preservado como dados
TOKEN_VAZIO       falta frontend, equivalência, hardware ou evidência
REJECTED_RUNTIME  saída ainda carrega VM, GC, interpretador ou dependência proibida
```

## 13. Falsificadores

A metodologia falha se:

1. uma linguagem hospedada for rotulada freestanding apenas porque gerou arquivo;
2. um interpretador, JVM, Go runtime, Swift runtime ou driver for ocultado;
3. `malloc`, GC, exceções ou símbolos indefinidos chegarem ao núcleo estrito;
4. duas funções equivalentes forem copiadas em vez de compartilhar kernel canônico;
5. um patch alterar bits fora da máscara;
6. paralelismo for declarado sem independência ou medição por ciclos;
7. o resultado divergir da referência bit a bit;
8. strip mudar a execução;
9. a granularidade física de escrita for confundida com a máscara lógica;
10. ausência de prova for convertida em sucesso.

## 14. Implementação desta etapa

```text
Apkc/lang_freestanding_policy.h
rafaelia/raf_lane16_patch.h
RAF_063_language_completion_freestanding.c
ci/contracts/rafaelia_language_completion_v1.tsv
scripts/audit_language_freestanding_contract.sh
docs/templates/RAFAELIA_LIBRARY_ASSIMILATION_MANIFEST.yaml
```

O selftest valida as 23 políticas, patch mascarado e dependências do plano de lanes. O objeto estrito é compilado sem símbolo indefinido. Isso prova o núcleo da metodologia, não a conclusão de todos os frontends.
