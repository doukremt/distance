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


def hamming(seq1, seq2, normalized=False):
	"""Compute the Hamming distance between the two sequences `seq1` and `seq2`.
	Both should have the same length.
	
	The return value is an integer between 0 and the length of the sequences provided,
	edge values included.
	"""
	if len(seq1) != len(seq2):
		raise ValueError("expected two strings of the same length")
	dist = sum(c1 != c2 for c1, c2 in zip(seq1, seq2))
	if normalized:
		return dist / float(len(seq1))
	return dist


def levenshtein(seq1, seq2, normalized=False):
	"""Compute the Levenshtein distance between the two sequences `seq1` and `seq2`.
	
	The return value is an integer between 0 and the longer sequence provided, edges included.
	"""
	len1, len2 = len(seq1), len(seq2)
	if len1 == 0:
		return len2
	if len2 == 0:
		return len1
	if len1 < len2:
		len1, len2 = len2, len1
		seq1, seq2 = seq2, seq1
	column = list(range(len2 + 1))
	for x in range(1, len1 + 1):
		column[0] = x
		last = x - 1
		for y in range(1, len2 + 1):
			old = column[y]
			cost = int(seq1[x - 1] != seq2[y - 1])
			column[y] = min(column[y] + 1, min(column[y - 1] + 1, last + cost))
			last = old
	if normalized:
		return column[len2] / float(max(len1, len2))
	return column[len2]


def jaccard(seq1, seq2):
	"""Compute the Jaccard distance between the two sequences `seq1` and `seq2`. They
	should contain hashable items, i. e.: not lists
	
	The return value is a float between 0 and 1. The lower the value, the closer the sequences.
	"""
	set1, set2 = set(seq1), set(seq2)
	return 1 - len(set1 & set2) / float(len(set1 | set2))


def sorensen(seq1, seq2):
	"""Compute the Sorensen distance between the two sequences `seq1` and `seq2`. They
	should contain hashable items, i. e.: not lists
	
	The return value is a float between 0 and 1. The lower the value, the closer the sequences.
	"""
	set1, set2 = set(seq1), set(seq2)
	return 1 - (2 * len(set1 & set2) / float(len(set1) + len(set2)))
