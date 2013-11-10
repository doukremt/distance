"Utilities for comparing sequences"

__all__ = ["hamming", "levenshtein", "jaccard", "sorensen", "quick_levenshtein",
	"iquick_levenshtein"]

try:
	from .cdistance import *
except ImportError:
	from .distance import *

from .distance import jaccard, sorensen
