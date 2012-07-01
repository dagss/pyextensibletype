from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
import os

include_dirs = ['include', '../ulib/src/base', '../sphlib-3.0/c']

extensions = [
    Extension("extensibletype.extensibletype",
              [os.path.join("extensibletype", "extensibletype.pyx"),
               '../ulib/src/base/md5sum.c',
               '../ulib/src/base/hash.c',
               '../sphlib-3.0/c/md5.c'],
              include_dirs=include_dirs)]

setup(cmdclass={'build_ext': build_ext},
      ext_modules=extensions)
