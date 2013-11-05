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

#define HAMMING_DOC \
	"hamming(seq1, seq2) -> hamming distance\n"\
	"Compute the Hamming distance between the two sequences `seq1` and `seq2`.\n"\
	"Both should have the same length.\n"\
	"\n"\
	"The return value is an integer between 0 and the length of the sequences provided,\n"\
	"edges included."

#define LEVENSHTEIN_DOC \
	"levenshtein(seq1, seq2) -> levenshtein distance\n"\
	"Compute the Levenshtein distance between the two sequences `seq1` and `seq2`.\n" \
	"\n" \
	"The return value is an integer between 0 and the length of the longer sequence provided,\n"\
	"edges included."


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


static PyObject * hamming(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *arg1, *arg2;
	Py_ssize_t len1, len2, dist;
	int do_normalize = 0;
	static char *keywords[] = {"arg1", "arg2", "normalized", NULL};
	
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|p:hamming", keywords, &arg1, &arg2, &do_normalize))
		return NULL;
	
	if (PyUnicode_Check(arg1) && PyUnicode_Check(arg2)) {
		if (PyUnicode_READY(arg1) == -1)
			return NULL;
		if (PyUnicode_READY(arg2) == -1)
			return NULL;
		len1 = PyUnicode_GET_LENGTH(arg1);
		len2 = PyUnicode_GET_LENGTH(arg2);
		if (len1 != len2) {
			PyErr_SetString(PyExc_ValueError, "expected two objects of the same len");
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
	
	if (do_normalize && (len1 != 0 || len2 != 0)) {
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
		Py_DECREF(temp_str);
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
		PyObject *temp_seq = seq1;
		Py_ssize_t temp_len = len1;
		seq1 = seq2;
		len1 = len2;
		seq2 = temp_seq;
		len2 = temp_len;
		Py_DECREF(temp_seq);
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


static PyObject * levenshtein(PyObject *self, PyObject *args, PyObject *kwargs)
{
	PyObject *arg1, *arg2;
	Py_ssize_t len1, len2, dist;
	int do_normalize = 0;
	static char *keywords[] = {"arg1", "arg2", "normalized", NULL};
	
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|p:levenshtein", keywords, &arg1, &arg2, &do_normalize))
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
	
	if (do_normalize && (len1 != 0 || len2 != 0)) {
		double max_len = (len1 > len2 ? len1 : len2);
		double normalized = dist / max_len;
		return Py_BuildValue("d", normalized);
	}
	return Py_BuildValue("n", dist);
	
}


static PyMethodDef CDistanceMethods[] = {
	{"levenshtein", (PyCFunction)levenshtein, METH_VARARGS | METH_KEYWORDS, LEVENSHTEIN_DOC},
	{"hamming", (PyCFunction)hamming, METH_VARARGS | METH_KEYWORDS, HAMMING_DOC},
	{NULL, NULL, 0, NULL}
};


static struct PyModuleDef cdistancemodule = {
	PyModuleDef_HEAD_INIT, "cdistance", NULL, -1, CDistanceMethods
};


PyMODINIT_FUNC PyInit_cdistance(void)
{
	return PyModule_Create(&cdistancemodule);
}
