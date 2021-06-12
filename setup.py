import os
import pathlib
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.build_py import build_py as _build_py

class CMakeExtension(Extension):
    def __init__(self, name):
        super().__init__(name, sources=[])

class build_ext(_build_ext):
    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)

    def build_cmake(self, ext):
        cwd = pathlib.Path().absolute()
        build_dir = pathlib.Path('build').absolute()
        build_dir.mkdir(exist_ok=True)
        os.chdir(build_dir)
        if not self.dry_run:
            self.spawn(['cmake', '..'])
            if self.debug:
                self.spawn(['cmake', '--build', '.', '--config', 'Debug', '--target', 'install'])
            else:
                self.spawn(['cmake', '--build', '.', '--config', 'Release', '--target', 'install'])
        os.chdir(cwd)

class build_py(_build_py):
    def run(self):
        self.run_command('build_ext')
        return super().run()

setup(
    name="ndi-python",
    version="4.6.2",
    description="Wrapper package for NDI SDK python bindings.",
    author="Naoto Kondo <cgigcp3yqt@gmail.com>",
    license="MIT",
    ext_modules=[CMakeExtension('')],
    cmdclass={
        'build_py': build_py,
        'build_ext': build_ext},
    packages=[''],
    package_data={'':['*.so', '*.pyd', '*.dll', '*.dylib', '*.txt']},
    include_package_data=False,
    python_requires='>=3.4',
    zip_safe=False
)