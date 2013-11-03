distance - Levenshtein and Hamming distance computation
=======================================================

This package provides facilities for computing Levenshtein and Hamming distance between arbitrary Python objects. It is only available for Python 3.3+.


Installation
------------

This is a C extension, so you need a C compiler available on your computer: typically Microsoft Visual C++ 2010 on Windows, and GCC on Mac and Linux. Python development files are also necessary to compile the package. On a Debian-like system, you can get all of these with:

	$ apt-get install gcc python3.3-dev

Then you can do:

	$ python3.3 setup.py install


Usage
-----

Fist import the module:

	>>> import distance

Two functions are provided: `levenshtein` and `hamming`. They both take two arguments, which are the objects to compare. Those objects can be of any type, as long as they support the sequence protocol: unicode strings, byte strings, lists, and tuples are ok. In case the objects provided are lists or tuples, they also should contain comparable objects.

Typical use case is to compare single words for similarity, as in spelling correction softwares:

	>>> distance.levenshtein("lenvestein", "levenshtein")
	3
	>>> distance.hamming("hamming", "hamning")
	1

Comparing lists of strings can also be useful for computing similarities between sentences, paragraphs, etc., in articles or books, as for plagiarism recognition:

	>>> sent1 = ['the', 'quick', 'brown', 'fox', 'jumps', 'over', 'the', 'lazy', 'dog']
	>>> sent2 = ['the', 'lazy', 'fox', 'jumps', 'over', 'the', 'crazy', 'dog']
	>>> distance.levenshtein(sent1, sent2)
	3

The above of course also works with numbers, etc.:

	>>> distance.levenshtein([1,2,3], [1,3,2])
	2


Implementation details
----------------------

Unicode strings are handled separately from the other sequence objects, in an efficient manner. Computing similarities between lists, tuples, and byte strings is likely to be slower, in particular for byte objects, which are internally converted to tuples.
