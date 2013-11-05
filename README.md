distance - Utilities for comparing sequences
============================================

This package provides helpers for computing similarities between arbitrary sequences. Included metrics are:

	* Levenshtein distance
	* Hamming distance
	* Jaccard distance
	* Sorensen distance

All distance computations are implemented in pure Python. Levenshtein and Hamming distances are also implemented in C.


Installation
------------

If you don't want to use the C extension, just unpack the archive and run:

	# python setup.py install

For the C extension to work, you need Python 3.3+, its headers files, and a C compiler (typically Microsoft Visual C++ 2010 on Windows, and GCC on Mac and Linux). On a Debian-like system, you can get all of these with:

	# apt-get install gcc python3.3-dev

Then you should type:

	# python3.3 setup.py install --with-c


Usage
-----

Fist import the module:

	>>> import distance

All functions provided take two arguments, which are the objects to compare. Arguments provided to `hamming` and `levenshtein` should support the sequence protocol: unicode strings, byte strings, lists, and tuples are ok. `jaccard` and `sorensen` use sets, so you can pass any iterable, at the condition that it is hashable.

Typical use case is to compare single words for similarity:

	>>> distance.levenshtein("lenvestein", "levenshtein")
	3
	>>> distance.hamming("hamming", "hamning")
	1


Comparing lists of strings can also be useful for computing similarities between sentences, paragraphs, etc.:

	>>> sent1 = ['the', 'quick', 'brown', 'fox', 'jumps', 'over', 'the', 'lazy', 'dog']
	>>> sent2 = ['the', 'lazy', 'fox', 'jumps', 'over', 'the', 'crazy', 'dog']
	>>> distance.levenshtein(sent1, sent2)
	3


To compare the results returned by the available metrics, a `normalize` keyword parameter can be supplied to `hamming` and `levenshtein`. If it evaluates to True, the return value will be a float between 0 and 1 inclusive, representing the similarity between the submitted sequences. 0 means identic, and 1 totally different:

	>>> distance.levenshtein("decide", "resize", normalized=True)
	0.5
	>>> distance.hamming("decide", "resize", normalized=True)
	0.5
	>>> distance.sorensen("decide", "resize")
	0.5555555555555556
	>>> distance.jaccard("decide", "resize")
	0.7142857142857143


Have fun!


Implementation details
----------------------

In the C implementation, unicode strings are handled separately from the other sequence objects, in an efficient manner. Computing similarities between lists, tuples, and byte strings is likely to be slower, in particular for byte objects, which are internally converted to tuples.

05/11/13: Added Sorensen and Jaccard metrics, fixed memory issue in Levenshtein.
