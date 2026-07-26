# Custody QR Profile V1

Estado: `IMPLEMENTED` estrutural + `VERIFIED_LIMITED_LOCAL`.  
Claim boundary: `claim_allowed=false`.

## Propósito

O QR é um **selo portátil de checkpoint** da cadeia:

```text
fonte/artefato
→ normalização
→ commits e manifests
→ estado dos gates
→ envelope compacto
→ QR
→ verificação
```

Ele não transporta banco, registro empresarial, nome de pessoa, CEP, CPF,
credencial, segredo ou chave privada.

## Conteúdo do QR

O perfil `RAFCUSTODY-QR/1` contém:

- `snapshot_id`;
- `epoch_id`;
- escopo;
- estado `claim_allowed`;
- gates local, remoto e produção;
- commits-base dos quatro PRs;
- declaração de privacidade;
- SHA-256 e CRC32 do corpo;
- autenticação opcional por HMAC-SHA256.

O QR inicial representa um checkpoint do encadeamento:

```text
Mapa #65
papers #24
RafPolimata #166
RafGitTools #309
```

Os commits dentro do QR são **baseline_heads**: registram a cadeia antes da
própria adição do perfil QR, evitando autorreferência circular.

## Artefatos

```text
tools/custody_qr_envelope.py
tests/test_custody_qr_envelope.py
docs/assets/forensic-custody-qr-v1.svg
docs/assets/forensic-custody-qr-v1.txt
docs/assets/forensic-custody-qr-v1.json
```

## Geração

Sem dependências externas, o envelope e o payload textual são gerados assim:

```bash
python3 tools/custody_qr_envelope.py build \
  --output-envelope build/custody-qr/envelope.json \
  --output-payload build/custody-qr/payload.txt
```

A renderização SVG é opcional e requer o pacote Python `qrcode`:

```bash
python3 tools/custody_qr_envelope.py build \
  --output-envelope build/custody-qr/envelope.json \
  --output-payload build/custody-qr/payload.txt \
  --output-svg build/custody-qr/custody.svg
```

Verificação:

```bash
python3 tools/custody_qr_envelope.py verify \
  --payload build/custody-qr/payload.txt
```

## Limites criptográficos

- CRC32 é verificação rápida de erro, não autenticação.
- SHA-256 identifica o corpo, mas qualquer pessoa pode recalculá-lo.
- HMAC autentica internamente quando a chave é fornecida fora do QR.
- HMAC não oferece verificação pública independente.
- Assinatura Ed25519 e timestamp independente permanecem `TOKEN_VAZIO`.
- O QR é um portador; a prova continua nos manifests, commits, recibos e logs.

## Recibo local

O conteúdo final foi compilado e testado em sandbox:

```text
py_compile: PASS
custody QR tests: 4 PASS / 0 FAIL
payload roundtrip: PASS
payload length: 685 characters
PII/secret-field guard: PASS
claim_allowed=false: PASS
```

Checkpoint gerado:

```text
snapshot_id = FDV1-20260726-QR01
sha256 = 2d6f4b2ce47939b0ffb78904c6acd1fcd190c1c2b0eab7bb6943e5852db44f1d
crc32 = 690a3641
auth_kind = NONE
```

## Falsificadores

O perfil falha se:

- aceitar alteração sem detectar mismatch;
- incluir PII, segredo ou dado bruto;
- permitir `claim_allowed=true`;
- tratar CRC/SHA como assinatura;
- usar o QR como única prova;
- quebrar a decodificação após leitura fiel do símbolo.

---

`F_ok`: checkpoint compacto, escaneável e verificável.  
`F_gap`: assinatura pública e timestamp independente ausentes.  
`F_next`: Ed25519 + registro de transparência + verificação no RafGitTools.
