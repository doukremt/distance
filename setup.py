# -*- coding: utf-8 -*- 

# Distance - Utilities for comparing sequences
# Copyright (C) 2013 Michaël Meyer

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


import os, sys
from distutils.core import setup, Extension

this_dir = os.path.dirname(os.path.abspath(__file__))
pkg_dir = os.path.join(this_dir, "distance")


def show_c_doc():
	exclude = {"jaccard", "sorensen"}
	import ast, _ast, re
	with open(os.path.join(pkg_dir, "distance.py")) as f:
		content = f.read()
	tree = ast.parse(content)
	for node in ast.iter_child_nodes(tree):
		if isinstance(node, _ast.FunctionDef) and not node.name in exclude:
			doc = ast.get_docstring(node)
			if not doc:
				continue
			defin = re.findall("def\s%s\s*(.+?)\s*:" % node.name, content)
			assert defin
			defin = node.name + defin[0] + 2 * '\\n\\\n'
			doc = doc.replace('\n', '\\n\\\n').replace('"', '\\"').replace(8 * ' ', 4 * ' ')
			doc = 'PyDoc_STRVAR(%s_doc,\n"%s%s\\\n");' % (node.name, defin, doc)
			sys.stderr.write(doc + 2 * '\n')


with open(os.path.join(this_dir, "README.md")) as f:
    long_description = f.read()

args = sys.argv[1:]

if "print-doc" in args:
	show_c_doc()
	sys.exit()

if "--with-c" in args:
	args.remove("--with-c")
	if sys.version_info[:2] < (3, 3):
		sys.stderr.write("Python 3.3+ is necessary for the C extension to work. " \
			"Your version is %s\n" % sys.version)
		sys.exit(1)
	ext_modules = [Extension('distance.cdistance', sources=["distance/distance.c"])]
else:
	sys.stderr.write("notice: no C support available\n")
	ext_modules = []

setup (
    name = 'Distance',
    version = '0.1.2',
    description = 'Utilities for computing similarities between sequences',
    long_description = long_description,
    author='Michaël Meyer',
    author_email='michaelnm.meyer@gmail.com',
    url='https://github.com/doukremt/distance',
    ext_modules = ext_modules,
    script_args = args,
    packages = ['distance'],
    classifiers=(
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'License :: OSI Approved :: GNU General Public License (GPL)',
        'Operating System :: OS Independent',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Programming Language :: C',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3.3',
    )
)
