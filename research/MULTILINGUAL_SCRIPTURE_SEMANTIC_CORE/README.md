# Núcleo Multilíngue de Escrituras e Semântica — MSSC V1

**Estado:** `REFERENCE / EMULATION_ONLY` — `claim_allowed=false`  
**Proprietário lógico:** `semantic-research`  
**Âncora no mapa:** [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md §4 · research`](../../docs/MAPA_ESTRUTURAL_REPOSITORIO.md)  
**Integração de runtime:** inexistente.  
**Importação de corpus:** bloqueada nesta geração.

Este núcleo isolado prepara o RafPolimata para analisar unidades textuais em quatro perfis:

```text
português brasileiro (pt-BR)
↔ hebraico bíblico (hbo)
↔ aramaico bíblico (arc)
↔ grego antigo/coiné (grc)
```

A análise combina camadas explicitamente separadas:

```text
texto original
→ normalização Unicode
→ grafemas aproximados
→ tokens anotados
→ lema e morfologia
→ sintaxe e tempo/modo verbal
→ transliteração
→ fonética aproximada com perfil declarado
→ alinhamento de tradução
→ relações intertextuais tipadas
→ métricas matemáticas
→ observáveis neurocognitivos não clínicos
→ recibo determinístico
```

## Por que a pronúncia nunca é “uma só”

A transcrição fonética depende de idioma, período, dialeto, edição, tradição de leitura e região. O português brasileiro também varia: uma pronúncia generalizada não representa igualmente São Paulo, Rio de Janeiro, Minas Gerais, Nordeste, Sul ou outras comunidades. O núcleo exige `profile` e `approximate=true`; para reconstruções antigas, IPA ausente permanece `TOKEN_VAZIO`.

Isso evita o erro de tratar letras iguais como sons iguais — exatamente o tipo de diferença que faz um estrangeiro produzir uma frase gramaticalmente correta, mas foneticamente inesperada para quem ouve.

## Fronteira neurocientífica

O emulador calcula apenas observáveis do texto e das anotações:

- contagem de tokens e cláusulas;
- entropia de caracteres e lemas;
- distância de dependência;
- densidade morfológica;
- cobertura de anotação fonética;
- assinatura de classes gramaticais.

Essas medidas podem alimentar hipóteses experimentais sobre memória de trabalho, prosódia, predição e integração sintática. Elas **não** medem cérebro, emoção, compreensão, fé, espiritualidade ou intenção autoral.

## Relações entre passagens

Cada aresta do grafo recebe tipo, evidência, limitações e estado epistemológico:

```text
translation_alignment | quotation | allusion | lexical | thematic
contrast | grammatical_parallel | hypothesis
```

Uma semelhança temática não vira automaticamente citação. Um lexema semelhante não prova intenção autoral. Uma tradução não é declarada semanticamente idêntica ao texto de origem.

## Corpus e licenças

Nenhum corpus completo foi incorporado. Os adaptadores permanecem desligados até que cada edição tenha proveniência, hash e licença registrados.

Candidatos técnicos para avaliação posterior:

- Open Scriptures Hebrew Bible: texto WLC declarado em domínio público; dados de lema/morfologia sob CC BY 4.0 segundo o projeto.
- MorphGNT: morfologia e lematização sob CC BY-SA, enquanto o texto grego subjacente depende da licença/EULA da edição escolhida.
- Aramaico bíblico: `TOKEN_VAZIO_CORPUS_SELECTION`; exige edição, dialeto, versificação e licença.
- Português: `TOKEN_VAZIO_PORTUGUESE_EDITION_SELECTION`; uma glosa de engenharia não substitui uma tradução bíblica licenciada.

Referências normativas previstas para o adaptador real:

- Unicode UAX #15 — normalização;
- Unicode UAX #29 — segmentação de grafemas, palavras e sentenças;
- BCP 47 / RFC 5646 — identificação de idiomas;
- International Phonetic Alphabet — representação fonética.

A implementação atual usa somente Python stdlib. A segmentação de grafemas é uma aproximação declarada e não reivindica conformidade completa com UAX #29.

## Emulação local

```sh
cd research/MULTILINGUAL_SCRIPTURE_SEMANTIC_CORE
python3 core/emulator.py --output results/demo_receipt.v1.json
python3 -m unittest discover -s tests -p 'test_*.py'
```

O fixture contém pequenos excertos linguísticos para validar scripts, morfologia, fonética aproximada e grafo. Não é uma edição bíblica nem corpus de pesquisa.

## Critérios antes de ligar

O núcleo só poderá sair de `EMULATION_ONLY` após, no mínimo:

1. corpus por idioma com edição, licença, URI, hash e versificação;
2. analisadores morfológicos validados por idioma e dialeto;
3. perfil de transliteração e pronúncia por tradição/período;
4. alinhamento verso, palavra e morfema com incerteza explícita;
5. fontes acadêmicas para cada relação intertextual forte;
6. estudo humano separado para qualquer alegação de compreensão ou neurociência;
7. auditoria legal, teológica, linguística e computacional;
8. comando manual explícito de ativação em geração futura.

Até lá:

```text
enabled = false
execution_mode = EMULATION_ONLY
claim_allowed = false
```
