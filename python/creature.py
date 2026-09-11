"""Dependency-free, typed ctypes bridge. Batch layout is [arena, player, ...]."""
from __future__ import annotations
import ctypes as C
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
ARENA_COUNT = 6
OBS_SIZE = 3628
SELF_SIZE, ENTITY_COUNT, ENTITY_SIZE, MOVE_SIZE = 66, 69, 44, 48
FEATURE_SIZE = 10
SLICES = dict(self=slice(0,66), entities=slice(66,3102), moves=slice(3102,3342),
              announced=slice(3342,3390), history=slice(3390,3582), global_=slice(3582,3622), mask=slice(3622,3628))
I = C.c_int32
F = C.c_float
P = C.c_void_p


def library():
    suffix = 'dylib' if sys.platform == 'darwin' else 'dll' if sys.platform == 'win32' else 'so'
    path = Path(os.environ.get('CREATURE_LIB', ROOT / 'build' / f'libcreature.{suffix}'))
    lib = C.CDLL(str(path))
    if lib.cr_version() != 10 or lib.cr_observation_version() != 8:
        raise RuntimeError("Incompatible simulation / observation schema")
    signatures = {
        'cr_development': ([P,I,I,I,I,I], I),
        'cr_version': ([], C.c_uint32), 'cr_content_hash': ([], C.c_uint32),
        'cr_observation_version': ([], C.c_uint32), 'cr_species_count': ([], I),
        'cr_species_name': ([I], C.c_char_p),
        'cr_arena_count': ([], I), 'cr_arena_name': ([I], C.c_char_p),
        'cr_reset_match': ([P, C.c_uint32, I, I, I, I], I), 'cr_observation_size': ([], I),
        'cr_create': ([C.c_uint32, I], P), 'cr_destroy': ([P], None),
        'cr_reset': ([P, C.c_uint32, I], I),
        'cr_step': ([P, C.POINTER(I), C.POINTER(I), C.POINTER(I)], I),
        'cr_observe': ([P, I, C.POINTER(F), C.c_size_t], I),
        'cr_scripted': ([P, I, C.POINTER(I)], I),
        'cr_command': ([P, I, I], I),
        'cr_snapshot': ([P, C.POINTER(C.c_uint8), C.c_size_t], C.c_size_t),
        'cr_restore': ([P, C.POINTER(C.c_uint8), C.c_size_t], I),
        'cr_hash': ([P], C.c_uint64),
        'cr_batch_step': ([C.POINTER(P), C.c_size_t, C.POINTER(I), C.POINTER(F),
                           C.POINTER(I), C.POINTER(I)], I),
    }
    for name, (args, result) in signatures.items():
        fn = getattr(lib, name)
        fn.argtypes, fn.restype = args, result
    if lib.cr_version() != 10 or lib.cr_observation_version() != 8 or lib.cr_observation_size() != OBS_SIZE:
        raise RuntimeError('Incompatible simulation / observation schema')
    return lib


def quantize(move=(0., 0.), aim=(1., 0.), ability=0):
    """Float policy boundary -> canonical integer replay actions. Reject NaN/Inf."""
    import math
    values = (*move, *aim)
    if len(values) != 4 or not all(math.isfinite(float(x)) for x in values):
        raise ValueError('Expected four finite movement/aim values')
    if not isinstance(ability, int) or not 0 <= ability <= 5:
        raise ValueError('Ability must be an integer 0..5')
    # Explicit ties away from zero (do not depend on Python bankers rounding).
    def q(x):
        v = max(-1., min(1., float(x))) * 1024
        return int(math.copysign(math.floor(abs(v) + .5), v))
    return [*(q(x) for x in values), ability]


class Batch:
    """Independent worlds; no global RNG and no implicit episode reset.

    step returns reusable ctypes buffers: observations [N,2,3628], reward FEATURES
    [N,2,10], status [N,4] = terminated,truncated,winner,actual_physics_ticks.
    Copy buffers before the next step when retaining trajectories.
    One batch per worker; do not concurrently operate on the same handle.
    """
    def __init__(self, size=1, seed=1, weather=0, species=(0,1), arena=0):
        if not 1 <= size <= 65536:
            raise ValueError('size must be 1..65536')
        self.lib = library()
        self.size = size
        self.handles = (P * size)()
        try:
            for i in range(size):
                self.handles[i] = self.lib.cr_create(seed + i, weather)
                if not self.handles[i]:
                    raise MemoryError('Could not allocate world')
                self.reset(i, seed+i, weather, species, arena)
        except Exception:
            self.close()
            raise
        self.actions = (I * (size * 10))()
        self.observations = (F * (size * 2 * OBS_SIZE))()
        self.features = (I * (size * 2 * FEATURE_SIZE))()
        self.status = (I * (size * 4))()

    def _handle(self, index):
        if not 0 <= index < self.size or not self.handles[index]:
            raise ValueError('Invalid arena index or closed batch')
        return self.handles[index]

    def reset(self, index, seed, weather=0, species=(0,1), arena=0):
        if len(species)!=2 or self.lib.cr_reset_match(self._handle(index),seed,weather,*species,arena):
            raise ValueError('Invalid species, weather, or arena')

    def development(self, index, player, arts=31, pace=100, capacity=1000, recovery=100):
        """Set observable developmental limits immediately after reset (before stepping)."""
        if self.lib.cr_development(self._handle(index), player, arts, pace, capacity, recovery):
            raise ValueError('Invalid development limits or world already running')

    @property
    def arena_names(self):
        return [self.lib.cr_arena_name(i).decode() for i in range(self.lib.cr_arena_count())]

    @property
    def species_names(self):
        return [self.lib.cr_species_name(i).decode() for i in range(self.lib.cr_species_count())]

    def observe(self, index=0, player=0):
        out = (F * OBS_SIZE)()
        if self.lib.cr_observe(self._handle(index), player, out, OBS_SIZE):
            raise ValueError('Invalid player')
        return out

    def scripted_actions(self):
        for i in range(self.size):
            for p in range(2):
                out = C.cast(C.byref(self.actions, (i * 10 + p * 5) * C.sizeof(I)), C.POINTER(I))
                self.lib.cr_scripted(self._handle(i), p, out)
        return self.actions

    def step(self, actions=None):
        self._handle(0)
        if actions is not None:
            if len(actions) != self.size * 10:
                raise ValueError('Expected N * 2 * 5 flattened integer actions')
            if any(not isinstance(x, int) or not -(2**31) <= x < 2**31 for x in actions):
                raise ValueError('Actions must be signed 32-bit integers')
            self.actions[:] = actions
        code = self.lib.cr_batch_step(self.handles, self.size, self.actions,
                                     self.observations, self.features, self.status)
        if code:
            raise RuntimeError('Invalid batch buffers or closed world')
        return self.observations, self.features, self.status

    def command(self, index, player, guidance):
        if self.lib.cr_command(self._handle(index), player, guidance):
            raise ValueError('Invalid player or guidance')

    def snapshot(self, index=0):
        handle = self._handle(index)
        size = self.lib.cr_snapshot(handle, None, 0)
        if not size:
            raise RuntimeError('Snapshot allocation failed')
        out = (C.c_uint8 * size)()
        if self.lib.cr_snapshot(handle, out, size) != size:
            raise RuntimeError('Snapshot failed')
        return bytes(out)

    def restore(self, data, index=0):
        buf = (C.c_uint8 * len(data)).from_buffer_copy(data)
        if self.lib.cr_restore(self._handle(index), buf, len(data)):
            raise ValueError('Invalid, corrupt or incompatible snapshot')

    def hash(self, index=0):
        return self.lib.cr_hash(self._handle(index))

    def close(self):
        if hasattr(self, 'handles'):
            for i in range(self.size):
                if self.handles[i]:
                    self.lib.cr_destroy(self.handles[i])
                    self.handles[i] = None

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    def __del__(self):
        self.close()
