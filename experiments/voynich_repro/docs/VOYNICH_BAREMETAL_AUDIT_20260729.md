# Auditoria Ω — Voynich bare-metal C/ASM

## Veredito

```text
status=BUILD_AND_INTEGRITY_PASS_END_TO_END_ANGULAR_VALIDATION_FAIL
claim_allowed=false
source_modified=false
```

O pacote contém **dois núcleos diferentes**:

1. `voynich_angular.c`: detector heurístico de glifos em PGM e estimador angular;
2. `voy_core.c`: tokenizador/famílias/transições de texto em modo freestanding.

Ambos são tecnicamente materializáveis sem libc ou heap. Entretanto, somente o núcleo textual corrigido fechou seu caminho sintético. O extrator angular ainda não recupera de ponta a ponta todos os ângulos inseridos.

## Fatos reproduzidos

- O binário enviado `voy_angular` é ELF x86_64 estático, sem seção dinâmica e sem símbolos externos indefinidos.
- SHA-256: `c6ad7c8708e7fd87d9854dcdb9d51ebe8d2f3544ae2cf7adbaea8d2eec21b426`.
- A saída `--test` do binário é idêntica à recompilação da fonte com GCC e Clang.
- O teste original detecta quatro candidatos, todos com `45°` e confiança `0%`.
- CRC32c original: `0xABDEF93D`.
- A suíte de auditoria fechou `14/14 PASS`.

## Falhas bloqueantes da versão recebida

### 1. ARM64 declarado, mas não compilável

O código angular contém inline assembly e constraints exclusivos de x86_64. O comando ARM64 do README falha com `invalid output constraint '=a'`.

### 2. Entry point viola a ABI de processo

`-Wl,--entry=voy_main` não transforma a pilha inicial do kernel em argumentos C. A execução direta compilada dessa forma terminou em `SIGSEGV`. É necessário `_start` por arquitetura.

### 3. Guia 512×512 contradiz o parser 256×256

O README recomenda `-resize 512x512`, mas `IW=IH=256`; o arquivo 512×512 é rejeitado.

### 4. O estimador angular original colapsa

`atan2i()` mistura graus e Q12, omite a escala correta de `180/π` e recebe `dy` e `dx` em unidades diferentes. No teste, entradas 20/55/90/130 viram 45/45/45/45.

### 5. O sintético não desenha os ângulos declarados

`synth()` usa `a % 90`, perdendo quadrante. O caso 90° vira resto zero; 130° vira 40°. Logo, o teste não é ground truth angular.

### 6. Chi-quadrado incorreto para amostras pequenas

`e=n/8` usa divisão inteira. Com `n<8`, `e=0` e a função retorna zero. O limiar publicado `1474` também não corresponde ao crítico usual de df=7 e p=0,05, aproximadamente `1407` após multiplicar por 100. Para a aproximação assintótica, o gate conservador adotado exige `n≥40`.

### 7. Direcionalidade não prova hipertexto

Mesmo uma rejeição válida de uniformidade provaria somente não uniformidade sob o modelo e amostra. Não confirma referência cruzada, intenção do escriba ou hipertexto medieval.

### 8. `openat` x86_64 perde o modo

No ABI de syscall x86_64, o quarto argumento precisa estar em `r10`. O wrapper recebido deixa o modo em `rcx`, produzindo permissões aleatórias. A execução observada criou arquivos com modos inválidos. A correção move `rcx → r10`.

### 9. Identidade de tokens não era exata

O `voy_core.c` identificava tokens por FNV-1a 32 + tamanho + primeiro/segundo/último byte. Colisões poderiam fundir tokens distintos. A revisão usa o hash como prefilter e compara os bytes originais.

### 10. Overflow contaminava o índice zero

Quando `MAX_TOK` ou `MAX_FAM` era excedido, as funções retornavam zero, podendo alterar `TOK[0]`/família zero. A revisão retorna `INVALID_INDEX` e preserva a lacuna.

### 11. `TOKENS_TOP` não estava ordenado

A seção listava os primeiros 256 tokens em ordem de inserção, não os mais frequentes. Foi renomeada para `TOKENS_FIRST_256`.

### 12. Falso truncamento em exatamente 1 MiB

A versão recebida marcava todo arquivo de exatamente 1 MiB como truncado. A revisão lê um byte adicional: 1 MiB exato gera zero lacunas; 1 MiB + 1 byte gera `0x1A11`.

## Correções materializadas

- syscalls externos por arquitetura;
- `_start` x86_64 e AArch64;
- `openat` x86_64 com `r10`;
- stack não executável;
- CORDIC inteiro para o kernel angular;
- chi-quadrado escalado sem divisão inteira do esperado;
- gate `TOKEN_VAZIO_SAMPLE_TOO_SMALL`;
- `claim_allowed=false` embutido no relatório;
- comparação exata dos bytes dos tokens;
- sentinela de overflow;
- distinção de 1 MiB exato e truncamento;
- arquivos de saída `0644`;
- cross-build AArch64 sem símbolos indefinidos.

## Resultado da variante corrigida

Kernel angular isolado:

```text
vetores esperados aproximadamente: 20, 55, 90, 130
resultado:                       21, 54, 90, 130
```

Detector completo sobre os glifos sintéticos:

```text
resultado: 45, 50, 90, 90
```

Portanto:

```text
angle_kernel=PASS_SYNTHETIC_VECTOR
end_to_end_detector=FAIL_SYNTHETIC_ANGLE_RECOVERY
```

A variante não deve ser promovida como extrator angular validado.

## Núcleo textual corrigido

Com `sample_voy_groups.txt`:

```text
input_bytes=160
tokens_unicos=19
familias=18
lacunas=0
crc32=0x9A5906F3
output_mode=0644
deterministic_rerun=PASS
```

A compilação AArch64 passou, mas não houve execução física AArch64 nesta sessão.

## Fechamento

- **F_ok:** build estático, custódia, identidade binária, correções ABI, núcleo textual e 14 testes.
- **F_gap:** detector angular end-to-end, imagens reais autorizadas, ground truth, falsos positivos, AArch64 físico e mecanismo semântico.
- **F_next:** corpus anotado cego de glifos `q`, MAE angular pré-registrado, precisão/recall, controles rotacionados e receipt Termux AArch64.

> A bússola agora mede o próprio erro; ainda não declara para onde o escriba apontava.
