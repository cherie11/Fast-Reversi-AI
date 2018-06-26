
import os, sys

from distutils.core import setup, Extension
from distutils import sysconfig

cpp_args = ['-std=c++11','-mmacosx-version-min=10.7','-stdlib=libc++']


ext_modules = [
    Extension(
    'mcts',
        ['MCTS.cpp', 'binding.cpp'],
        include_dirs=['pybind11/include'],
    language='c++',
    extra_compile_args = cpp_args,
    ),
]

setup(
    name='mcts',
    version='0.0.1',
    author='wxw',
    author_email='wxw@zju.edu',
    description='MCTS',
    ext_modules=ext_modules,
)