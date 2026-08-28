#!/usr/bin/env python3
"""Versioned arXiv query strategy validated by provider-bound evidence.

This module is deliberately small and side-effect free. It does not promote a
scientific claim; it only turns a free-text bibliography query into the
validated retrieval expression used by the acquisition client.
"""
from __future__ import annotations

import re

STRATEGY_ID = "anchor_and_v1"
_TOKEN_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.+:/-]*", re.UNICODE)


def build_search_expression(query: str, max_terms: int = 3) -> str:
    """Return an arXiv API expression using up to the first three query anchors.

    Empty/whitespace-only input fails closed instead of silently broadening the
    provider search. Terms are quoted independently so phrase-level coupling is
    removed while each selected anchor remains explicit.
    """
    terms = _TOKEN_RE.findall(query or "")
    if not terms:
        raise ValueError("arXiv query has no searchable anchors")
    anchors = terms[:max_terms]
    return " AND ".join(f'all:"{term}"' for term in anchors)


def strategy_receipt(query: str) -> dict:
    """Return deterministic strategy metadata suitable for source receipts."""
    expression = build_search_expression(query)
    return {
        "query_strategy": STRATEGY_ID,
        "search_expression": expression,
        "claim_allowed": False,
    }
