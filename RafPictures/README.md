# RafPictures — pacote de imagens multidimensionais V1

**Estado:** implementação candidata em branch; testes CI e interoperabilidade Drive física exigem receipts. `claim_allowed=false`.

RafPictures é um contêiner **sem perdas de bytes** para várias imagens heterogêneas e metadados de eixos. Não tenta transformar uma fotografia plana em uma cena 3D: `z`, `t`, `c`, ângulo, projeção e outros campos são **declarações explícitas** feitas pelo produtor. Ausência = `TOKEN_VAZIO`.

## Entradas rápidas (Python 3.10+, somente biblioteca padrão)

Executar a partir de `RafPictures/`:

    python3 -m rafpictures pack album.rafp foto.png outra.jpg --depth profundidade.png --preview capa.jpg
    python3 -m rafpictures inspect album.rafp
    python3 -m rafpictures verify album.rafp
    python3 -m rafpictures extract album.rafp imagens-extraidas
    python3 -m rafpictures preview album.rafp capa-extraida.jpg
    python3 -m unittest discover -s tests -v

Para coordenadas adicionais, crie `coords.json` (índice zero de cada entrada, na ordem: fotos, depth, mask, preview):

    {"0":{"z":2,"t":1,"c":0,"projection":"frontal","angle":30}}

Depois adicione `--coords coords.json` ao comando `pack`.

## Extensão e cabeçalho como seletores

- **.rafp**: extensão recomendada para o contêiner; assinaturas são mais confiáveis que extensões.
- **ZIP local header**: inicia com `PK` e preserva compatibilidade com ferramentas ZIP.
- **Primeiro arquivo ZIP**: `mimetype`, sem compressão, conteúdo exato `application/vnd.rafaelia.rafpictures+zip`.
- **Último arquivo**: `manifest.json`, com `schema=rafaelia.rafpictures.v1` e checksum SHA-256 por imagem.
- **Imagens**: PNG, JPEG, GIF, WebP, TIFF, BMP, EXR, QOI, JXL, AVIF, HEIC e HEIF reconhecidos por bytes; nenhum codec de decodificação é embutido.
- **Extensão interna**: preservada como pista para aplicativos externos, inclusive se não coincidir com o MIME identificado.

## Google Drive

Envie `album.rafp` ao Drive como **arquivo binário**, sem conversão para formato Google nativo. Para uma prévia visível no Drive, envie também `capa-extraida.jpg` ou `capa-extraida.png` como arquivo acompanhante. O Drive não é obrigado a mostrar miniatura do `.rafp` ou reconhecer seu MIME customizado. Download deve preservar o arquivo integral.

O suporte de V1 é **interoperabilidade de armazenamento/exportação**; integração autenticada, sincronização bidirecional, thumbnails automáticas, Drive API e testes físicos permanecem `TOKEN_VAZIO`.

## Estrutura

    RafPictures/
      START_HERE.md
      README.md
      SPEC.md
      LICENSE_POLICY.md
      rafpictures/{__init__,__main__,sniff,contract,pack,verify}.py
      include/rafpictures_sniff.h
      tests/test_container.py
      receipts/

## Limites e segurança

Máximo 128 objetos, 512 MiB por objeto, total 4 GiB, manifesto <= 1 MiB. O V1 exige ZIP_STORED (não recompacta JPEG/PNG), rejeita membros extras, duplicados, cifrados, symlinks, travessias de caminho e falhas de hash. Extração é feita em pasta temporária e promovida somente após verificação; destinos existentes não são sobrescritos. Os originais não são modificados.

**Licença:** consultar `LICENSE_POLICY.md`. **Proveniência:** imagem fonte ≠ pacote ≠ preview ≠ execução ≠ evidência.

**Separação futura:** diretório autocontido para migração a um repositório independente `RafPictures`, sem alterar `main` nem apagar o módulo precursor.
