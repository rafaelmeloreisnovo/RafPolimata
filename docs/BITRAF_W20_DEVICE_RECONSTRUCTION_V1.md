# Reconstrução do novo dispositivo — ponte BITRAF8000 ↔ W20 V1

**2026-10-14 · estado:** `EXPERIMENTAL_SOURCE_BOUND / NÃO PROMOVER`. Não representa um novo chip, placa, bootloader instalado, APK nem detecção física de perda. **`claim_allowed=false`.** Esta é uma extensão reversível de modelo binário, não alteração do contrato de BITRAF Matrix 8000/4096.

## Origem e fontes mínimas

| Autoridade | Fonte | Escopo |
| --- | --- | --- |
| RafPolimata | `rafaelia/bitraf_matrix.h` e `docs/BITRAF_MATRIX_8000_4096_V1.md` | `S=(x,y,z,v,p)`, 8000 estados, core 4096, shell 3904; geometria e índices |
| Drive privado | Documento “RAFAELIA — Icosfera f=2 × Fibonacci × Palavra 20-bit × Paridade Dual — V1” | proposta `W20=A10|V8|pA|pV`, 256000 combinações, paridades sem ECC geral, K64 e lacunas; material privado não copiado |
| Drive HOTSTATE V4.5 | `MOBILE BITRAF RECONSTRUCTION` (2026-10-14) | Realme Note 50 indisponível; reconstruir a partir de fonte, não do aparelho destruído |
| Mapa | `indices/BITRAF_LOSS_VECTOR_MODEL_V1.md` | erasure, bit flip, omissão, alias, síndrome e proveniência são conceitos diferentes |
| Vectras/Termux/RafGitTools | RMR/BitOmega; `rmr/Rrr/rafaelia_b1.S..b8.S`; gates ARM32 | produtores de integração separados, nenhum prova W20 Android end-to-end |

Sem publicar IDs privados de mensagens ou de ficheiros. O documento do Drive é autoridade da hipótese histórica, não do exato código executável. Regras: `SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`, `TOKEN_VAZIO != 0`, `IMPLEMENTED_UNTESTED != PASS`.

## Ponte nova: hipótese de mapeamento sem colisões

A fonte BITRAF existente define:

```text
cell  = 100*x + 10*y + z       (0..999)
vertex = 0..3                  (2 bits)
phase  = 0..1                  (1 bit)
index8000 = ((cell*4)+vertex)*2 + phase
```

O documento W20 define uma palavra `A10|V8|pA|pV`; não define por si só os significados de todos os oito bits do payload. Esta PR introduz deliberadamente **um perfil experimental**, para preservar todas as coordenadas BITRAF e adicionar um subvalor local:

```text
subvalue5 = 0..31
V8        = (subvalue5 << 3) | (vertex << 1) | phase
pA        = parity(XOR of 10 address bits)
pV        = parity(XOR of 8 value bits)

bits 19..10 = A10 = cell
bits  9..2  = V8
bit      1  = pA
bit      0  = pV
bits 31..20 = 0 (when stored as uint32)
```

Então `1000 * 4 * 2 * 32 = 256000` registros W20 válidos. A bijeção existe para o par `(state8000, subvalue5)`; **não** equivale a uma bijeção simples `8000 ↔ 256000`. O valor `phase` da fonte BITRAF não é a paridade `pV`: `phase` é um bit de estado carregado em V8, enquanto `pV` é computado sobre V8. Não mudar ou sobrecarregar `bitraf_state` nem `bitraf_index8000`.

**Distinções estritas:**

- `MATRIX8000 = CORE4096 + SHELL3904`, com `10³*4*2` versus `8³*4*2`. `core_index4096` não é endereço de byte, nem `A10` é identidade de bit físico na RAM.
- Palavra lógica de 20 bits necessita 3 bytes se armazenada isoladamente com padding; dois registros podem ser empacotados em 5 bytes. A camada W20 implementa palavras em `unsigned int` de 32 bits com bits reservados altos zerados; o sucessor `bitraf_w20_pair40_v1.h` materializa adicionalmente o **serializador exato 2×W20 em 5 bytes**, em ordem de bits pouco-significativos primeiro, sem dependência de endianness do host. Não é um algoritmo de compressão nem armazenamento com correção de erro.
- Paridade dual detecta todo **erro isolado de um bit**, mas não detecta necessariamente dois flips no mesmo campo e **não recupera payload perdido**. `CRC`, `ECC`, `FEC`, `erasure` e `TOKEN_VAZIO` são tipos separados.
- Icosfera `f=2` tem `V=42,E=120,F=80`; igualdade entre 42 vértices e `6×7`, ou com outros índices, não fornece isomorfismo, ISA nem circuito.
- `K64=8*(4+4)` em oito colunas é outra granularidade; ligação `K64 → W20` requer esquema explícito e vetor round-trip. Não assumir compatibilidade de layout.
- `56 hyperformas`, `56% de perda`, representações em base 60, acústica/fonemas e execução por 7 direções continuam **gaps de integração**, sem claim de implementação neste header.

## Arquivos acrescentados e executabilidade

- `rafaelia/bitraf_w20_bridge_v1.h`: núcleo C puro freestanding, sem libc, malloc, I/O, syscalls, tipos independentes de plataforma, valida limites/paridades/reserved bits antes de escrever a saída.
- `tests/bitraf_w20_bridge_test.c`: `8000 × 32 = 256000` round-trips determinísticos, `256000 × 20 = 5120000` corrupções isoladas, verificações de parâmetros inválidos e preservação dos destinos em FAIL; demonstra 2-bit flip que escapa à paridade.
- `tests/bitraf_w20_bridge_freestanding.c`: prova de compilação `armv7-none-eabi` e ausência de símbolos indefinidos em objeto ARM32.
- `rafaelia/bitraf_w20_pair40_v1.h`: serializador freestanding 40-bit, dois registros validados para 5 bytes, leitura/gravação atômica da saída no escopo da chamada.
- `tests/bitraf_w20_pair40_test.c`: 128000 pares, 640000 bytes, 5120000 mutações isoladas, dois compiladores e vetor golden da primeira dupla.
- `.github/workflows/bitraf-w20-bridge.yml`: GCC/Clang, saída comparada, `readelf`, `nm -u`, hashes e exact-head PR.

**Sequência obrigatória de evidência física para futuro dispositivo ARM32:**

```text
SOURCE at exact SHA → toolchain manifest → freestanding object + ABI report
→ Android adapter scoped → APK/DEX/ELF hashes → signature verification
→ physical ARM32 install/launch and output capture → receipt
→ independent review → promotion decision (provider-enforced)
```

Nesta PR, o fechamento máximo é `SOURCE+HOSTED_TEST+ARM32_OBJECT`; os demais elos ficam `TOKEN_VAZIO`. Não instalar no Realme indisponível. O Motorola ARM32 é rota potencial *se autorizado e disponível*, não aparelho executado nesta rodada.

## Sete direções de falsificação

| Direção | Falha/contraprova | Status inicial |
| --- | --- | --- |
| Identidade | `decode(encode(state,payload)) != (state,payload)` | gate exaustivo na CI |
| Semântica | `phase` confundida com `pV`; `∅` confundido com zero | código separa; análise contextual mantém `TOKEN_VAZIO` |
| Privacidade | ID privado, body do NOVOexport ou chave OAuth em diff/receipt | nenhuma importação privada pretendida |
| Temporalidade | execução do SHA antigo apresentada como prova do HEAD atual | receipt com SHA exato obrigatório |
| Falsificação acadêmica | single flip aceito; duplo flip declarado detectável sem teste | adversariais positivo e contraexemplo |
| Governança | PR mergeada com CodeScan/revisão/proteção ausentes | `NÃO PROMOVER` e sem merge |
| ARM32 | compilar objeto chamado de teste físico/APK | físico `TOKEN_VAZIO` |

## R3

`F_ok`: ponte matemática definida e código/testes submetidos para verificação. `F_gap`: hipótese de subvalue5 não é canônica até ratificação; serialização com cabeçalho/versionamento de arquivo, ECC, FEC, performance/memória em aparelho, adaptador Android, envelope de compilação e autoridade de promoção ausentes. `F_next`: CI de PR no exato SHA; depois serialização sem padding com vetores; depois integração HOST/Android sem regressão; aparelho físico e recibos independentes por último. `rollback`: fechar a PR ou reverter apenas os novos arquivos; header e formatos existentes mantidos byte-a-byte.
