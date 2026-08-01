# RafPolimata — Living Book Domain IR V1

Este adaptador compila uma célula do Livro Vivo em uma **IR não executável**.

```text
célula validada
→ seleção de módulo
→ ação permitida
→ política e capabilities
→ IR canônica com digest triplo
```

Comando:

```bash
python3 tools/living_book_domain_ir/compile_domain_cell_ir.py \
  --cell /caminho/music.v1.json \
  --module support.math \
  --action PROPOSE_ANALYSIS \
  --intent-id INT-MUSIC-0001 \
  --out COMPILA/living-book/INT-MUSIC-0001.ir.json
```

A V1 aceita apenas:

```text
INDEX_ONLY
PROPOSE_ANALYSIS
PROPOSE_TRANSLATION
PROPOSE_TEST
```

Ela rejeita execução, rede, publicação, merge, exclusão, divulgação privada, `shell_eval` e promoção de claim.

A IR não carrega o resumo da semente. Carrega apenas IDs, digests, módulo, ação, capabilities, gates, idioma de retorno e contrato esperado de receipt.

```text
claim_allowed=false
execution_allowed=false
publication_allowed=false
```

Este módulo não substitui o produtor `LivroVivo_ThisBookLives`, o controle do `Mapa`, o transporte do `RafGitTools` nem a execução Termux.
