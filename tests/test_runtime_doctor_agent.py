import importlib.util
import json
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "scripts" / "runtime_doctor_agent.py"
SPEC = importlib.util.spec_from_file_location("runtime_doctor_agent", MODULE_PATH)
assert SPEC and SPEC.loader
RDA = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = RDA
SPEC.loader.exec_module(RDA)

FRIDA_MODULE_PATH = ROOT / "scripts" / "frida_runtime_probe.py"
FRIDA_SPEC = importlib.util.spec_from_file_location("frida_runtime_probe", FRIDA_MODULE_PATH)
assert FRIDA_SPEC and FRIDA_SPEC.loader
FRIDA = importlib.util.module_from_spec(FRIDA_SPEC)
sys.modules[FRIDA_SPEC.name] = FRIDA
FRIDA_SPEC.loader.exec_module(FRIDA)


def test_skill_registry_loads():
    raw, skills = RDA.load_skills(ROOT / "configs" / "runtime-doctor-skills.v1.json")
    assert raw["schema"] == "raf.runtime-doctor-skills.v1"
    ids = {skill.id for skill in skills}
    assert "termux_host_diagnostic" in ids
    assert "frida_runtime_observer" in ids
    assert "llama_backend_doctor" in ids
    assert "rafpolimata_evidence_doctor" in ids
    assert "rafaeliaprivate_context" in ids
    assert ["termux_host_diagnostic", "frida_runtime_observer"] in raw["route_graph"]
    assert ["frida_runtime_observer", "rafpolimata_evidence_doctor"] in raw["route_graph"]


def test_parse_last_json_line():
    text = "verbose line\nmore logs\n{\"arch\":\"armv7l\",\"vulkan\":true}\n"
    assert RDA.parse_last_json_line(text) == {"arch": "armv7l", "vulkan": True}


def test_runtime_routes_vulkan_neon_opencl():
    probes = [
        {
            "skill_id": "termux_host_diagnostic",
            "payload": {
                "arch": "armv7l",
                "neon": True,
                "opencl": True,
                "vulkan": True,
                "ram_avail": 900000,
                "page_sz": 4096,
                "oom": 0,
            },
        }
    ]
    routes = RDA.route_from_runtime(probes, RDA.Trace(False))
    codes = {route["code"] for route in routes}
    assert "ARM32_ROUTE" in codes
    assert "LLAMA_VULKAN_CANDIDATE" in codes
    assert "OPENCL_GENERAL_GPU_CANDIDATE" in codes
    assert "NEON_ACCELERATION_CANDIDATE" in codes
    assert "MEMORY_PRESSURE" in codes
    assert "LLAMA_CPU_FALLBACK" not in codes


def test_runtime_routes_cpu_fallback_and_oom():
    probes = [
        {
            "skill_id": "termux_host_diagnostic",
            "payload": {
                "arch": "aarch64",
                "neon": False,
                "opencl": False,
                "vulkan": False,
                "ram_avail": 400000,
                "page_sz": 16384,
                "oom": 700,
            },
        }
    ]
    routes = RDA.route_from_runtime(probes, RDA.Trace(False))
    codes = {route["code"] for route in routes}
    assert "ARM64_ROUTE" in codes
    assert "LLAMA_CPU_FALLBACK" in codes
    assert "MEMORY_PRESSURE_CRITICAL" in codes
    assert "OOM_KILL_RISK" in codes
    assert "ANDROID_16K_PAGE_CANDIDATE" in codes


def test_laplace_learning_never_claims_without_history():
    empty = RDA.confidence_for(RDA.Counter())
    assert empty["state"] == "TOKEN_VAZIO_NO_OUTCOME_HISTORY"
    assert empty["success_rate_laplace"] is None

    counter = RDA.Counter({"PASS": 3, "FAIL": 1})
    learned = RDA.confidence_for(counter)
    assert learned["state"] == "EVIDENCE_WEIGHT_AVAILABLE"
    assert learned["observations"] == 4
    assert learned["success_rate_laplace"] == round(4 / 6, 6)


def test_safe_probe_rejects_install_and_delete():
    assert RDA.safe_probe_command(("sh", "Arme/Add/diagnose.sh", "--json"))
    assert RDA.safe_probe_command(("python3", "scripts/frida_runtime_probe.py", "--json"))
    assert not RDA.safe_probe_command(("sh", "-c", "pkg install x"))
    assert not RDA.safe_probe_command(("sh", "-c", "rm -rf x"))


def test_build_report_executes_claim_boundary_without_name_error(tmp_path):
    args = RDA.parser().parse_args(["--workspace", str(tmp_path)])
    report = RDA.build_report(args)
    assert report["claim_boundary"]["automatic_repair"] is False
    assert report["claim_boundary"]["automatic_install"] is False
    assert report["claim_boundary"]["automatic_delete"] is False
    assert report["claim_boundary"]["claim_allowed"] is False
    assert report["summary"]["state"] == "PASS_LIMITED"
    assert any(gap["id"] == "GAP-RD-RUNTIME-EXECUTION" for gap in report["gap_ledger"])


def test_identical_probe_is_executed_once_and_reused(tmp_path, monkeypatch):
    repo = tmp_path / "repo"
    repo.mkdir()
    probe = ("python3", "probe.py", "--json")
    skills = [
        RDA.Skill("host", "L1", "repo", "host", probe, ("host",), "runtime_json"),
        RDA.Skill("gpu", "L2", "repo", "gpu", probe, ("gpu",), "runtime_json"),
    ]
    calls = []

    def fake_run_probe(skill, repo_root, timeout_s, trace):
        calls.append((skill.id, repo_root, timeout_s))
        return {
            "skill_id": skill.id,
            "state": "PASS",
            "repo": skill.repo,
            "exit_code": 0,
            "command": list(skill.probe),
            "payload": {"arch": "armv7l"},
            "stdout_tail": "{}",
            "stderr_tail": "",
        }

    monkeypatch.setattr(RDA, "run_probe", fake_run_probe)
    results = RDA.execute_selected_probes(skills, {"repo": repo}, 5, RDA.Trace(False))
    assert len(calls) == 1
    assert len(results) == 2
    assert results[0]["probe_cache"]["reused"] is False
    assert results[1]["probe_cache"]["reused"] is True
    assert results[1]["probe_cache"]["source_skill_id"] == "host"


def test_reachable_route_graph_preserves_downstream_context():
    registry = {
        "route_graph": [
            ["host", "frida"],
            ["frida", "evidence"],
            ["evidence", "L7"],
            ["unrelated", "L7"],
        ]
    }
    assert RDA.reachable_route_graph(registry, {"host"}) == [
        ["host", "frida"],
        ["frida", "evidence"],
        ["evidence", "L7"],
    ]


def test_build_doctor_evidence_is_hash_bound_and_schema_checked(tmp_path):
    report_path = tmp_path / "build-doctor.json"
    report_path.write_text(
        json.dumps(
            {
                "schema": "raf.ecosystem-build-doctor-report.v1",
                "summary": {
                    "state": "REVIEW_REQUIRED",
                    "highest_severity": "high",
                    "findings": 2,
                    "by_code": {"example": 2},
                    "by_repo": {"demo": 2},
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
    records = RDA.load_build_doctor_reports([str(report_path)], RDA.Trace(False))
    assert len(records) == 1
    assert records[0]["state"] == "INGESTED_HASH_BOUND_REPORT"
    assert records[0]["source_sha256"] == RDA.sha256_file(report_path)
    assert records[0]["summary"]["state"] == "REVIEW_REQUIRED"
    assert "source_path" not in records[0]


def test_gap_ledger_marks_frida_physical_receipt_as_p0():
    args = RDA.parser().parse_args([])
    skill_routes = [
        {
            "skill_id": "frida_runtime_observer",
            "repo_state": "AVAILABLE",
        }
    ]
    gaps = RDA.build_gap_ledger(args, skill_routes, [], [])
    physical = next(g for g in gaps if g["id"] == "GAP-RD-FRIDA-PHYSICAL")
    assert physical["urgency"] == "P0"
    assert physical["state"] == "TOKEN_VAZIO_PHYSICAL_DEVICE_RECEIPT_REQUIRED"


def test_frida_device_parser_is_read_only_and_structured():
    text = "Id  Type   Name\n----------------------\nlocal local  Local System\nusb1 usb    Android Device\n"
    devices = FRIDA.parse_device_lines(text)
    assert devices == [
        {"id": "local", "type": "local", "name": "Local System"},
        {"id": "usb1", "type": "usb", "name": "Android Device"},
    ]


def test_frida_device_identifiers_are_pseudonymized():
    raw = [{"id": "DEVICE-ID-123", "type": "usb", "name": "Test Phone"}]
    safe = FRIDA.pseudonymize_devices(raw)
    serialized = str(safe)
    assert "DEVICE-ID-123" not in serialized
    assert "Test Phone" not in serialized
    assert safe[0]["type"] == "usb"
    assert safe[0]["name_present"] is True
    assert len(safe[0]["id_sha256_16"]) == 16
    assert len(safe[0]["name_sha256_16"]) == 16


def test_frida_probe_output_is_minimized():
    raw = {"command": ["frida-ls-devices"], "exit_code": 0, "stdout": "device-output", "stderr": ""}
    safe = FRIDA.redact_probe_output(raw, "device-enumeration")
    assert safe["stdout"] == "<redacted:device-enumeration>"
    assert safe["stdout_sha256"] == FRIDA.sha256_text("device-output")
    assert safe["stdout_bytes"] == len(b"device-output")


def test_frida_report_never_claims_dynamic_actions(monkeypatch):
    monkeypatch.setattr(FRIDA, "command_info", lambda name: {"name": name, "available": False, "path": None})
    monkeypatch.setattr(FRIDA, "getprop", lambda key: None)
    report = FRIDA.build_report(None)
    assert report["state"] == "TOKEN_VAZIO_FRIDA_TOOLS_NOT_FOUND"
    assert report["capabilities"]["attach_tested"] is False
    assert report["capabilities"]["hook_tested"] is False
    assert report["policy"]["automatic_patch"] is False
    assert report["policy"]["claim_allowed"] is False
    assert report["privacy"]["raw_device_ids_stored"] is False
    assert report["privacy"]["raw_device_enumeration_stdout_stored"] is False
