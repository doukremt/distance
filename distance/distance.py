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

"Utilities for comparing sequences"


from array import array


def hamming(seq1, seq2, normalized=False):
	"""Compute the Hamming distance between the two sequences `seq1` and `seq2`.
	The Hamming distance is the number of differing items in two ordered sequences
	of the same length. If the sequences submitted do not have the same length,
	an error will be raised.
	
	If `normalized` evaluates to `False`, the return value will be an integer between
	0 and the length of the sequences provided, edge values included; otherwise, it
	will be a float between 0 and 1 included, where 0 means equal, and 1 totally different.
	Normalized hamming distance is computed as:
	
		hamming_dist / len(seq1)
	"""
	L = len(seq1)
	if L != len(seq2):
		raise ValueError("expected two strings of the same length")
	if L == 0:
		return 0.0 if normalized else 0  # equal
	dist = sum(c1 != c2 for c1, c2 in zip(seq1, seq2))
	if normalized:
		return dist / float(L)
	return dist


def levenshtein(seq1, seq2, normalized=False):
	"""Compute the Levenshtein distance between the two sequences `seq1` and `seq2`.
	The Levenshtein distance is the minimum number of edit operations necessary for
	transforming one sequence into the other. The edit operations allowed are:
	
		* deletion:     ABC -> BC, AC, AB
		* insertion:    ABC -> ABCD, EABC, AEBC..
		* substitution: ABC -> ABE, ADC, FBC..
	
	If `normalized` evaluates to `False`, the return value will be an integer between
	0 and the length of the sequences provided, edge values included; otherwise, it
	will be a float between 0 and 1 included, where 0 means equal, and 1 totally different.
	Normalized levenshtein distance is computed as:
	
		lev_dist / max(len(seq1), len(seq2))
	"""
	if seq1 == seq2:
		return 0.0 if normalized else 0
	len1, len2 = len(seq1), len(seq2)
	if len1 == 0:
		return 1.0 if normalized else len2
	if len2 == 0:
		return 1.0 if normalized else len1
	if len1 < len2:
		len1, len2 = len2, len1
		seq1, seq2 = seq2, seq1
	
	column = array('I', range(len1 + 1))
	for x in range(1, len1 + 1):
		column[0] = x
		last = x - 1
		for y in range(1, len2 + 1):
			old = column[y]
			cost = int(seq1[x - 1] != seq2[y - 1])
			column[y] = min(column[y] + 1, min(column[y - 1] + 1, last + cost))
			last = old
	if normalized:
		# already checked division by zero
		return column[len2] / float(max(len1, len2))
	return column[len2]


def jaccard(seq1, seq2):
	"""Compute the Jaccard distance between the two sequences `seq1` and `seq2`.
	They should contain hashable items.
	
	The return value is a float between 0 and 1, where 0 means equal, and 1 totally different.
	"""
	set1, set2 = set(seq1), set(seq2)
	return 1 - len(set1 & set2) / float(len(set1 | set2))


def sorensen(seq1, seq2):
	"""Compute the Sorensen distance between the two sequences `seq1` and `seq2`.
	They should contain hashable items.
	
	The return value is a float between 0 and 1, where 0 means equal, and 1 totally different.
	"""
	set1, set2 = set(seq1), set(seq2)
	return 1 - (2 * len(set1 & set2) / float(len(set1) + len(set2)))


def fast_comp(str1, str2, transpositions=False):
	"""Compute the distance between the two strings `str1` and `str2` up to a maximum
	of 2 included, and return it. If the edit distance between the two strings is higher
	than that, -1 is returned.
	
	`str1` and `str2` are expected to be unicode strings. If `transpositions` is `True`,
	transpositions will be taken into account when computing the distance between the
	submitted strings, e.g.:

		>>> fast_comp("abc", "bac", transpositions=False)
		2
		>>> fast_comp("abc", "bac", transpositions=True)
		1

	The algorithm comes from `http://writingarchives.sakura.ne.jp/fastcomp`. I've added
	transpositions support to the original code, and rewritten it in C.
	"""
	replace, insert, delete = "r", "i", "d"

	L1, L2  = len(str1), len(str2)
	if L1 < L2:
		L1, L2 = L2, L1
		str1, str2 = str2, str1

	ldiff = L1 - L2
	if ldiff == 0:
		models = (insert+delete, delete+insert, replace+replace)
	elif ldiff == 1:
		models = (delete+replace, replace+delete)
	elif ldiff == 2:
		models = (delete+delete,)
	else:
		return -1

	res = 3
	for model in models:
		i = j = c = 0
		while (i < L1) and (j < L2):
			if str1[i] != str2[j]:
				c = c+1
				if 2 < c:
					break
            
				if transpositions and ldiff != 2 \
            	and i < L1 - 1 and j < L2 - 1 \
            	and str1[i+1] == str2[j] and str1[i] == str2[j+1]:
					i, j = i+2, j+2
				else:
					cmd = model[c-1]
					if cmd == delete:
						i = i+1
					elif cmd == insert:
						j = j+1
					else:
						assert cmd == replace
						i,j = i+1, j+1
			else:
				i,j = i+1, j+1

		if 2 < c:
			continue
		elif i < L1:
			if L1-i <= model[c:].count(delete):
				c = c + (L1-i)
			else:
				continue
		elif j < L2:
			if L2-j <= model[c:].count(insert):
				c = c + (L2-j)
			else:
				continue

		if c < res:
			res = c

	if res == 3:
		res = -1
	return res


def ifast_comp(str1, strs, transpositions=False):
	"""Return an iterator over all the strings in `strs` which distance from `str1`
	is lower or equal to 2. The strings which distance from the reference string
	is higher than that are dropped.
	
		`str1`: the reference unicode string.
		`strs`: an iterable of unicode strings (can be a generator).
		`transpositions` has the same sense than in `fast_comp`.
	
	The return value is a series of pairs (distance, string).
	
	This is intended to be used to filter from a long list of strings the ones that
	are unlikely to be good spelling suggestions for the reference string (distance 2
	being considered a high enough value in most cases).
	
	This is faster than `levensthein` by an order of magnitude, so use this if you're
	only interested in strings which are below distance 2 from the reference string.
	
	You might want to call `sorted()` on the iterator to get the results in a
	significant order:
	
		>>> g = ifast_comp("foo", ["fo", "bar", "foob", "foo", "foobaz"])
		>>> sorted(g)
		[(0, 'foo'), (1, 'fo'), (1, 'foob')]
	"""
	for str2 in strs:
		dist = fast_comp(str1, str2, transpositions)
		if dist != -1:
			yield dist, str2


def lcsubstrings(seq1, seq2, positions=False):
	"""Find the longest common substring(s) in the sequences `seq1` and `seq2`.
	
	If positions evaluates to `True` only their position(s) will be returned,
	together with their length, in a tuple:
	
		(length, [(start pos in seq1, start pos in seq2)..])
	
	Otherwise, the substrings themselves will be returned, in a set.
	
	Example:
	
		>>> lcsubstrings("sedentar", "dentist")
		{'dent'}
		>>> lcsubstrings("sedentar", "dentist", positions=True)
		(4, [(2, 0)])
	"""
	L1, L2 = len(seq1), len(seq2)
	ms = []
	mlen = last = 0
	# should make sure to minimize array size
	column = array('I', range(L2))
	for i in range(L1):
		for j in range(L2):
			old = column[j]
			if seq1[i] == seq2[j]:
				if i == 0 or j == 0:
					column[j] = 1
				else:
					column[j] = last + 1
				if column[j] > mlen:
					mlen = column[j]
					ms = [(i, j)]
				elif column[j] == mlen:
					ms.append((i, j))
			else:
				column[j] = 0
			last = old
	if positions:
		return (mlen, list((i-mlen+1, j-mlen+1) for i, j in ms if ms))
	return set(seq1[i-mlen+1:i+1] for i, _ in ms if ms)
