# Tensor Relacional Contextual v1

## Limite honesto

Este componente não lê nem modifica pesos internos do GPT, atenção nativa, embeddings privados ou estados ocultos. O termo **tensor** designa uma matriz externa e auditável:

\[
X_{p,f}=\text{valor da feature }f\text{ para o caminho semântico }p
\]

com pesos de governança explícitos:

\[
S(p)=\sum_f w_fX_{p,f}-\sum_j \lambda_jP_{p,j}
\]

Os pesos são versionados, revisáveis e substituíveis por calibração posterior. Não são prova científica e não fingem reproduzir o modelo.

## Features

- correspondência lexical;
- suporte de fontes;
- proveniência;
- coerência entre camadas;
- completude de unidades;
- adequação temporal;
- contraevidência;
- pressão de lacunas.

A correspondência lexical recebe peso baixo. Fonte e proveniência recebem pesos maiores. Um caminho não se torna elegível sem suporte, proveniência, coerência de camada e ausência de pressão bloqueante.

## Fórmula Vinho

O fixture compara três caminhos:

1. oráculo lexical;
2. salto direto clima → preço;
3. clima/meteorologia → fenologia → rendimento/qualidade → mercado → preço.

O terceiro caminho é estruturalmente superior, mas o sistema ainda se abstém porque origem, literatura, dados e unidades permanecem vazios.

## Uso

```bash
python3 scripts/contextual_relational_tensor.py \
  examples/wine-formula-relational-tensor.v1.json

python3 -m unittest tests.test_contextual_relational_tensor
```

## R3

```text
F_ok   = matriz explícita, ranking determinístico e abstinência
F_gap  = calibração em corpus real e avaliação humana autorizada
F_next = executar benchmark com casos conhecidos e medir MRR, cobertura e falsa promoção
```
