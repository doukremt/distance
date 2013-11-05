"Utilities for comparing sequences"

__all__ = ["levenshtein", "hamming", "sorensen", "jaccard"]

try:
	from .cdistance import levenshtein, hamming
except ImportError:
	from .distance import levenshtein, hamming

from .distance import jaccard, sorensen
