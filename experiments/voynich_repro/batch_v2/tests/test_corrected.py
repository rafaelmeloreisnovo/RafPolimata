from pathlib import Path
import importlib.util

def load(path):
    spec=importlib.util.spec_from_file_location("m",path)
    m=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m

def test_sequence_order():
    m=load(Path(__file__).parents[1]/"corrected/voynich_analysis_v2.py")
    assert m.sequence_order(10,"123",5)==[0,1,3,6,7]
    assert m.sequence_order(10,"0123",5)==[0,0,1,3,6]

def test_safe_basename():
    m=load(Path(__file__).parents[1]/"corrected/voynich_downloader_v2.py")
    assert m.safe_basename("folder/page_001.jpg")=="page_001.jpg"
    for bad in ("../x.jpg","/x.jpg","a\\b.jpg"):
        try: m.safe_basename(bad)
        except ValueError: pass
        else: raise AssertionError(bad)
