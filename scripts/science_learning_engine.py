"""Compatibility entrypoint for the corrected Science Learning Engine v2.

PR #178's first implementation is superseded because ORCID search results were
parsed as works and acquisition states were overstated as scientific validation.
The public command remains unchanged.
"""
try:
    from scripts.science_learning_engine_v2 import *  # noqa: F401,F403
except ModuleNotFoundError:
    from science_learning_engine_v2 import *  # noqa: F401,F403

if __name__ == "__main__":
    main()
