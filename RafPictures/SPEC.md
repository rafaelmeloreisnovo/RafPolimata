# RAFP v1 — contrato de contêiner e interoperabilidade

## Detecção do formato

1. Leia o prefixo binário: o ZIP fornece `50 4B 03 04` (`PK`, não identifica RAFP sozinho).
2. Leia a primeira entrada ZIP: deve chamar-se `mimetype`, com método STORED e bytes ASCII exatos `application/vnd.rafaelia.rafpictures+zip`.
3. Leia a última entrada `manifest.json`; valide `schema`, conjunto e ordem de assets, tipos, limites e checksums.
4. Use `assets[i].mime` como tipo detectado pelo cabeçalho do **payload**, nunca confie exclusivamente na extensão; aplicativos podem escolher seu decodificador por este campo.
5. `version unknown` deve falhar fechado: sem inventar compatibilidade com esquemas futuros.

## Topologia

    album.rafp [ZIP / .rafp]
       mimetype (primeira entrada; ZIP_STORED)
       assets/0000.png (bytes opacos, originais)
       assets/0001.jpg (bytes opacos, originais)
       ...
       manifest.json (última entrada; JSON UTF-8)

Manifesto v1 contém: `schema`, `axes:[x,y,z,t,c,layer]`, `asset_count`, `assets[]`, `preserves_original_bytes`.
Cada asset registra `path`, `source_name`, `role` (`color`, `depth`, `mask`, `preview`), `mime`, `bytes`, `sha256` e `coordinates` (campos não medidos = `TOKEN_VAZIO`).

**Não** afirma que pixels foram decodificados, que profundidade foi medida ou que a imagem possui 3D material. `depth` é uma camada fornecida pelo usuário, sem unidade/calibração implícita.

## Interoperabilidade

| Surface | Seleção | Estado V1 |
|---|---|---|
| Ferramentas ZIP | assinatura PK + estrutura do arquivo | suporte estrutural |
| Aplicativos de imagem | exportar camada original com extensão e MIME | bytes preservados |
| Google Drive | upload/download como arquivo binário | especificado, não testado end-to-end |
| Miniatura Drive | JPEG/PNG lado a lado | exportação implementada, upload não automatizado |
| Android SAF/Termux | arquivos locais `.rafp`, adaptador Python | dispositivo não testado |
| C freestanding | `rafpictures_sniff.h` | detector, não decoder nem parser ZIP |

## Segurança, reversibilidade, gates

Archive inputs read-only. No duplicate member names, traversal, symlinks, unbounded sizes, unsolicited compression, silent overwrites. Verify SHA-256 and image magic before promotion. No personal EXIF is stripped: **metadados internos das imagens continuam presentes**; avalie privacidade antes de compartilhar.

Gate local: `python3 -m unittest discover -s tests -v` e C11 `-Wall -Wextra -Werror -pedantic`. Gate GitHub CI: execução e conclusão PASS na revisão exata. Gate Android: runtime físico multi-ABI com receipt próprio. Gate Drive: upload + download + igualdade SHA-256 e preview separada. Até esses gates, `claim_allowed=false`.

Rollback: fechar PR e descartar branch; originais permanecem intocados. Mudança incompatível de layout exige novo `schema` e novos testes; nunca reinterpretar V1 silenciosamente.
