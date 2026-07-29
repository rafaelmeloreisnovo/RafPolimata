# VOYNICH BATCH AUDIT — 2026-07-29

```text
EVENT_ID=VOYNICH-BATCH-AUDIT-20260729T033600-0300
status=LOCAL_MULTI_LAYER_AUDIT_COMPLETE_CLAIMS_BLOCKED
claim_allowed=false
```

## 1. Escopo recebido

Foram preservados e hasheados dez objetos:

- `voy_core.c`
- `voy.py`
- `voynich_exacordex.c`
- `voynich_toroidal.c`
- `vectra_proof_suite_armv7_v2.tar.gz`
- `sementes.zip`
- `sample_voy_groups.txt`
- `voy.zip`
- `voynich_downloader.c`
- `voynich_analysis.py`

Os três arquivos compactados passaram por inspeção de nomes, caminhos, links e expansão antes de qualquer uso. Não foram encontrados `../`, caminhos absolutos, links simbólicos, hardlinks ou dispositivos especiais.

## 2. Separação canônica de camadas

| Camada | Objeto | Estado |
|---|---|---|
| Detector visual | `voy_core.c`, `voy.zip` | compila e executa, mas falha no ground truth angular |
| Simulação toroidal | `voynich_toroidal.c` | executa, porém é aleatória e promove conclusão não demonstrada |
| Análise de imagens | `voynich_analysis.py` | executa, mas reduz os quatro modos a caminhadas lineares quase equivalentes |
| Aquisição | `voy.py`, `voynich_downloader.c` | contrato de fonte e segurança incompletos |
| Contador web | `voynich_exacordex.c` | conta substrings em página da Wikipedia, não no manuscrito |
| Corpus de grupos | `sample_voy_groups.txt` | 15/20 contagens individuais conferem; semântica não demonstrada |
| Prova vetorial | `vectra_proof_suite_armv7_v2.tar.gz` | prova estrutural forte; execução física V2 pendente |
| Memória histórica | `sementes.zip` | integridade interna forte; scripts não executados |

## 3. `voy_core.c` — regressão angular reproduzida

O arquivo recebido não contém as implementações `_write`, `_read`, `_open`, `_close` e `_exit`. Para testar somente o núcleo C, foi usado um shim de auditoria x86_64 separado, sem alterar a fonte.

Resultado reproduzido:

```text
expected_angles=20,55,90,130
reported_angles=45,45,45,45
confidence=0,0,0,0
chi2x100=0
crc32c=ABDEF93D
```

O programa detecta quatro janelas, mas não recupera a orientação dos traços. O teste é, portanto, um teste de detecção de regiões sintéticas, não um teste angular válido.

Outro bloqueio: o texto `RETOQUES DIRECIONAIS — hipotese confirmada` promove uma estatística para uma interpretação histórica. Mesmo uma distribuição angular não uniforme não demonstra referência cruzada ou hipertexto medieval.

```text
status=BUILD_PASS_END_TO_END_ANGLE_FAIL
claim_allowed=false
```

## 4. `voynich_toroidal.c` — simulação apresentada como conclusão

A fonte compilou com cinco warnings e executou. Porém:

1. `srand(time(NULL))` torna a saída não determinística;
2. o toro não contém 240 páginas fixas — cada célula recebe conteúdo por Bernoulli aproximado;
3. `char rgb[3]` produz valores negativos em plataformas com `char` assinado;
4. as ocorrências textuais são números fixos simulados;
5. a conclusão sobre a natureza do manuscrito é impressa independentemente dos resultados.

Campanha com seeds 1..8:

```text
content_count=242,241,228,227,251,249,232,231
negative_rgb_cells=209,206,201,203,221,217,201,196
```

Apesar disso, o programa imprime `240 páginas reais`. O fato e a mensagem divergem.

## 5. `voynich_analysis.py` — quatro nomes, duas ordens equivalentes

O script foi executado sobre oito imagens sintéticas determinísticas.

Resultado:

```text
RAW  order=[0,1,2,3,4,5,6,7]
JPEG order=[1,2,3,4,5,6,7,0]
GIF  order=[1,2,3,4,5,6,7,0]
EXEC order=[3,4,5,6,7,0,1,2]
```

Causa: `toroidal_navigation()` usa somente `dx`; quando `dx==0`, força `stride=1`. Assim:

- `JPEG` e `GIF` são exatamente a mesma navegação;
- `EXEC` é apenas rotação da mesma caminhada linear;
- `dy` e `dz` não participam;
- nenhuma estrutura 7D é calculada.

A conclusão de que o manuscrito “não é um código cifrado” é impressa sem teste comparativo, falsificador ou ground truth.

## 6. Aquisição de dados

### `voy.py`

A fonte usa Internet Archive, mas:

- não define timeout;
- não congela metadata, licença ou hash do objeto remoto;
- o fallback inventa 240 nomes por padrão, sem provar que existam;
- os nomes baixados podem não corresponder ao padrão `voynich_*.jpg` esperado pelo analisador;
- `requests` e `tqdm` contradizem o objetivo de zero dependências.

### `voynich_downloader.c`

Bloqueios estáticos:

- entrada ZIP é concatenada diretamente ao diretório de saída: vulnerável a path traversal;
- `malloc(sb.size)` não possui teto;
- retorno de `zip_fread()` não é validado;
- HTTP 4xx/5xx pode ser salvo como sucesso porque `CURLOPT_FAILONERROR` não é configurado;
- arquivo parcial não recebe hash ou manifest;
- o ZIP Zenodo referenciado é um pacote de corpus/análise de ~280 kB, não um conjunto de imagens de alta resolução;
- a extração desse dataset dentro de `images/` quebra o contrato da aplicação;
- o processamento de imagem é aleatório e pode dividir por zero quando RGB=(0,0,0).

A compilação completa desse downloader permaneceu `TOKEN_VAZIO_LIBZIP_UNAVAILABLE` neste ambiente.

## 7. `voynich_exacordex.c`

A fonte compila com libcurl e libxml2. Ela baixa a página da Wikipedia sobre o manuscrito e conta strings numéricas `123`, `0123`, `01123`, `0001123` e `42` no HTML convertido em texto.

Isso mede a página web naquele momento, não o corpus Voynich. O resultado não pode sustentar uma propriedade do manuscrito.

## 8. Auditoria de `sample_voy_groups.txt`

A comparação foi feita contra o corpus local `voynich_eva_words.txt` anteriormente congelado.

```text
individual_counts_matching=15/20
```

Divergências principais:

| Token | Declarado | Observado |
|---|---:|---:|
| `chotey` | 134 | 9 |
| `darol` | 89 | 3 |
| `dshedy` | 47 | 36 |
| `shol` | 198 | 187 |
| `shody` | 156 | 55 |

`qokeedy` também aparece duplicado na tabela.

Totais de prefixo:

| Prefixo | Declarado | Observado |
|---|---:|---:|
| `qo` | 5228 | 5258 |
| `ch` | 5974 | 5983 |
| `da` | 2328 | 2339 |
| `sh` | 3221 | 3238 |

As etiquetas “marcador de referência”, “poção/pigmento”, “artigo”, “versículo” e raízes árabes permanecem `HIPÓTESE` ou `MODELO_ANALÓGICO`; frequência não prova semântica.

## 9. Vectra Proof Suite ARMv7 V2

Este é o componente tecnicamente mais consistente do lote.

### Integridade

```text
archive_sha256=e4f012b1bf9b83117bc4f4ff3d716785bd178480b51a51297a8ec494001a8df3
manifest_entries=16
manifest_hashes=16/16 PASS
```

### ELF fornecido

```text
format=ELF32 ARM EABI5
linkage=static
program_headers=2 LOAD
NEEDED=0
INTERP=0
dynamic_section=absent
sha256=95f2d5ff249e14a05baf729aed5b78848e1c92e09ea2073300866cf5fcc61345
```

### Recompilação cruzada

A fonte foi recompilada com Clang/LLD usando os flags e linker script do pacote.

```text
rebuild_sha256=95f2d5ff249e14a05baf729aed5b78848e1c92e09ea2073300866cf5fcc61345
byte_identical=true
```

Isso prova reprodutibilidade estrutural neste ambiente. Não prova execução NEON física, tempo, percentis ou equivalência em aparelho ARMv7.

```text
physical_run=TOKEN_VAZIO_NOT_EXECUTED_X86_64_HOST
```

## 10. `sementes.zip`

```text
archive_sha256=6bbbb8988b9f547820d32d0eb876b205c80c8af060cfe31d4eb60819b49e3342
entries=106
unsafe_entries=0
source_hashes=14/14 PASS
generated_hashes=59/59 PASS
scripts_executed=false
```

O pacote apresenta boa cadeia de hashes. Seus scripts foram preservados sem execução automática porque o arquivo contém geradores, binários e comandos que exigem auditoria individual.

Uma lacuna documental: os manifests usam caminhos históricos/absolutos e não podem ser passados diretamente a `sha256sum -c` após extração em outro diretório. Os hashes são válidos, mas o manifest não é relocável.

## 11. Correções produzidas

### `voynich_toroidal_v2.c`

- seed explícita;
- exatamente 240 células de conteúdo;
- RGB `uint8_t`;
- zeros preservados como pausa;
- movimento declarado como convenção cíclica x/y/z;
- saída determinística com FNV-1a;
- `status=SIMULATION_ONLY`;
- nenhuma conclusão linguística/histórica.

Build `-Werror` e duas execuções idênticas: PASS.

### `voynich_analysis_v2.py`

- recebe diretório/padrão/steps por CLI;
- hashes SHA-256 de todas as imagens;
- zeros são pausas, não “hiperformas”;
- relatório JSON;
- limitações explícitas;
- nenhum claim de decifração ou 7D.

### `voynich_downloader_v2.py`

- somente biblioteca padrão;
- metadata primeiro;
- `--max-files 0` faz apenas inventário;
- timeout e limites de tamanho;
- saneamento de nomes;
- manifest com hash da metadata e hashes dos downloads;
- sem fallback inventado.

A rede não foi executada nesta auditoria.

## 12. Fechamento R3

```text
F_ok:
  10 inputs hasheados
  3 archives estruturalmente seguros
  Vectra 16/16 hashes + cross-build byte-idêntico
  sementes 14/14 + 59/59 hashes
  correções locais compiladas/testadas

F_gap:
  detector angular end-to-end
  corpus oficial de imagens com licença e hashes
  execução física Vectra ARMv7 V2
  semântica cega e reprodução independente
  libzip indisponível para build do downloader original

F_next:
  congelar um conjunto oficial de imagens
  criar ground truth cego dos glifos/ângulos
  medir precisão, recall e erro angular
  executar Vectra V2 no aparelho ARMv7
```

**Parábola:** a bússola que gira sobre números aleatórios pode desenhar círculos perfeitos; somente o marco fixo prova que ela aponta para algum lugar.
