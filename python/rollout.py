"""Example rollout collector; scripted baseline, not a trained model.

    python3 python/rollout.py --arenas 256 --decisions 1000
"""
import argparse
import time
from creature import Batch, ARENA_COUNT
p = argparse.ArgumentParser()
p.add_argument('--arenas', type=int, default=256)
p.add_argument('--decisions', type=int, default=1000)
a = p.parse_args()
start = time.perf_counter()
finished = ticks = 0
with Batch(a.arenas, seed=42, weather=2, species=(12,27)) as batch:
    for t in range(a.decisions):
        observations, features, status = batch.step(batch.scripted_actions())
        # Reward remains a training choice. Zero-sum sparse KO reward example:
        # reward[player] = features[player * 10 + 4] - features[player * 10 + 5]
        # Copy terminal observation / recurrent state BEFORE resetting.
        for i in range(a.arenas):
            ticks += status[i * 4 + 3]
            if status[i * 4] or status[i * 4 + 1]:
                finished += 1
                batch.reset(i, 42 + finished * a.arenas + i, weather=t % 3, species=(i%40,(i+17)%40), arena=t%ARENA_COUNT)
seconds = time.perf_counter() - start
print(f'{finished} episodes; {ticks:,} physics ticks; {seconds:.3f}s')
print(f'{a.arenas*a.decisions/seconds:,.0f} joint decisions/s including Python collection')
