#!/usr/bin/env python

from setuptools import setup
import version

setup(name='OutputCheck',
      version=version.get_git_version(),
      description="A tool for checking the output of console programs inspired by LLVM's FileCheck",
      author='Daniel Liew',
      author_email='delcypher@gmail.com',
      url='https://github.com/stp/OutputCheck',
      packages=['OutputCheck'],
      classifiers=[ 'Environment :: Console',
                    'Development Status :: 3 - Alpha',
                    'Intended Audience :: Developers',
                    'License :: OSI Approved :: BSD License',
                    'Natural Language :: English',
                    'Operating System :: OS Independent', # Not tested
                    'Programming Language :: Python :: 2.7',
                    'Programming Language :: Python :: 3',
                    'Topic :: Software Development :: Testing',
                    'Topic :: Text Processing',
                    'Topic :: Utilities'
                  ],
      long_description=open('pypi_description.rst','r').read(),
      entry_points = { 'console_scripts': ['OutputCheck=OutputCheck.Driver:scriptEntryPoint'] }
     )
