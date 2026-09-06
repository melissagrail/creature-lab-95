"""Dependency-free loader for the optional C++ inference library; no Torch import."""
import ctypes as C
import os
from pathlib import Path
import sys
from creature import ROOT, OBS_SIZE
F=C.c_float;I=C.c_int32
class NativeBrain:
    def __init__(self,path):
        suffix='dylib' if sys.platform=='darwin' else 'dll' if sys.platform=='win32' else 'so'
        filename='tinibrain.dll' if sys.platform=='win32' else f'libtinibrain.{suffix}'
        self.lib=C.CDLL(os.environ.get('TINIBRAIN_LIB',str(ROOT/'build'/filename)))
        self.lib.tb_create.argtypes=[C.c_char_p];self.lib.tb_create.restype=C.c_void_p
        self.lib.tb_destroy.argtypes=[C.c_void_p];self.lib.tb_destroy.restype=None
        self.lib.tb_forward.argtypes=[C.c_void_p,C.POINTER(F),C.POINTER(F),C.POINTER(F)];self.lib.tb_forward.restype=I
        self.lib.tb_action.argtypes=[C.c_void_p,C.POINTER(F),C.POINTER(F),C.POINTER(I)];self.lib.tb_action.restype=I
        self.handle=self.lib.tb_create(os.fsencode(path))
        if not self.handle:raise ValueError('Missing, corrupt or incompatible .tbrain model')
    def forward(self,obs,memory):
        if not self.handle or len(obs)!=OBS_SIZE or len(memory)!=96:raise ValueError('Invalid brain input or closed model')
        o=(F*OBS_SIZE)(*obs);h=(F*96)(*memory);out=(F*107)()
        if self.lib.tb_forward(self.handle,o,h,out):raise ValueError('Invalid brain input')
        return list(out)
    def action(self,obs,memory):
        if not self.handle or len(obs)!=OBS_SIZE or len(memory)!=96:raise ValueError('Invalid brain input or closed model')
        o=(F*OBS_SIZE)(*obs);h=(F*96)(*memory);a=(I*5)()
        if self.lib.tb_action(self.handle,o,h,a):raise ValueError('Invalid brain input')
        return list(a),list(h)
    def close(self):
        if getattr(self,'handle',None):self.lib.tb_destroy(self.handle);self.handle=None
    def __enter__(self):return self
    def __exit__(self,*_):self.close()
    def __del__(self):self.close()
