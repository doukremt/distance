/* 
Distance - Levenshtein and Hamming distance computation
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

#define module_DOC "Levenshtein and Hamming distance computation"

#define hamming_DOC \
	"hamming(seq1, seq2) -> hamming distance\n" \
	"\n" \
	"Compute the Hamming distance between two sequences."

#define levenshtein_DOC \
	"levenshtein(seq1, seq2) -> levenshthein distance\n" \
	"\n" \
	"Compute the Levensthein distance between two sequences." \


static PyObject * uni_hamming(PyObject *token1, PyObject *token2)
{
	Py_ssize_t length1, length2;
	void *ptr1, *ptr2;
	int kind1, kind2;
	Py_ssize_t i, distance = 0;
	
	if (PyUnicode_READY(token1) == -1)
		return NULL;
	if (PyUnicode_READY(token2) == -1)
		return NULL;
	
	length1 = PyUnicode_GET_LENGTH(token1);
	length2 = PyUnicode_GET_LENGTH(token2);

	if (length1 != length2) {
		PyErr_SetString(PyExc_ValueError, "expected two objects of the same length");
		return NULL;
	}
	
	ptr1 = PyUnicode_DATA(token1);
	ptr2 = PyUnicode_DATA(token2);

	kind1 = PyUnicode_KIND(token1);
	kind2 = PyUnicode_KIND(token2);
    
	for (i = 0; i < length1; i++) {
		if (PyUnicode_READ(kind1, ptr1, i) != PyUnicode_READ(kind2, ptr2, i))
			distance++;
	}

	return Py_BuildValue("n", distance);
}


static PyObject * rich_hamming(PyObject *object1, PyObject *object2)
{
	PyObject *seq1, *seq2, *item1, *item2;
	Py_ssize_t length1, length2;
	Py_ssize_t i, comp, distance = 0;
	
	seq1 = PySequence_Fast(object1, "");
	seq2 = PySequence_Fast(object2, "");
	
	if (seq1 == NULL || seq2 == NULL) {
		Py_XDECREF(seq1);
		Py_XDECREF(seq2);
		PyErr_SetString(PyExc_RuntimeError, "failed to get objects as tuples");
		return NULL;
	}
	
	length1 = PySequence_Fast_GET_SIZE(seq1);
	length2 = PySequence_Fast_GET_SIZE(seq2);
	
	if (length1 == -1 || length2 == -1) {
		Py_DECREF(seq1);
		Py_DECREF(seq2);
		PyErr_SetString(PyExc_RuntimeError, "failed to get length of objects");
		return NULL;
	}

	if (length1 != length2) {
		Py_DECREF(seq1);
		Py_DECREF(seq2);
		PyErr_SetString(PyExc_ValueError, "expected two objects of the same length");
		return NULL;
	}
	
	for (i = 0; i < length1; i++) {
		item1 = PySequence_Fast_GET_ITEM(seq1, i);
		item2 = PySequence_Fast_GET_ITEM(seq2, i);
		comp = PyObject_RichCompareBool(item1, item2, Py_EQ);
		if (comp == -1) {
			Py_DECREF(seq1);
			Py_DECREF(seq2);
			PyErr_SetString(PyExc_RuntimeError, "failed to compare objects");
			return NULL;
		}
		if (!comp)
			distance++;
	}
	
	Py_DECREF(seq1);
	Py_DECREF(seq2);

	return Py_BuildValue("n", distance);
}


static PyObject * hamming(PyObject *self, PyObject *args)
{
	PyObject *arg1, *arg2;
	
	if (!PyArg_ParseTuple(args, "OO:hamming", &arg1, &arg2))
		return NULL;
	
	if (PyUnicode_Check(arg1) && PyUnicode_Check(arg2))
		return uni_hamming(arg1, arg2);
	else if (PySequence_Check(arg1) && PySequence_Check(arg2))
		return rich_hamming(arg1, arg2);

	PyErr_SetString(PyExc_TypeError, "expected two sequence objects");
	return NULL;
}


static PyObject * uni_levenshtein(PyObject *token1, PyObject *token2)
{
	Py_ssize_t length1, length2;
	void *ptr1, *ptr2;
	int kind1, kind2;
	Py_ssize_t x, y, lastdiag, olddiag;
	
	if (PyUnicode_READY(token1) != 0)
		return NULL;
	if (PyUnicode_READY(token2) != 0)
		return NULL;
	
	length1 = PyUnicode_GET_LENGTH(token1);
	length2 = PyUnicode_GET_LENGTH(token2);
	
	if (length1 == 0)
		return Py_BuildValue("n", length2);
	if (length2 == 0)
		return Py_BuildValue("n", length1);

	ptr1 = PyUnicode_DATA(token1);
	ptr2 = PyUnicode_DATA(token2);

	kind1 = PyUnicode_KIND(token1);
	kind2 = PyUnicode_KIND(token2);
	
	Py_ssize_t column[length1 + 1];

	for (y = 1; y <= length1; y++)
		column[y] = y;
	for (x = 1; x <= length2; x++) {
		column[0] = x;
		for (y = 1, lastdiag = x - 1; y <= length1; y++) {
			olddiag = column[y];
			column[y] = MIN3(column[y] + 1, column[y - 1] + 1, lastdiag + \
			(PyUnicode_READ(kind1, ptr1, y - 1) == PyUnicode_READ(kind2, ptr2, x - 1) ? 0 : 1));
			lastdiag = olddiag;
		}
	}

	return Py_BuildValue("n", column[length1]);
}


static PyObject * rich_levenshtein(PyObject *object1, PyObject *object2)
{
	PyObject *seq1, *seq2, *item1, *item2;
	Py_ssize_t length1, length2;
	Py_ssize_t x, y, lastdiag, olddiag;
	int comp;
	
	seq1 = PySequence_Fast(object1, "");
	seq2 = PySequence_Fast(object2, "");
	
	if (seq1 == NULL || seq2 == NULL) {
		Py_XDECREF(seq1);
		Py_XDECREF(seq2);
		PyErr_SetString(PyExc_RuntimeError, "failed to get objects as tuples");
		return NULL;
	}
	
	length1 = PySequence_Fast_GET_SIZE(seq1);
	length2 = PySequence_Fast_GET_SIZE(seq2);
	
	if (length1 == -1 || length2 == -1) {
		Py_DECREF(seq1);
		Py_DECREF(seq2);
		PyErr_SetString(PyExc_RuntimeError, "failed to get length of objects");
		return NULL;
	}
	
	Py_ssize_t column[length1];

	for (y = 1; y <= length1; y++)
		column[y] = y;
	for (x = 1; x <= length2; x++) {
		column[0] = x;
		for (y = 1, lastdiag = x - 1; y <= length1; y++) {
			olddiag = column[y];
			item1 = PySequence_Fast_GET_ITEM(seq1, y - 1);
			item2 = PySequence_Fast_GET_ITEM(seq2, x - 1);
			comp = PyObject_RichCompareBool(item1, item2, Py_EQ);
			if (comp == -1) {
				Py_DECREF(seq1);
				Py_DECREF(seq2);
				PyErr_SetString(PyExc_RuntimeError, "failed to compare objects");
				return NULL;
			}
			column[y] = MIN3(column[y] + 1, column[y - 1] + 1, lastdiag + (comp == 1 ? 0 : 1));
			lastdiag = olddiag;
		}
	}

	Py_DECREF(seq1);
	Py_DECREF(seq2);
	
	return Py_BuildValue("n", column[length1]);
}


static PyObject * levenshtein(PyObject *self, PyObject *args)
{
	PyObject *arg1, *arg2;
	
	if (!PyArg_ParseTuple(args, "OO:levenshtein", &arg1, &arg2))
		return NULL;
	
	if (PyUnicode_Check(arg1) && PyUnicode_Check(arg2))
		return uni_levenshtein(arg1, arg2);
	else if (PySequence_Check(arg1) && PySequence_Check(arg2))
		return rich_levenshtein(arg1, arg2);

	PyErr_SetString(PyExc_TypeError, "expected two sequence objects");
	return NULL;
}


static PyMethodDef DistanceMethods[] = {
	{"levenshtein", levenshtein, METH_VARARGS, levenshtein_DOC},
	{"hamming", hamming, METH_VARARGS, hamming_DOC},
	{NULL, NULL, 0, NULL}
};


static struct PyModuleDef distancemodule = {
	PyModuleDef_HEAD_INIT, "distance", module_DOC, -1, DistanceMethods
};


PyMODINIT_FUNC PyInit_distance(void)
{
	return PyModule_Create(&distancemodule);
}
