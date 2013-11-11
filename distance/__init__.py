"Utilities for comparing sequences"

__all__ = ["hamming", "levenshtein", "jaccard", "sorensen", "quick_levenshtein",
	"iquick_levenshtein", "fast_comp", "ifast_comp"]

try:
	from .cdistance import *
except ImportError:
	from .distance import *

from .distance import jaccard, sorensen

def quick_levenshtein(str1, str2):
	return fast_comp(str1, str2, transpositions=False)

def iquick_levenshtein(str1, strs):
	return ifast_comp(str1, str2, transpositions=False)
