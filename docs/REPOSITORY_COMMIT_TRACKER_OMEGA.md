# Repository Commit Tracker Ω

Ju> **Entrada canônica:** docs/AGENTES.md §8 (entradas canônicas por subsistema) e §3 (estados canônicos — ciclo de sessão e rastreabilidade). Rastreador Ω de commits e forks em somente leitura — anel inicial de 16 repositórios, cadeia SHA-256 de shards e cadência de 15 minutos.

> **Estado:** `AUDIT / CANDIDATE`  
> **Modo:** metadados, commits e linhagem de forks em somente leitura  
> **Cadência configurada:** 15 minutos  
> **Entrada:** `configs/repository-tracker.v1.json`  
> **Execução:** `scripts/repo_commit_tracker.py`  
> **CI:** `.github/workflows/repository-commit-tracker.yml`  
> **Claims:** `claim_allowed=false`

## 1. Finalidade

O tracker observa um conjunto explícito e limitado de repositórios, registra alterações de commits e cabeças de forks e produz uma cadeia de shards verificável.

Ele não é um agente que modifica projetos externos. Não realiza:

- fork automático;
- merge automático;
- push automático;
- checkout de forks;
- execução de código externo;
- instalação de software observado;
- inferência de intenção dos autores.

A célula mínima é:

\[
\mathcal C_n=
\langle
id_n,
sha_{commit},
sha_{payload},
sha_{cadeia},
origem,
semântica,
tempo
\rangle
\]

## 2. Anéis iniciais

O primeiro anel contém 16 repositórios, ordenados por prioridade:

1. `RafPolimata`;
2. `Mapa`;
3. `RafGitTools`;
4. Termux App;
5. Termux API;
6. `PCR_Rafaelia_Code_seed`;
7. `UserLAnd`;
8. `UserLAnd2`;
9. `Vectras-VM-Android`;
10. `qemu_rafaelia`;
11. `BLAKE3`;
12. RLL;
13. corpus privado de chunks;
14. kernel Linux;
15. Android ROM;
16. Android `frameworks/base`.

O nome falado `PCE_` não apareceu literalmente no inventário acessível. Ele foi registrado somente como alias provisório de `PCR_Rafaelia_Code_seed`:

```text
PCE_ → PCR_Rafaelia_Code_seed
estado = TOKEN_VAZIO_ALIAS_RESOLUTION
```

Isso permite localizar o projeto sem afirmar que as duas grafias são definitivamente equivalentes.

## 3. Ciclo de quinze minutos

```text
RESTORE STATE
      ↓
VALIDATE CONFIG + TESTS
      ↓
READ REPOSITORY METADATA
      ↓
READ LATEST COMMITS
      ↓
READ BOUNDED FORK HEADS
      ↓
COMPARE WITH PREVIOUS BASELINE
      ↓
ALLOCATE SHARDS
      ↓
UPDATE HASH CHAIN
      ↓
EMIT MANIFEST + ZIP
      ↓
SAVE CACHE + UPLOAD ARTIFACT
```

A agenda utiliza:

```yaml
cron: "7,22,37,52 * * * *"
```

Os minutos deslocados reduzem concorrência no início exato da hora. O GitHub pode atrasar execuções agendadas; a cadência é uma intenção operacional, não garantia de relógio rígido.

O workflow agendado somente começa depois que o arquivo estiver na branch padrão. Enquanto o PR permanece draft, existe implementação, mas não existe execução periódica ativa.

## 4. Contador de shard

O alfabeto é:

```text
0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~
```

Ele contém:

- números;
- letras maiúsculas;
- letras minúsculas;
- símbolos ASCII visíveis seguros para URL e nome de arquivo.

Foram excluídos `/`, `\\`, espaços, aspas e caracteres de shell perigosos.

O incremento é posicional, da direita para a esquerda, com carry:

```text
00000000
00000001
...
00000009
0000000A
...
0000000~
00000010
```

A largura inicial é oito. O contador não cresce silenciosamente além da largura: overflow é falha explícita e exige migração versionada.

## 5. Cadeia de custódia

Para cada evento:

\[
p_n=SHA256(JSON_{canônico}(evento_n))
\]

\[
h_n=SHA256(h_{n-1}\parallel id_n\parallel p_n)
\]

O shard registra:

```json
{
  "shard_id": "00000001",
  "previous_chain_hash": "...",
  "payload_sha256": "...",
  "chain_hash": "...",
  "event": {}
}
```

Alterar mensagem, commit, origem ou classificação semântica modifica o payload e rompe a cadeia.

## 6. Baseline e delta

Na primeira leitura, os commits retornados pela API são usados somente para formar a linha de base:

```text
primeira observação ≠ commit novo
```

Somente um SHA ausente da janela anterior pode produzir evento posterior.

Essa regra evita transformar os doze commits históricos retornados pela primeira consulta em doze descobertas recentes.

## 7. Linhagem de forks

Para repositórios habilitados, o tracker lê até quatro forks recentes e consulta somente o commit da cabeça de cada um.

Ele preserva:

- repositório-fonte;
- nome do fork;
- branch padrão;
- SHA da cabeça;
- mudança desde o ciclo anterior.

Ele não calcula automaticamente autoria, cópia, derivação intelectual ou intenção.

\[
\text{fork head semelhante}
\not\Rightarrow
\text{causalidade ou apropriação}
\]

## 8. Índice semântico determinístico

A primeira linha da mensagem de commit recebe etiquetas por regras transparentes:

```text
build, ci, test, docs, security, performance,
android, virtualization, kernel, crypto,
governance, fix, feature
```

Depois é gerada uma impressão digital:

\[
f=SHA256(tokens_{normalizados}+tags)
\]

Isso cria um índice evolutivo reproduzível. Não é compreensão humana, inteligência treinada nem prova de significado profundo.

## 9. Estado longitudinal

O cache guarda:

- último SHA observado;
- janela de SHAs recentes;
- cabeças de forks;
- próximo shard;
- cabeça da cadeia;
- histórico semântico limitado a 8.192 eventos;
- contagem de ciclos estáveis.

O limite impede crescimento indefinido. O histórico é uma janela operacional, não um corpus total.

## 10. Equilíbrio e snapshot

Quando quatro execuções consecutivas não encontram eventos novos:

```text
snapshot_ready = true
```

Isso representa estabilidade observada na janela, não equilíbrio matemático universal.

Cada execução produz artifact com retenção de 14 dias. Um snapshot estável recebe retenção de 30 dias.

O ZIP interno é determinístico:

- caminhos ordenados;
- timestamp fixo;
- permissões fixas;
- compressão Deflate nível 9.

## 11. Autenticação

O token padrão de Actions normalmente alcança apenas o repositório do workflow.

Para observar os repositórios privados selecionados, deve existir um secret com token de leitura de escopo mínimo:

```text
RAFAELIA_GITHUB_READ_TOKEN
```

Permissões recomendadas:

```text
Contents: read-only
Metadata: read-only
Pull requests: read-only, somente se futuramente necessário
```

Sem esse secret, o workflow usa `github.token`. Repositórios inacessíveis ficam:

```text
TOKEN_VAZIO_AUTH_SCOPE_OR_NOT_FOUND
```

Nenhum token é escrito no manifesto, cache sanitizado ou artifact.

## 12. Limites operacionais

```text
repositórios por ciclo = 16
commits por repositório = 12
forks por repositório   = 4
requisições máximas     = 180
timeout do job          = 12 minutos
```

A lista explícita funciona como allowlist. A CI não percorre infinitamente todos os forks do GitHub.

## 13. Execução local

Validação sem rede:

```sh
python3 scripts/repo_commit_tracker.py --validate-only
python3 -m unittest tests.test_repo_commit_tracker
```

Execução com token de leitura:

```sh
export GITHUB_API_TOKEN='token-read-only'
python3 scripts/repo_commit_tracker.py \
  --state-dir .tracker-state \
  --output-dir build/repository-tracker
```

## 14. Fato, lacuna e próximo gate

### Fato

- existe configuração de 16 repositórios;
- existe motor de consulta bounded;
- existe contador ASCII seguro;
- existe cadeia SHA-256;
- existem nove testes determinísticos;
- existe workflow de quinze minutos.

### Lacuna

```text
execução no default branch       = TOKEN_VAZIO_UNTIL_MERGE
secret multirrepositório         = TOKEN_VAZIO_SECRET_SCOPE
primeiro artifact remoto         = TOKEN_VAZIO_CI_RUN
FreeBSD no conjunto observado    = TOKEN_VAZIO_REPOSITORY_MAPPING
PCE_ como nome exato             = TOKEN_VAZIO_ALIAS_RESOLUTION
```

### Próximo gate

1. revisão humana do PR;
2. merge consciente na branch padrão;
3. criar secret read-only com escopo limitado;
4. executar `workflow_dispatch` uma vez;
5. conferir manifesto e consumo da API;
6. somente então manter a agenda periódica.

\[
\boxed{
TOKEN\_VAZIO
\rightarrow
baseline
\rightarrow
delta
\rightarrow
shard
\rightarrow
cadeia
\rightarrow
snapshot
}
\]
