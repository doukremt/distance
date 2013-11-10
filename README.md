distance - Utilities for comparing sequences
============================================

This package provides helpers for computing similarities between arbitrary sequences. Included metrics are:

	* Levenshtein distance (classic and faster version)
	* Hamming distance
	* Jaccard distance
	* Sorensen distance

All distance computations are implemented in pure Python. Levenshtein (classic and faster version) and Hamming distances are also implemented in C.


Installation
------------

If you don't want or need to use the C extension, just unpack the archive and run:

	# python setup.py install

For the C extension to work, you need Python 3.3+, its headers files, and a C compiler (typically Microsoft Visual C++ 2010 on Windows, and GCC on Mac and Linux). On a Debian-like system, you can get all of these with:

	# apt-get install gcc python3.3-dev

Then you should type:

	# python3.3 setup.py install --with-c

Note the use of the `--with-c` switch.


Usage
-----

Fist import the module:

	>>> import distance

All functions provided take two arguments, which are the objects to compare. Arguments provided to `hamming` and `levenshtein` can be unicode strings, byte strings, lists, or tuples. `jaccard` and `sorensen` use sets, so you can pass in any iterable, at the condition that it is hashable.

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

If a `normalized` keyword parameter is supplied to `hamming` or `levenshtein` and evaluates to True, the return value of these functions will be a float between 0 and 1 inclusive, where 0 means identic, and 1 totally different:

	>>> distance.levenshtein("decide", "resize", normalized=True)
	0.5
	>>> distance.hamming("decide", "resize", normalized=True)
	0.5

Jaccard and sorensen return a normalized value per default:

	>>> distance.sorensen("decide", "resize")
	0.5555555555555556
	>>> distance.jaccard("decide", "resize")
	0.7142857142857143

Finally, there is a `quick_levenshtein` function, which computes the levenshtein distance between two strings up to a value of 2 included, and is quite faster than the classic Levenshtein implementation. The python version comes from [here](http://writingarchives.sakura.ne.jp/fastcomp), and has been rewritten in C. Also, an iterator type `iquick_levenshtein` is provided, which comes handy to filter from a long list of strings the ones that resemble a given one:

	>>> g = iquick_levenshtein("foo", ["fo", "bar", "foob", "foo", "foobaz"])
	>>> sorted(g)
	[(0, 'foo'), (1, 'fo'), (1, 'foob')]


See the functions documentation (`help(funcname)`) for more details.

Have fun!


Implementation details
----------------------

In the C implementation, unicode strings are handled separately from the other sequence objects. Computing similarities between lists, tuples, and byte strings is likely to be slower.

05/11/13: Added Sorensen and Jaccard metrics, fixed memory issue in Levenshtein.
10/11/13: Added `quick_levenshtein` and `iquick_levenshtein`.
