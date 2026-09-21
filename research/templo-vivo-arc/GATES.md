# Gates do Templo Vivo ARC

## Gate summary

```yaml
G1:
  name: Seed Reconstruction
  result: PASS
  evidence: "9/9 hashes-fonte coincidiram; 10.449 registros; reconstrução exata"
  scope: "corpus normalizado declarado"
  limit: "reconstruibilidade estruturale não semântica"

G2:
  name: Total-Cost Compression
  result: FAIL_STRONG
  evidence: "plain+Brotli = 317.003 B; seed+Brotli = 320.306 B; +schema+decoder = 324.771 B"
  scope: "hipótese forte de compressão superior"
  limit: "refutada no experimento executado"

G3:
  name: Null/Shuffle
  result: PASS
  evidence: "3 famílias × 32 permutações; p ≈ 0,030303"
  scope: "efeito de ordem/estrutura no corpus"
  limit: "não implica lei universal"

UDHR:
  name: Universal Declaration of Human Rights control
  result: PASS_RECONSTRUCTION
  evidence: "90 segmentos / 270 registros; reconstrução exata"
  scope: "controle externo"
  limit: "estrutura consistente, não significado universal"
```

## Conclusão controlada

- `G1` validou reconstrução.
- `G2` refutou a versão forte da hipótese de compressão.
- `G3` mostrou efeito mensurável de ordem/estrutura.
- o controle externo reforçou a consistência do padrão sem elevar a conclusão a ciência global.
- `claim_allowed` permanece `false`.

## Protocolo final

```text
reconstruibilidade != decifração
preservação de ordem != significado universal
compressão superior forte != validado
semântica != evidência executada
```
