"""Compatibility entrypoint for the corrected Science Learning Engine v2.

PR #178's first implementation is superseded because ORCID search results were
parsed as works and acquisition states were overstated as scientific validation.
The public command and import surface remain compatible.
"""
try:
    from scripts import science_learning_engine_v2 as _impl
except ModuleNotFoundError:
    import science_learning_engine_v2 as _impl

# Preserve the former module surface, including private helpers used by tests.
globals().update({name: value for name, value in vars(_impl).items() if not name.startswith("__")})

if __name__ == "__main__":
    _impl.main()
