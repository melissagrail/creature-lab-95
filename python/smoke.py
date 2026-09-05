"""FFI parity, batch independence, snapshot forks and lifecycle regression tests."""
from creature import Batch, OBS_SIZE, quantize
import math

with Batch(8, seed=20, weather=2) as env, Batch(1) as fork:
    for i in range(40):
        obs, features, status = env.step(env.scripted_actions())
        assert len(obs) == 8 * 2 * OBS_SIZE
        assert all(math.isfinite(x) for x in obs)
    assert len(env.species_names)==40
    data = env.snapshot(3)
    fork.restore(data)
    assert env.hash(3) == fork.hash()
    for i in range(50):
        actions = list(env.scripted_actions())
        env.step(actions)
        fork.step(actions[30:40])
        assert env.hash(3) == fork.hash()
    old = fork.hash()
    try:
        fork.restore(data[:-1])
        raise AssertionError('Accepted truncated snapshot')
    except ValueError:
        pass
    assert old == fork.hash()
    try:
        fork.observe(player=2)
        raise AssertionError('Accepted invalid player')
    except ValueError:
        pass
    assert quantize((2., -.5), (0., 1.), 3) == [1024, -512, 0, 1024, 3]
    env.reset(0, 77, 2, species=(12,27), arena=1)
    for _ in range(100):
        env.step(env.scripted_actions())
    assert env.hash(0)==0x78b43d0560ba4a49
    print(f"Python golden {env.hash(0):016x}")
fork.close()  # Idempotent.
try:
    fork.observe()
    raise AssertionError('Use after close allowed')
except ValueError:
    pass
print('Python FFI: batch/scalar parity, golden hash, forks, validation and close passed')
