# Governance anchor: CLOSURE_L2.
import copy
import importlib.util
import json
import pathlib
import tempfile
import unittest
from unittest import mock

ROOT = pathlib.Path(__file__).resolve().parents[1]


def load_module(name, path):
    spec=importlib.util.spec_from_file_location(name,path)
    mod=importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(mod)
    return mod


eg=load_module("evidence_garden",ROOT/"scripts"/"evidence_garden.py")
fa=load_module("evidence_garden_falsifier_audit",ROOT/"scripts"/"evidence_garden_falsifier_audit.py")


class EvidenceGardenFalsifierTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base=json.loads((ROOT/"Benchmark/evidence_garden/demo_experiment.json").read_text(encoding="utf-8"))

    def write_cfg(self,cfg):
        td=tempfile.TemporaryDirectory(dir=ROOT)
        p=pathlib.Path(td.name)/"cfg.json"
        p.write_text(json.dumps(cfg),encoding="utf-8")
        self.addCleanup(td.cleanup)
        return p

    def load_mut(self, mutate):
        cfg=copy.deepcopy(self.base); mutate(cfg)
        return eg.load_config(self.write_cfg(cfg))

    def test_f00_wrong_schema_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c.__setitem__("schema","bad"))

    def test_f01_missing_experiment_id_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c.pop("experiment_id"))

    def test_f02_duplicate_variant_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c["variants"][1].__setitem__("id",c["variants"][0]["id"]))

    def test_f03_unknown_baseline_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c.__setitem__("baseline_variant","void"))

    def test_f04_invalid_command_argv_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c["variants"][0].__setitem__("command","sh bad"))

    def test_f05_path_escape_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c["artifacts"][0].__setitem__("path","../escape"))

    def test_f06_missing_required_artifact_fails_identity(self):
        cfg=copy.deepcopy(self.base)
        cfg["artifacts"]=[{"path":"build/definitely-missing-evidence-garden.bin","required":True}]
        with tempfile.TemporaryDirectory(dir=ROOT) as td:
            r=eg.run_experiment(cfg,"0"*64,pathlib.Path(td)/"receipt.json")
        self.assertEqual(r["stations"]["S0_identity"]["state"],"FAIL")
        self.assertEqual(r["run_state"],"FAIL")

    def test_f07_missing_command_executable_fails_identity(self):
        cfg=copy.deepcopy(self.base)
        cfg["variants"][0]["command"]=["rafpolimata-command-that-does-not-exist"]
        with tempfile.TemporaryDirectory(dir=ROOT) as td:
            r=eg.run_experiment(cfg,"1"*64,pathlib.Path(td)/"receipt.json")
        self.assertEqual(r["stations"]["S0_identity"]["state"],"FAIL")
        self.assertIn("reference",r["stations"]["S0_identity"]["missing_commands"])
        self.assertEqual(r["run_state"],"FAIL")

    def test_f08_wrong_expected_exit_fails_correctness(self):
        cfg=copy.deepcopy(self.base); cfg["correctness"]["expected_exit"]=7
        r=eg.correctness_station(cfg)
        self.assertEqual(r["state"],"FAIL")

    def test_f09_wrong_reference_digest_fails_accuracy(self):
        cfg=copy.deepcopy(self.base); cfg["correctness"]["reference_stdout_sha256"]="0"*64
        r=eg.correctness_station(cfg)
        self.assertEqual(r["reference_accuracy"]["state"],"FAIL")
        self.assertEqual(r["state"],"FAIL")

    def test_f10_required_probe_unavailable_fails(self):
        cfg=copy.deepcopy(self.base)
        cfg["observability"]={"enabled":True,"probes":[{"kind":"strace_summary","required":True}]}
        with tempfile.TemporaryDirectory(dir=ROOT) as td, mock.patch.object(eg.shutil,"which",return_value=None):
            r=eg.observability_station(cfg,pathlib.Path(td))
        self.assertEqual(r["state"],"FAIL")

    def test_f11_optional_probe_unavailable_is_bounded(self):
        cfg=copy.deepcopy(self.base)
        cfg["observability"]={"enabled":True,"probes":[{"kind":"strace_summary","required":False}]}
        with tempfile.TemporaryDirectory(dir=ROOT) as td, mock.patch.object(eg.shutil,"which",return_value=None):
            r=eg.observability_station(cfg,pathlib.Path(td))
        self.assertEqual(r["state"],"PASS")
        self.assertEqual(r["probes"][0]["state"],eg.TOKEN_VAZIO)

    def test_required_observability_failure_blocks_run(self):
        cfg=copy.deepcopy(self.base)
        with tempfile.TemporaryDirectory(dir=ROOT) as td, mock.patch.object(eg,"observability_station",return_value={"state":"FAIL"}):
            r=eg.run_experiment(cfg,"2"*64,pathlib.Path(td)/"receipt.json")
        self.assertEqual(r["run_state"],"FAIL")

    def test_f12_zero_rounds_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c["performance"].__setitem__("rounds",0))

    def test_f13_negative_warmup_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c["performance"].__setitem__("warmup",-1))

    def test_f14_unsupported_order_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c["performance"].__setitem__("order","random-unrecorded"))

    def test_invalid_timeout_rejected(self):
        with self.assertRaises(ValueError): self.load_mut(lambda c:c["performance"].__setitem__("timeout_seconds",0))

    def test_unsupported_probe_kind_rejected(self):
        with self.assertRaises(ValueError):
            self.load_mut(lambda c:c["observability"].__setitem__("probes",[{"kind":"mystery","required":False}]))

    def test_f15_execution_timeout_fails(self):
        r=eg.run_capture(["python3","-c","import time; time.sleep(0.05)"],timeout=0.001)
        self.assertEqual(r["state"],"FAIL")
        self.assertTrue(r.get("timeout"))

    def test_f16_multi_factor_causality_not_promoted(self):
        d=eg.factors_delta({"a":0,"b":0},{"a":1,"b":1})
        self.assertEqual(len(d),2)

    def test_f17_small_n_interval_stays_token_vazio(self):
        self.assertEqual(eg.median_ci_nonparametric([1,2,3,4,5])["state"],eg.TOKEN_VAZIO)

    def sample_receipt(self):
        cfg=copy.deepcopy(self.base)
        with tempfile.TemporaryDirectory(dir=ROOT) as td:
            return eg.run_experiment(cfg,"3"*64,pathlib.Path(td)/"receipt.json")

    def test_f18_receipt_schema_mismatch_rejected(self):
        a=self.sample_receipt(); b=copy.deepcopy(a); b["schema"]="bad"
        self.assertEqual(eg.compare_receipts([a,b])["state"],"FAIL")

    def test_f19_receipt_source_mismatch_rejected(self):
        a=self.sample_receipt(); b=copy.deepcopy(a)
        b["stations"]["S0_identity"]["repository_head"]="different"
        self.assertEqual(eg.compare_receipts([a,b])["state"],"FAIL")

    def test_f20_receipt_correctness_mismatch_rejected(self):
        a=self.sample_receipt(); b=copy.deepcopy(a)
        first=next(iter(b["stations"]["S1_correctness"]["variants"].values()))
        first["stdout_sha256"]="f"*64
        self.assertEqual(eg.compare_receipts([a,b])["state"],"FAIL")

    def test_f21_claim_promotion_forbidden(self):
        a=self.sample_receipt(); b=copy.deepcopy(a); b["claim_allowed"]=True
        self.assertEqual(eg.compare_receipts([a,b])["state"],"FAIL")

    def test_f22_physical_visibility_remains_token_vazio(self):
        a=self.sample_receipt()
        self.assertEqual(a["stations"]["S7_claim_gate"]["physical_signal_visibility"],eg.TOKEN_VAZIO)

    def test_f23_open_world_unknown_unknown_remains_token_vazio(self):
        matrix=json.loads((ROOT/"Benchmark/evidence_garden/falsifier_matrix.v2.json").read_text(encoding="utf-8"))
        result=fa.audit(matrix)
        self.assertEqual(result["state"],"PASS")
        self.assertEqual(result["open_world_unknown_unknown"],eg.TOKEN_VAZIO)
        self.assertEqual(result["pairwise_permutation_count"],66)

    def test_matrix_covers_all_declared_stations_and_dimensions(self):
        matrix=json.loads((ROOT/"Benchmark/evidence_garden/falsifier_matrix.v2.json").read_text(encoding="utf-8"))
        result=fa.audit(matrix)
        self.assertEqual(result["known_falsifier_closure"],"PASS")
        self.assertEqual(result["errors"],[])


if __name__=="__main__":
    unittest.main()
