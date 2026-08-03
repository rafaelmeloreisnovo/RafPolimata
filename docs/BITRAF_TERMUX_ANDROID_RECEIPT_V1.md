# BITRAF Matrix 8000/4096 — execução física no Termux Android V1

**Estado:** `READY_FOR_PHYSICAL_EXECUTION`  
**Origem congelada:** PR #194 / merge `fd2b9925c6cbde730741117ec3c6dd979db9b25e`  
**Branch de receipt:** `feature/bitraf-termux-receipt-v1-20260803`  
**Regra:** não alterar `main`, não promover ausência de log a `PASS`, `claim_allowed=false`.

## 1. Escolher sem escolher

O processo não seleciona manualmente um resultado favorável. Ele fixa as réguas antes da execução:

1. repositório e branch esperados;
2. commit de origem da Matriz BITRAF;
3. seis hashes SHA-256 congelados;
4. testes exaustivos já definidos;
5. saída determinística executada duas vezes;
6. receipt JSON com ambiente, comandos, resultados e hashes;
7. commit e push somente para a branch de prova.

Se qualquer invariante falhar, o script termina com `BITRAF_TERMUX_RECEIPT=FAIL` e não produz alegação de sucesso.

## 2. Bloco único para o celular

Cole o bloco completo no Termux:

```bash
set -Eeuo pipefail

REPO_DIR="$HOME/RafPolimata-bitraf-termux"
BRANCH="feature/bitraf-termux-receipt-v1-20260803"

pkg update -y
pkg install -y git clang python coreutils

if [ ! -d "$REPO_DIR/.git" ]; then
  git clone --branch "$BRANCH" --single-branch \
    https://github.com/rafaelmeloreisnovo/RafPolimata.git "$REPO_DIR"
else
  cd "$REPO_DIR"
  [ -z "$(git status --porcelain)" ] || {
    echo "ABORT: árvore local possui alterações; nada foi apagado."
    exit 1
  }
  git fetch origin "$BRANCH"
  git switch "$BRANCH"
  git pull --ff-only origin "$BRANCH"
fi

cd "$REPO_DIR"
bash scripts/run_bitraf_termux_receipt.sh
```

O script compila o teste C de forma nativa com o Clang disponível no Android. O cabeçalho BITRAF permanece freestanding-friendly; o executável de teste utiliza `stdio` apenas como casca de verificação e emissão do receipt.

## 3. Saída final esperada

Somente após teste, commit e push bem-sucedidos:

```text
BITRAF_TERMUX_RECEIPT=PASS
SOURCE_REVISION=fd2b9925c6cbde730741117ec3c6dd979db9b25e
RECEIPT_COMMIT=<commit criado no celular>
RECEIPT_PATH=auditoria/receipts/bitraf_termux/<UTC>_<ARQUITETURA>/receipt.json
BRANCH=feature/bitraf-termux-receipt-v1-20260803
CLAIM_ALLOWED=false
```

## 4. Artefatos gerados pelo celular

O receipt é preservado em:

```text
auditoria/receipts/bitraf_termux/<UTC>_<ARQUITETURA>/
├── receipt.json
├── environment.txt
├── SHA256SUMS
├── python_tests.log
├── bitraf.stdout
├── bitraf.stdout.repeat
├── manifest_command.log
├── build/bitraf-termux-clang
└── generated_manifest/
    ├── manifest.json
    └── states_core.csv
```

O binário é incluído deliberadamente como evidência do artefato executado. Ele não é promovido a release nem tratado como binário portátil entre arquiteturas.

## 5. Autenticação do push

O script não grava token. O `git push` utiliza a autenticação GitHub já configurada no Termux. Se o GitHub solicitar credencial por HTTPS, deve ser usado um método de autenticação válido da conta; senha comum não deve ser colocada no repositório, no script ou no receipt.

Falha de autenticação não invalida os logs locais, mas o estado remoto permanece:

```text
TOKEN_VAZIO_REMOTE_PUSH
```

até o receipt ser efetivamente enviado e o commit remoto ser conferido.

## 6. Critérios epistemológicos

### F_ok após execução completa

- hashes congelados correspondem aos bytes mesclados;
- Python termina em `OK`;
- C nativo termina em `BITRAF_MATRIX_V1=PASS`;
- `8000/8000`, `4096/4096` e `3904` permanecem válidos;
- duas execuções nativas produzem stdout idêntico;
- receipt é commitado e enviado à branch correta.

### F_gap antes da execução no aparelho

- arquitetura física ainda não registrada;
- versão Android/SDK ainda não registrada;
- hash do binário Android ainda não existe;
- commit remoto do receipt ainda não existe.

### F_next

Executar o bloco único no aparelho, revisar o receipt remoto e somente depois decidir entre:

1. reprodução em um segundo Android independente; ou
2. adaptador não destrutivo BITRAF → AllStar Matrix em branch separada.

## 7. Parábola da ponte marcada

O mestre não escolheu qual viajante chegaria ao outro lado.

Ele marcou cada tábua, mediu cada corda e escreveu o peso suportado. Depois permitiu que o viajante atravessasse.

Se a ponte resistisse, haveria receipt. Se uma tábua cedesse, haveria um ponto exato de correção.

> Escolher sem escolher é fixar a ponte antes de conhecer o resultado da travessia.

**FIAT LUX · Ω = Amor**
