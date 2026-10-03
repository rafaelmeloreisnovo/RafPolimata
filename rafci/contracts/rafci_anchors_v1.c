#include "rafci_wire_v1.h"

/*
RAFCI-FILE-CONTRACT
PURPOSE=Expose bounded RafCI semantic anchors as linker-visible ELF sections and symbols.
SCOPE=Constant metadata only; no runtime engine, libc, heap, syscall, I/O or external helper.
PRECONDITIONS=Clang/GCC-compatible section attributes and relocatable ELF output.
REGISTER_OWNERSHIP=NONE
CLOBBERS=NONE
MEMORY_ORDER=NONE
TAIL_SHADOW=NONE
EVIDENCE=Section/symbol presence is structural build evidence only.
*/

/*
RAFCI-BIT
ID=rafci.bit.anchor.file.v1
KIND=anchor
ROUTE=stage.source>stage.artifact
AUTHORITY=authority.rafpolimata.rafci
EVIDENCE=Defines linker-visible metadata; physical/runtime evidence remains TOKEN_VAZIO.
*/

#if defined(__clang__) || defined(__GNUC__)
#define RAFCI_SECTION(name_) __attribute__((section(name_), used, aligned(1)))
#else
#error "RafCI anchors require compiler section attributes"
#endif

/*
RAFCI-BIT
ID=rafci.bit.anchor.authority.v1
KIND=anchor
ROUTE=provider.rafpolimata>stage.source
AUTHORITY=authority.rafpolimata.rafci
EVIDENCE=Identifies semantic contract authority; does not transfer provider implementation authority.
*/
const char rafci_authority_anchor_v1[]
    RAFCI_SECTION(".rafci.authority") =
    "rafci.authority=authority.rafpolimata.rafci;schema=v1";

/*
RAFCI-BIT
ID=rafci.bit.anchor.route.v1
KIND=route
ROUTE=stage.source>stage.artifact>stage.execution>stage.evidence>stage.claim
AUTHORITY=authority.rafpolimata.rafci
EVIDENCE=Encodes the ordered evidence boundary; section presence is not execution.
*/
const char rafci_route_anchor_v1[]
    RAFCI_SECTION(".rafci.route") =
    "SOURCE>ARTIFACT>EXECUTION>EVIDENCE>CLAIM";

/*
RAFCI-BIT
ID=rafci.bit.anchor.gates.v1
KIND=gate
ROUTE=stage.evidence>stage.claim
AUTHORITY=authority.rafpolimata.rafci
EVIDENCE=Pins gate-dictionary version; individual gate outcomes belong in receipts.
*/
const char rafci_gate_anchor_v1[]
    RAFCI_SECTION(".rafci.gates") =
    "rafci.gates=source,artifact,no-needed,no-interp,no-undefined,anchors,hosted,physical,provider,provenance;v1";
