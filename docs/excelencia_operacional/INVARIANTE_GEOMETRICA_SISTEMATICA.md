# Invariante Geométrica Sistemática

## 1. Definição

Todo objeto processado é representado pelo vetor:

\[
K=(I,O,C,P,E,D,T,R,N)
\]

onde:

- `I`: identidade por conteúdo;
- `O`: origem e cadeia de proveniência;
- `C`: contexto e domínio;
- `P`: política de privacidade;
- `E`: estado epistêmico;
- `D`: dependências;
- `T`: transformação aplicada;
- `R`: resultado e evidência;
- `N`: próximo passo verificável.

A transformação é válida quando preserva a projeção mínima necessária:

\[
\Pi_{min}(K_{entrada}) \subseteq K_{saida}
\]

Isto não exige copiar todos os bytes para todos os níveis. Exige conservar os campos capazes de reconstruir identidade, origem, decisão e limite.

## 2. Geometria operacional

A arquitetura usa a mesma forma em escalas diferentes:

```text
byte -> registro -> segmento -> arquivo -> corpus -> claim -> produto
```

Em cada escala:

```text
origem -> observação -> transformação -> evidência -> retorno
```

A recorrência dessa relação é a invariante geométrica. Ela é estrutural e auditável; não afirma uma nova lei física.

## 3. Critério de coerência

Defina:

\[
C_o = I_d \times P_r \times B_d \times E_v \times R_v
\]

com fatores normalizados entre 0 e 1:

- `I_d`: identidade preservada;
- `P_r`: proveniência resolvida;
- `B_d`: limites declarados;
- `E_v`: evidência vinculada;
- `R_v`: reversibilidade.

Se qualquer fator obrigatório for zero, o artefato não pode ser promovido a `VERIFIED`.

## 4. Eficiência sem perda de coerência

A otimização deve seguir:

```text
streaming > carga integral
chunks limitados > buffers ilimitados
content-addressing > identidade por caminho
checkpoint > reinício total
índice > varredura repetida
cache validado > cache opaco
abstinência > conclusão inventada
```

## 5. Estados de transformação

| Estado | Significado |
|---|---|
| `RAW` | original preservado |
| `PARSED` | estrutura sintática reconhecida |
| `NORMALIZED` | representação canônica emitida |
| `INDEXED` | consultas limitadas disponíveis |
| `VERIFIED` | critérios e evidências atendidos |
| `QUARANTINED` | risco, corrupção ou política impedem avanço |
| `TOKEN_VAZIO` | insuficiência de evidência |

## 6. Anti-invariantes

Bloquear:

- modificar o original silenciosamente;
- misturar identidade com localização;
- corrigir datas sem registrar conflito;
- tratar nome semelhante como mesma entidade;
- declarar integridade com constante fixa;
- usar throughput sintético como utilidade comprovada;
- promover execução local para validade universal;
- remover `TOKEN_VAZIO` sem nova evidência.

## 7. Fórmula de fechamento

\[
F = requisitos \land testes \land evidencias \land limites \land rollback
\]

Sem todos os termos aplicáveis, o estado máximo permitido é `PASS_LIMITED`.
