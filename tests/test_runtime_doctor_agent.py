import importlib.util
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "scripts" / "runtime_doctor_agent.py"
SPEC = importlib.util.spec_from_file_location("runtime_doctor_agent", MODULE_PATH)
assert SPEC and SPEC.loader
RDA = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = RDA
SPEC.loader.exec_module(RDA)


def test_skill_registry_loads():
    raw, skills = RDA.load_skills(ROOT / "configs" / "runtime-doctor-skills.v1.json")
    assert raw["schema"] == "raf.runtime-doctor-skills.v1"
    ids = {skill.id for skill in skills}
    assert "termux_host_diagnostic" in ids
    assert "llama_backend_doctor" in ids
    assert "rafpolimata_evidence_doctor" in ids
    assert "rafaeliaprivate_context" in ids


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
    assert not RDA.safe_probe_command(("sh", "-c", "pkg install x"))
    assert not RDA.safe_probe_command(("sh", "-c", "rm -rf x"))
