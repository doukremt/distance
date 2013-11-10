/*
Distance - Utilities for comparing sequences
Copyright (C) 2013 Michaël Meyer

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "Python.h"


#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

PyDoc_STRVAR(hamming_doc,
"hamming(seq1, seq2, normalized=False) -> hamming distance\n\
\n\
Compute the Hamming distance between the two sequences `seq1` and `seq2`.\n\
The Hamming distance is the number of differing items in two ordered sequences\n\
of the same length. If the sequences submitted do not have the same length,\n\
an error will be raised.\n\
\n\
If `normalized` evaluates to `False`, the return value will be an integer between\n\
0 and the length of the sequences provided, edge values included; otherwise, it\n\
will be a float between 0 and 1 included, where 0 means equal, and 1 totally different.\n\
Normalized hamming distance is computed as:\n\
\n\
    hamming_dist / len(seq1)\
");

PyDoc_STRVAR(levenshtein_doc,
"levenshtein(seq1, seq2, normalized=False) -> levenshtein distance\n\
\n\
Compute the Levenshtein distance between the two sequences `seq1` and `seq2`.\n\
The Levenshtein distance is the minimum number of edit operations necessary for\n\
transforming one sequence into the other. The edit operations allowed are:\n\
\n\
    * deletion:     ABC -> BC, AC, AB\n\
    * insertion:    ABC -> ABCD, EABC, AEBC..\n\
    * substitution: ABC -> ABE, ADC, FBC..\n\
\n\
If `normalized` evaluates to `False`, the return value will be an integer between\n\
0 and the length of the sequences provided, edge values included; otherwise, it\n\
will be a float between 0 and 1 included, where 0 means equal, and 1 totally different.\n\
Normalized levenshtein distance is computed as:\n\
\n\
    lev_dist / max(len(seq1), len(seq2))\
");

PyDoc_STRVAR(quick_levenshtein_doc,
"quick_levenshtein(str1, str2) -> levenshtein distance\n\
\n\
Compute the levenshtein distance between the two strings `str1` and `str2` up\n\
to a maximum of 2 included, and return it. If the edit distance between the two\n\
strings is higher than that, -1 is returned.\n\
\n\
This can be a lot faster than `levenshtein`.\n\
\n\
The algorithm comes from `http://writingarchives.sakura.ne.jp/fastcomp`. The python\n\
code is the original one, and it has been rewritten in C for a large performance gain.\
");

PyDoc_STRVAR(iquick_levenshtein_doc,
"iquick_levenshtein(str1, strs) -> (distance, str2)..\n\
\n\
Return an iterator over all the strings in `strs` which distance from `str1`\n\
is lower or equal to 2. The strings which distance from the reference string\n\
is higher than that are dropped.\n\
\n\
    `str1`: the reference unicode string.\n\
    `strs`: an iterable of unicode strings (can be a generator).\n\
\n\
The return value is a series of pairs (distance, string).\n\
\n\
This is intended to be used to filter from a long list of strings the ones that\n\
are unlikely to be good spelling suggestions for the reference string (distance 2\n\
being considered a high enough value in most cases).\n\
\n\
This is faster than `levensthein` by an order of magnitude, so use this if you're\n\
only interested in strings which are below distance 2 from the reference string.\n\
\n\
You might want to call `sorted()` on the iterator to get the results in a\n\
significant order:\n\
\n\
    >>> g = iquick_levenshtein(\"foo\", [\"fo\", \"bar\", \"foob\", \"foo\", \"foobaz\"])\n\
    >>> sorted(g)\n\
    [(0, 'foo'), (1, 'fo'), (1, 'foob')]\
");


static Py_ssize_t
uni_hamming(PyObject *str1, PyObject *str2, Py_ssize_t len1, Py_ssize_t len2)
{
	Py_ssize_t i, dist = 0;
	
	void *ptr1 = PyUnicode_DATA(str1);
	void *ptr2 = PyUnicode_DATA(str2);
	int kind1 = PyUnicode_KIND(str1);
	int kind2 = PyUnicode_KIND(str2);
    
	for (i = 0; i < len1; i++) {
		if (PyUnicode_READ(kind1, ptr1, i) != PyUnicode_READ(kind2, ptr2, i))
			dist++;
	}

	return dist;
}


static Py_ssize_t
rich_hamming(PyObject *seq1, PyObject *seq2, Py_ssize_t len1, Py_ssize_t len2)
{
	Py_ssize_t i, comp, dist = 0;
	
	for (i = 0; i < len1; i++) {
		comp = PyObject_RichCompareBool(
			PySequence_Fast_GET_ITEM(seq1, i),
			PySequence_Fast_GET_ITEM(seq2, i),
			Py_EQ);
		if (comp == -1) {
			Py_DECREF(seq1);
			Py_DECREF(seq2);
			PyErr_SetString(PyExc_RuntimeError, "failed to compare objects");
			return -1;
		}
		if (!comp)
			dist++;
	}
	
	Py_DECREF(seq1);
	Py_DECREF(seq2);

	return dist;
}


static PyObject *
hamming(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *arg1, *arg2;
	Py_ssize_t len1, len2, dist;
	int do_normalize = 0;
	static char *keywords[] = {"arg1", "arg2", "normalized", NULL};
	
	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
		"OO|p:hamming", keywords, &arg1, &arg2, &do_normalize))
		return NULL;
	
	if (PyUnicode_Check(arg1) && PyUnicode_Check(arg2)) {
	
		if (PyUnicode_READY(arg1) == -1)
			return NULL;
		if (PyUnicode_READY(arg2) == -1)
			return NULL;
		len1 = PyUnicode_GET_LENGTH(arg1);
		len2 = PyUnicode_GET_LENGTH(arg2);
		if (len1 != len2) {
			PyErr_SetString(PyExc_ValueError, "expected two objects of the same length");
			return NULL;
		}
		dist = uni_hamming(arg1, arg2, len1, len2);
	}
	
	else if (PySequence_Check(arg1) && PySequence_Check(arg2)) {
	
		PyObject *seq1 = PySequence_Fast(arg1, "");
		PyObject *seq2 = PySequence_Fast(arg2, "");
	
		if (seq1 == NULL || seq2 == NULL) {
			Py_XDECREF(seq1);
			Py_XDECREF(seq2);
			PyErr_SetString(PyExc_RuntimeError, "failed to get objects as tuples");
			return NULL;
		}
	
		len1 = PySequence_Fast_GET_SIZE(seq1);
		len2 = PySequence_Fast_GET_SIZE(seq2);
		if (len1 == -1 || len2 == -1) {
			Py_DECREF(seq1);
			Py_DECREF(seq2);
			PyErr_SetString(PyExc_RuntimeError, "failed to get len of objects");
			return NULL;
		}
		if (len1 != len2) {
			Py_DECREF(seq1);
			Py_DECREF(seq2);
			PyErr_SetString(PyExc_ValueError, "expected two objects of the same len");
			return NULL;
		}
		dist = rich_hamming(seq1, seq2, len1, len2);
	}
	
	else {
		PyErr_SetString(PyExc_TypeError, "expected two sequence objects");
		return NULL;
	}
	
	if (dist == -1)
		return NULL;
	
	if (do_normalize) {
		if (len1 == 0 && len2 == 0)
			return Py_BuildValue("f", 0.0);  /* identical */
		double normalized = dist / (double)len1;
		return Py_BuildValue("d", normalized);
	}
	return Py_BuildValue("n", dist);
}


static Py_ssize_t
uni_levenshtein(PyObject *str1, PyObject *str2, Py_ssize_t len1, Py_ssize_t len2)
{
	Py_ssize_t x, y, last, old;
	Py_ssize_t *column;
	int kind1, kind2;
	void *ptr1, *ptr2;
	unsigned short cost;
	
	if (len1 == 0)
		return len2;
	if (len2 == 0)
		return len1;

	if (len1 < len2) {
		PyObject *temp_str = str1;
		Py_ssize_t temp_len = len1;
		str1 = str2;
		len1 = len2;
		str2 = temp_str;
		len2 = temp_len;
	}

	ptr1 = PyUnicode_DATA(str1);
	ptr2 = PyUnicode_DATA(str2);
	kind1 = PyUnicode_KIND(str1);
	kind2 = PyUnicode_KIND(str2);
	
	column = (Py_ssize_t*) malloc((len1 + 1) * sizeof(Py_ssize_t));
	if (column == NULL) {
		Py_DECREF(str1);
		Py_DECREF(str2);
		PyErr_SetString(PyExc_RuntimeError, "no memory");
		return -1;
	}

	for (y = 1; y <= len1; y++)
		column[y] = y;
	for (x = 1; x <= len2; x++) {
		column[0] = x;
		for (y = 1, last = x - 1; y <= len1; y++) {
			old = column[y];
			cost = (PyUnicode_READ(kind1, ptr1, y - 1) != PyUnicode_READ(kind2, ptr2, x - 1));
			column[y] = MIN3(column[y] + 1, column[y - 1] + 1, last + cost);
			last = old;
		}
	}

	free(column);
	return column[len1];
}


static Py_ssize_t
rich_levenshtein(PyObject *seq1, PyObject *seq2, Py_ssize_t len1, Py_ssize_t len2)
{
	Py_ssize_t *column;
	Py_ssize_t x, y, last, old;
	int comp;
	
	if (len1 == 0 || len2 == 0) {
		Py_DECREF(seq1);
		Py_DECREF(seq2);
		if (len1 == 0)
			return len2;
		return len1;
	}
	
	if (len1 < len2) {
		Py_ssize_t len1b = len1;
		PyObject *seq1b = seq1;
		len1 = len2;
		seq1 = seq2;
		len2 = len1b;
		seq2 = seq1b;
	}
		
	column = (Py_ssize_t*) malloc((len1 + 1) * sizeof(Py_ssize_t));
	if (column == NULL) {
		Py_DECREF(seq1);
		Py_DECREF(seq2);
		PyErr_SetString(PyExc_RuntimeError, "no memory");
		return -1;
	}

	for (y = 1; y <= len1; y++)
		column[y] = y;
	for (x = 1; x <= len2; x++) {
		column[0] = x;
		for (y = 1, last = x - 1; y <= len1; y++) {
			old = column[y];
			comp = PyObject_RichCompareBool(
				PySequence_Fast_GET_ITEM(seq1, y - 1),
				PySequence_Fast_GET_ITEM(seq2, x - 1),
				Py_EQ);
			if (comp == -1) {
				Py_DECREF(seq1);
				Py_DECREF(seq2);
				free(column);
				PyErr_SetString(PyExc_RuntimeError, "failed to compare objects");
				return -1;
			}
			column[y] = MIN3(column[y] + 1, column[y - 1] + 1, last + (comp != 1));
			last = old;
		}
	}

	Py_DECREF(seq1);
	Py_DECREF(seq2);
	free(column);
	
	return column[len1];
}


static PyObject *
levenshtein(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *arg1, *arg2;
	Py_ssize_t len1, len2, dist;
	int do_normalize = 0;
	static char *keywords[] = {"arg1", "arg2", "normalized", NULL};
	
	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
		"OO|p:levenshtein", keywords, &arg1, &arg2, &do_normalize))
		return NULL;

	if (PyUnicode_Check(arg1) && PyUnicode_Check(arg2)) {
	
		if (PyUnicode_READY(arg1) != 0)
			return NULL;
		if (PyUnicode_READY(arg2) != 0)
			return NULL;
		len1 = PyUnicode_GET_LENGTH(arg1);
		len2 = PyUnicode_GET_LENGTH(arg2);

		dist = uni_levenshtein(arg1, arg2, len1, len2);
	}

	else if (PySequence_Check(arg1) && PySequence_Check(arg2)) {
	
		PyObject *seq1 = PySequence_Fast(arg1, "");
		PyObject *seq2 = PySequence_Fast(arg2, "");
	
		if (seq1 == NULL || seq2 == NULL) {
			Py_XDECREF(seq1);
			Py_XDECREF(seq2);
			PyErr_SetString(PyExc_RuntimeError, "failed to get objects as tuples");
			return NULL;
		}
	
		len1 = PySequence_Fast_GET_SIZE(seq1);
		len2 = PySequence_Fast_GET_SIZE(seq2);
		if (len1 == -1 || len2 == -1) {
			Py_DECREF(seq1);
			Py_DECREF(seq2);
			PyErr_SetString(PyExc_RuntimeError, "failed to get len of objects");
			return NULL;
		}
	
		dist = rich_levenshtein(arg1, arg2, len1, len2);
	}
	
	else {
		PyErr_SetString(PyExc_TypeError, "expected two sequence objects");
		return NULL;
	}

	if (dist == -1)
		return NULL;
	
	if (do_normalize) {
		if (len1 == 0 && len2 == 0)
			return Py_BuildValue("f", 0.0);
		double max_len = (len1 > len2 ? len1 : len2);
		return Py_BuildValue("d", dist / max_len);
	}
	return Py_BuildValue("n", dist);
	
}


/* Quick levenshtein */


static short
uni_quick_levenshtein(void *ptr1, void *ptr2, int kind1, int kind2, Py_ssize_t len1, Py_ssize_t len2)
{
	char *models[3];
	short m, cnt, res = 3;
	Py_ssize_t i, j, c;
	
	Py_ssize_t ldiff = len1 - len2;

	/* i, d, r = insert, delete, replace */
	switch (ldiff) {
		case 0:
			models[2] = "id";
			models[1] = "di";
			models[0] = "rr";
			m = 2;
			break;
		case 1:
			models[1] = "dr";
			models[0] = "rd";
			m = 1;
			break;
		case 2:
			models[0] = "dd";
			m = 0;
			break;
		default:
			return -1;
	}

	for (; m >= 0; m--) {
	
		i = j = c = 0;
		
		while (i < len1 && j < len2)
		{
			if (PyUnicode_READ(kind1, ptr1, i) != PyUnicode_READ(kind2, ptr2, j)) {
				c++;
				if (c > 2)
					break;
				
				if (models[m][c - 1] == 'd')
					i++;
				else if (models[m][c - 1] == 'i')
					j++;
				else {
					i++;
					j++;
				}
			}
			else {
				i++;
				j++;
			}
		}
		
		if (c > 2)
			continue;

		else if (i < len1) {
			if (c == 1)
				cnt = (models[m][1] == 'd');
			else
				cnt = (models[m][0] == 'd') + (models[m][1] == 'd');
			if (len1 - i <= cnt) {
				c = c + (len1 - i);
			}
			else
				continue;
		}
		else if (j < len2) {
			if (len2 - j <= (models[m][c] == 'i'))
				c = c + (len2 - j);
			else
				continue;
		}
		if (c < res) {
			res = c;
		}
	}

	if (res == 3)
		res = -1;
		
	return res;
}

static PyObject *
quick_levenshtein(PyObject *self, PyObject *args)
{
	PyObject *arg1, *arg2;
	Py_ssize_t len1, len2;
	short dist;

	if (!PyArg_ParseTuple(args, "UU:quick_levenshtein", &arg1, &arg2))
		return NULL;

	if (PyUnicode_READY(arg1) != 0)
		return NULL;
	if (PyUnicode_READY(arg2) != 0)
		return NULL;

	len1 = PyUnicode_GET_LENGTH(arg1);
	len2 = PyUnicode_GET_LENGTH(arg2);
	void *ptr1 = PyUnicode_DATA(arg1);
	void *ptr2 = PyUnicode_DATA(arg2);
	int kind1 = PyUnicode_KIND(arg1);
	int kind2 = PyUnicode_KIND(arg2);
	
	if (len1 > len2)
		dist = uni_quick_levenshtein(ptr1, ptr2, kind1, kind2, len1, len2);
	else
		dist = uni_quick_levenshtein(ptr2, ptr1, kind2, kind1, len2, len1);
	
	return Py_BuildValue("h", dist);	
}


typedef struct {
    PyObject_HEAD
    PyObject *itor;
    PyObject *str1;
    void *ptr1;
    int kind1;
    Py_ssize_t len1;
} ItorState;


static void itor_dealloc(ItorState *state)
{
	Py_DECREF(state->str1);
	Py_XDECREF(state->itor);
	Py_TYPE(state)->tp_free(state);
}


static PyObject * iquick_levenshtein(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	PyObject *arg1, *arg2, *itor;

	if (!PyArg_ParseTuple(args, "UO:iquick_levenshtein", &arg1, &arg2))
		return NULL;

	if (PyUnicode_READY(arg1) != 0)
		return NULL;
	if ((itor = PyObject_GetIter(arg2)) == NULL) {
		PyErr_SetString(PyExc_ValueError, "expected an iterable");
		return NULL;
	}

	ItorState *state = (ItorState *)type->tp_alloc(type, 0);
	if (state == NULL) {
		Py_DECREF(itor);
		return NULL;
	}
    
	state->itor = itor;
	Py_INCREF(arg1);
	state->str1 = arg1;
	state->len1 = PyUnicode_GET_LENGTH(arg1);
	state->ptr1 = PyUnicode_DATA(arg1);
	state->kind1 = PyUnicode_KIND(arg1);
	  
	return (PyObject *)state;
}


static PyObject * iquick_levenshtein_next(ItorState *state)
{
	PyObject *str2, *rv;
	Py_ssize_t len2;
	void *ptr2;
	int kind2;
	short dist = -1;
	
	str2 = PyIter_Next(state->itor);
	
	while (str2 != NULL) {
	
		if (!PyUnicode_Check(str2)) {
			Py_DECREF(str2);
			PyErr_SetString(PyExc_ValueError, "expected unicodes in iterable");
			return NULL;
		}
		if (PyUnicode_READY(str2) != 0) {
			Py_DECREF(str2);
			return NULL;
		}
	
		len2 = PyUnicode_GET_LENGTH(str2);
		ptr2 = PyUnicode_DATA(str2);
		kind2 = PyUnicode_KIND(str2);
		
		if (state->len1 > len2)
			dist = uni_quick_levenshtein(state->ptr1, ptr2, state->kind1, kind2, state->len1, len2);
		else
			dist = uni_quick_levenshtein(ptr2, state->ptr1, kind2, state->kind1, len2, state->len1);
		
		if (dist != -1) {
			rv = Py_BuildValue("(hO)", dist, str2);
			Py_DECREF(str2);
			return rv;
		}
		
		Py_DECREF(str2);
		str2 = PyIter_Next(state->itor);
	
	}

	return NULL;
}


PyTypeObject IQuickLevenshtein_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "distance.iquick_levenshtein", /* tp_name */
    sizeof(ItorState), /* tp_basicsize */
    0, /* tp_itemsize */
    (destructor)itor_dealloc, /* tp_dealloc */
    0, /* tp_print */
    0, /* tp_getattr */
    0, /* tp_setattr */
    0, /* tp_reserved */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_hash */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    iquick_levenshtein_doc, /* tp_doc */
    0, /* tp_traverse */
    0, /* tp_clear */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    PyObject_SelfIter, /* tp_iter */
    (iternextfunc)iquick_levenshtein_next, /* tp_iternext */
    0, /* tp_methods */
    0, /* tp_members */
    0, /* tp_getset */
    0, /* tp_base */
    0, /* tp_dict */
    0, /* tp_descr_get */
    0, /* tp_descr_set */
    0, /* tp_dictoffset */
    0, /* tp_init */
    PyType_GenericAlloc, /* tp_alloc */
    iquick_levenshtein, /* tp_new */
};


static PyMethodDef CDistanceMethods[] = {
	{"levenshtein", (PyCFunction)levenshtein, METH_VARARGS | METH_KEYWORDS, levenshtein_doc},
	{"hamming", (PyCFunction)hamming, METH_VARARGS | METH_KEYWORDS, hamming_doc},
	{"quick_levenshtein", quick_levenshtein, METH_VARARGS, quick_levenshtein_doc},
	{NULL, NULL, 0, NULL}
};


static struct PyModuleDef cdistancemodule = {
	PyModuleDef_HEAD_INIT, "cdistance", NULL, -1, CDistanceMethods
};


PyMODINIT_FUNC PyInit_cdistance(void)
{
	PyObject *module = PyModule_Create(&cdistancemodule);
	if (!module)
		return NULL;

	if (PyType_Ready(&IQuickLevenshtein_Type) != 0)
		return NULL;
	
	Py_INCREF((PyObject *)&IQuickLevenshtein_Type);
	PyModule_AddObject(module, "iquick_levenshtein", (PyObject *)&IQuickLevenshtein_Type);
	
	return module;
}
