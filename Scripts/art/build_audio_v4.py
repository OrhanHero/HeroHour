"""Deterministic original synthesized prototype effects; no reference-game audio."""
from pathlib import Path
import math, random, struct, wave

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'Art/Audio';OUT.mkdir(parents=True,exist_ok=True)
RATE=22050
for name,duration,low,decay in [('Rifle',.12,180,32),('Cannon',.3,70,15),('Impact',.45,45,10)]:
    rng=random.Random(name);samples=[];smooth=0.
    for i in range(int(RATE*duration)):
        t=i/RATE;noise=rng.uniform(-1,1);smooth=.72*smooth+.28*noise
        envelope=math.exp(-t*decay)*min(1,t*1500)
        tone=math.sin(2*math.pi*(low*t+low*.6*(1-math.exp(-12*t))/12))
        signal=(smooth*.7+tone*.3)*envelope
        samples.append(struct.pack('<h',int(max(-1,min(1,signal))*.75*32767)))
    with wave.open(str(OUT/f'A_{name}.wav'),'wb') as output:
        output.setparams((1,2,RATE,len(samples),'NONE','not compressed'));output.writeframes(b''.join(samples))
    print(name,len(samples),'original mono PCM samples')
