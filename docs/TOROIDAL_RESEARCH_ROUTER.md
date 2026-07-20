# Toroidal Research Router

`RafPolimata` routes research objects between canonical repository authorities.
It does not transfer ownership and does not promote scientific claims.

## Routing invariant

```text
source authority -> object kind -> allowed action -> target authority
```

Examples:

- an RLL formula may be sent to `RafPolimata` for validation or execution;
- an RLL bounded claim may be consumed by `llamaRafaelia`;
- repository metadata originates in `Mapa`;
- device evidence originates in the runtime repository;
- evidence classification and promotion gates remain in `RafGitTools`.

## Hard blocks

- `PROMOTE` is forbidden to the router;
- a scientific claim cannot originate from the orchestration repository;
- a device claim without a runtime receipt becomes `BLOCKED_TOKEN_VAZIO`;
- a route never returns `claim_allowed=true`;
- unknown repositories, kinds, actions and authority crossings fail closed.

## Files

```text
contracts/toroidal_research_router.v1.json
scripts/toroidal_research_router.py
examples/toroidal_research_router.example.json
tests/test_toroidal_research_router.py
docs/TOROIDAL_RESEARCH_ROUTER.md
```

The validator is Python standard-library only and is integrated into the existing
formal-science and local truth gates. No additional workflow is introduced.
