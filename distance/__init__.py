"Utilities for comparing sequences"

try:
	from .cdistance import *
	from .distance import jaccard, sorensen
except ImportError:
	from .distance import *
