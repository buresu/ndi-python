import os

if os.name == 'nt':
    os.add_dll_directory(os.path.dirname(__file__))

from .NDIlib import *
