import os
import sys

if os.name == 'nt' and sys.version_info.major >= 3 and sys.version_info.minor >= 8:
    os.add_dll_directory(os.path.dirname(__file__))

from .NDIlib import *
