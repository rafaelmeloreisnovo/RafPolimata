#!/usr/bin/env python3
from __future__ import annotations
import json, math
from pathlib import Path

Q=math.sqrt(3.0)/2.0

def pg_frequencies(f0: float, count: int) -> list[float]:
    return [f0*(Q**n) for n in range(count)]

def harmonic_frequencies(f0: float, count: int) -> list[float]:
    return [f0*(n+1) for n in range(count)]

def triangle(side: float=1.0):
    h=side*Q
    return {"side":side,"height":h,"pythagoras_check":(side/2)**2+h*h,"expected_side_sq":side*side}

def venturi(area1:float, area2:float, v1:float, rho:float=1.225):
    v2=area1*v1/area2
    delta_p=0.5*rho*(v2*v2-v1*v1)
    return {
        "A1":area1,"A2":area2,"v1":v1,"v2":v2,
        "ideal_static_pressure_drop_P1_minus_P2":delta_p,
        "boundary":"INCOMPRESSIBLE_IDEAL_BERNOULLI_MODEL_NOT_ACOUSTIC_WAVE_EQUATION"
    }

def wave_samples(freqs:list[float], sample_rate:int=48000, duration:float=0.04):
    n=int(sample_rate*duration)
    out=[]
    for i in range(n):
        t=i/sample_rate
        y=sum(math.sin(2*math.pi*f*t)/(j+1) for j,f in enumerate(freqs))
        out.append((t,y))
    return out

def svg_wave(samples, width=1200, height=360):
    if len(samples) > 240:
        step=max(1,len(samples)//240)
        samples=samples[::step]
    ys=[y for _,y in samples]; lo=min(ys); hi=max(ys); span=hi-lo or 1
    pts=[]
    for i,(_,y) in enumerate(samples):
        x=i*(width-40)/(len(samples)-1)+20
        sy=height-20-(y-lo)*(height-40)/span
        pts.append(f"{x:.2f},{sy:.2f}")
    return f'''<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">
<rect width="100%" height="100%" fill="white"/>
<text x="20" y="24" font-family="sans-serif" font-size="18">RAFAELIA PG acoustic test — synthetic only — q=sqrt(3)/2</text>
<polyline fill="none" stroke="black" stroke-width="1" points="{' '.join(pts)}"/>
</svg>'''

def svg_triangle(side_px=320, margin=60):
    width=side_px+2*margin; height=int(side_px*Q)+2*margin
    x1,y1=margin,height-margin; x2,y2=margin+side_px,height-margin; xm=(x1+x2)/2; yt=margin
    return f'''<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">
<rect width="100%" height="100%" fill="white"/>
<polyline points="{x1},{y1} {xm},{yt} {x2},{y2} {x1},{y1}" fill="none" stroke="black" stroke-width="2"/>
<line x1="{xm}" y1="{yt}" x2="{xm}" y2="{y1}" stroke="black" stroke-dasharray="6,6"/>
<text x="{xm+8}" y="{(yt+y1)/2}" font-family="sans-serif" font-size="16">h=sqrt(3)/2 · s</text>
<text x="{xm-42}" y="{y1+28}" font-family="sans-serif" font-size="16">s/2</text>
</svg>'''

def main():
    root=Path(__file__).resolve().parents[1]
    results=root/"results"; results.mkdir(parents=True, exist_ok=True)
    pg=pg_frequencies(440.0,7)
    harm=harmonic_frequencies(440.0,7)
    report={
      "schema":"rafpolimata.acoustic-geometry-bridge.report.v1",
      "claim_allowed":False,
      "q":Q,
      "triangle":triangle(),
      "pg_test_hz":pg,
      "true_harmonics_hz":harm,
      "critical_distinction":"PG(q=sqrt(3)/2) is not a harmonic series; harmonics are integer multiples of f0.",
      "venturi_example":venturi(0.01,0.005,2.0),
      "room_acoustics_boundary":"Audience occupancy changes absorption and reverberation; it is not modeled as simple destructive cancellation of all waves."
    }
    (results/"acoustic_geometry_report.v1.json").write_text(json.dumps(report,indent=2,sort_keys=True)+"\n")
    (results/"pg_wave_synthetic.svg").write_text(svg_wave(wave_samples(pg))+"\n")
    (results/"equilateral_projection.svg").write_text(svg_triangle()+"\n")

if __name__=="__main__":
    main()
