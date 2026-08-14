#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import json
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] if Path(__file__).resolve().parent.name == "scripts" else Path.cwd()
MODULE_PATH = ROOT / "scripts" / "runtime_doctor_agent.py"
if not MODULE_PATH.exists():
    MODULE_PATH = Path('/mnt/data/runtime_doctor_agent.py')

SPEC = importlib.util.spec_from_file_location("runtime_doctor_agent_ci", MODULE_PATH)
if SPEC is None or SPEC.loader is None:
    raise SystemExit("FAIL: cannot load runtime_doctor_agent.py")
RDA = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = RDA
SPEC.loader.exec_module(RDA)


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS {label}")


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="runtime-doctor-ci-") as tmp_raw:
        tmp = Path(tmp_raw)
        workspace = tmp / "workspace"
        repo = workspace / "repo"
        repo.mkdir(parents=True)

        counter = repo / "counter.txt"
        probe = repo / "probe.py"
        probe.write_text(
            "from pathlib import Path\n"
            "import json\n"
            "p=Path('counter.txt')\n"
            "n=int(p.read_text() or '0') if p.exists() else 0\n"
            "p.write_text(str(n+1))\n"
            "print(json.dumps({'arch':'aarch64','neon':True,'opencl':False,'vulkan':False,'ram_avail':900000,'page_sz':4096,'oom':0}, sort_keys=True))\n",
            encoding="utf-8",
        )

        command = ("python3", "probe.py")
        skills = [
            RDA.Skill("host", "L1", "repo", "host", command, ("termux",), "runtime_json"),
            RDA.Skill("gpu", "L2", "repo", "gpu", command, ("gpu",), "runtime_json"),
        ]
        probe_results = RDA.execute_selected_probes(skills, {"repo": repo}, 10, RDA.Trace(False))
        require(counter.read_text(encoding="utf-8") == "1", "identical_probe_executes_once")
        require(probe_results[0]["probe_cache"]["reused"] is False, "first_probe_not_reused")
        require(probe_results[1]["probe_cache"]["reused"] is True, "second_probe_reused")
        require(probe_results[1]["probe_cache"]["source_skill_id"] == "host", "probe_reuse_provenance")

        graph = {
            "route_graph": [
                ["host", "frida"],
                ["frida", "evidence"],
                ["evidence", "L7"],
                ["unrelated", "L7"],
            ]
        }
        require(
            RDA.reachable_route_graph(graph, {"host"})
            == [["host", "frida"], ["frida", "evidence"], ["evidence", "L7"]],
            "reachable_route_graph",
        )

        bd = tmp / "build-doctor.json"
        bd.write_text(
            json.dumps(
                {
                    "schema": "raf.ecosystem-build-doctor-report.v1",
                    "summary": {
                        "state": "PASS_LIMITED",
                        "highest_severity": "medium",
                        "findings": 1,
                        "by_code": {"demo": 1},
                        "by_repo": {"repo": 1},
                    },
                    "findings": [],
                    "claim_boundary": {
                        "static_analysis": "VERIFIED_BY_EXECUTION",
                        "build_execution": "TOKEN_VAZIO",
                    },
                },
                sort_keys=True,
            ),
            encoding="utf-8",
        )
        evidence = RDA.load_build_doctor_reports([str(bd)], RDA.Trace(False))
        require(evidence[0]["state"] == "INGESTED_HASH_BOUND_REPORT", "build_doctor_ingested")
        require(evidence[0]["source_sha256"] == RDA.sha256_file(bd), "build_doctor_sha256_bound")
        require("source_path" not in evidence[0], "build_doctor_path_minimized")

        registry = tmp / "skills.json"
        registry.write_text(
            json.dumps(
                {
                    "schema": "raf.runtime-doctor-skills.v1",
                    "skills": [
                        {
                            "id": "frida_runtime_observer",
                            "level": "L2.5",
                            "repo": "repo",
                            "role": "observer",
                            "probe": [],
                            "keywords": ["frida"],
                            "evidence": "runtime_readiness_receipt",
                        }
                    ],
                    "route_graph": [["frida_runtime_observer", "L7"]],
                },
                sort_keys=True,
            ),
            encoding="utf-8",
        )
        args = RDA.parser().parse_args(
            [
                "--skills",
                str(registry),
                "--workspace",
                str(workspace),
                "--symptom",
                "frida",
                "--build-doctor-report",
                str(bd),
            ]
        )
        report = RDA.build_report(args)
        require(report["claim_boundary"]["automatic_repair"] is False, "automatic_repair_false")
        require(report["claim_boundary"]["automatic_install"] is False, "automatic_install_false")
        require(report["claim_boundary"]["automatic_delete"] is False, "automatic_delete_false")
        require(report["claim_boundary"]["claim_allowed"] is False, "claim_allowed_false")
        require(
            any(
                gap["id"] == "GAP-RD-FRIDA-PHYSICAL"
                and gap["urgency"] == "P0"
                and gap["state"] == "TOKEN_VAZIO_PHYSICAL_DEVICE_RECEIPT_REQUIRED"
                for gap in report["gap_ledger"]
            ),
            "frida_physical_gap_preserved_p0",
        )
        require(
            report["claim_boundary"]["build_doctor_evidence"] == "HASH_BOUND_INPUT_REPORT_ONLY",
            "static_evidence_not_promoted_to_build",
        )

        contract = ROOT / "contracts" / "runtime-doctor-agent-report.schema.json"
        if contract.exists():
            parsed = json.loads(contract.read_text(encoding="utf-8"))
            require(parsed.get("type") == "object", "report_contract_json_parse")

        print("RUNTIME_DOCTOR_COHERENCE_GATE=PASS")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
