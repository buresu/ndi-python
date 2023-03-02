import os
import sys
import shutil
from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name):
        super().__init__(name, sources=[])


class CMakeBuild(build_ext):
    def run(self):
        # build and install
        cwd = os.getcwd()
        build_dir = os.path.join(cwd, 'build')
        os.makedirs(build_dir, exist_ok=True)
        os.chdir(build_dir)
        install_dir = os.path.join(build_dir, 'install')
        if not self.dry_run:
            cmake_args = ['cmake', '..',
                          '-DCMAKE_INSTALL_PREFIX=%s' % install_dir,
                          '-DPYTHON_EXECUTABLE=%s' % sys.executable]
            if "CMAKE_ARGS" in os.environ:
                cmake_args += [
                    item for item in os.environ["CMAKE_ARGS"].split(" ") if item]
            self.spawn(cmake_args)
            if self.debug:
                self.spawn(['cmake', '--build', '.', '--config',
                           'Debug', '--target', 'install'])
            else:
                self.spawn(['cmake', '--build', '.', '--config',
                           'Release', '--target', 'install'])
        os.chdir(cwd)
        # move
        for ext in self.extensions:
            dst_dir = os.path.dirname(self.get_ext_fullpath(ext.name))
            lib_dir = os.path.join(dst_dir, 'NDIlib')
            os.makedirs(lib_dir, exist_ok=True)
            files = os.listdir(install_dir)
            for filename in files:
                filepath = os.path.join(install_dir, filename)
                if os.path.isfile(filepath):
                    shutil.copy(filepath, lib_dir)


# read description
this_directory = Path(__file__).parent
long_description = (this_directory / "README.md").read_text()

# setup
setup(
    name='ndi-python',
    version='5.1.1.5',
    description='Wrapper package for NDI SDK python bindings.',
    long_description=long_description,
    long_description_content_type='text/markdown',
    author='Naoto Kondo <cgigcp3yqt@gmail.com>',
    url='https://github.com/buresu/ndi-python',
    license="MIT",
    python_requires='>=3.7',
    install_requires=['numpy'],
    ext_modules=[CMakeExtension('NDIlib')],
    cmdclass={'build_ext': CMakeBuild},
    packages=['NDIlib'],
    package_data={'NDIlib': ['*.so*', '*.pyd', '*.dll', '*.dylib', '*.txt']},
    zip_safe=False,
    keywords=['NDI', 'NewTek', 'Video Production'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: MacOS',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: C++',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3 :: Only',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: Implementation :: CPython',
        'Topic :: Software Development',
        'Topic :: Software Development :: Libraries',
        'Topic :: Multimedia',
        'Topic :: Multimedia :: Graphics',
        'Topic :: Multimedia :: Sound/Audio',
        'Topic :: Multimedia :: Video',
        'Topic :: System :: Networking',
    ],
)
