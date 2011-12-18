#include <Python.h>
#define STORM_MODULE
#include "stormmodule.h"

#ifdef __cplusplus
extern "C" {
#endif

static PyObject *StormError;

/*
 * Manipulating MPQ archives
 */

static PyObject * Storm_SFileOpenArchive(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	char *name;
	int priority;
	int flags;
	bool result;

	if (!PyArg_ParseTuple(args, "sii:SFileOpenArchive", &name, &priority, &flags)) {
		return NULL;
	}
	result = SFileOpenArchive(name, priority, MPQ_OPEN_READ_ONLY, &mpq);

	if (!result) {
		PyErr_SetString(StormError, "Error opening archive");
		return NULL;
	}

	return Py_BuildValue("l", mpq);
}

static PyObject * Storm_SFileAddListFile(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	char *name;
	bool result;

	if (!PyArg_ParseTuple(args, "is:SFileAddListFile", &mpq, &name)) {
		return NULL;
	}
	result = SFileAddListFile(mpq, name);

	if (result != ERROR_SUCCESS) {
		PyErr_SetString(StormError, "Error adding listfile");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject * Storm_SFileFlushArchive(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	bool result;

	if (!PyArg_ParseTuple(args, "i:SFileFlushArchive", &mpq)) {
		return NULL;
	}
	result = SFileFlushArchive(mpq);

	if (!result) {
		PyErr_SetString(StormError, "Error flushing archive, archive may be corrupted!");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject * Storm_SFileCloseArchive(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	bool result;

	if (!PyArg_ParseTuple(args, "i:SFileCloseArchive", &mpq)) {
		return NULL;
	}
	result = SFileCloseArchive(mpq);

	if (!result) {
		PyErr_SetString(StormError, "Error closing archive");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject * Storm_SFileCompactArchive(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	char *listfile;
	bool reserved = 0; /* Unused */
	bool result;

	if (!PyArg_ParseTuple(args, "iz:SFileCompactArchive", &mpq, &listfile)) {
		return NULL;
	}
	result = SFileCompactArchive(mpq, listfile, reserved);

	if (!result) {
		PyErr_SetString(StormError, "Error compacting archive");
		return NULL;
	}

	Py_RETURN_NONE;
}

/*
 * Using Patched archives
 */

static PyObject * Storm_SFileOpenPatchArchive(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	char *name;
	char *prefix;
	int flags;
	bool result;

	if (!PyArg_ParseTuple(args, "iszi:SFileOpenPatchArchive", &mpq, &name, &prefix, &flags)) {
		return NULL;
	}
	result = SFileOpenPatchArchive(mpq, name, prefix, flags);

	if (!result) {
		PyErr_SetString(StormError, "Could not patch archive");
		return NULL;
	}

	Py_RETURN_TRUE;
}

static PyObject * Storm_SFileIsPatchedArchive(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	bool result;

	if (!PyArg_ParseTuple(args, "i:SFileIsPatchedArchive", &mpq)) {
		return NULL;
	}
	result = SFileIsPatchedArchive(mpq);

	if (!result) {
		Py_RETURN_FALSE;
	}

	Py_RETURN_TRUE;
}

/*
 * Reading Files
 */

static PyObject * Storm_SFileOpenFileEx(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	char *name;
	int scope;
	HANDLE file = NULL;
	bool result;

	if (!PyArg_ParseTuple(args, "isi:SFileOpenFileEx", &mpq, &name, &scope)) {
		return NULL;
	}
	result = SFileOpenFileEx(mpq, name, scope, &file);

	if (!result) {
		PyErr_SetString(StormError, "Error opening file");
		return NULL;
	}

	return Py_BuildValue("l", file);
}

static PyObject * Storm_SFileGetFileSize(PyObject *self, PyObject *args) {
	HANDLE file = NULL;
	unsigned int sizeLow;
	unsigned int sizeHigh;

	if (!PyArg_ParseTuple(args, "i:SFileGetFileSize", &file)) {
		return NULL;
	}
	sizeLow = SFileGetFileSize(file, &sizeHigh);

	if (sizeLow == SFILE_INVALID_SIZE) {
		PyErr_SetString(StormError, "Error getting file size");
		return NULL;
	}

	return Py_BuildValue("l", sizeLow | sizeHigh);
}

static PyObject * Storm_SFileSetFilePointer(PyObject *self, PyObject *args) {
	HANDLE file = NULL;
	unsigned long offset;
	int whence;
	int posLow;
	int posHigh;
	unsigned int result;

	if (!PyArg_ParseTuple(args, "iii:SFileSetFilePointer", &file, &offset, &whence)) {
		return NULL;
	}

	posLow = (unsigned int) offset;
	posHigh = (unsigned int)(offset >> 32);

	result = SFileSetFilePointer(file, posLow, &posHigh, whence);

	if (result == SFILE_INVALID_SIZE) {
		PyErr_SetString(StormError, "Error seeking in file");
		return NULL;
	}

	return Py_BuildValue("l", result | posHigh);
}

static PyObject * Storm_SFileReadFile(PyObject *self, PyObject *args) {
	HANDLE file = NULL;
	char * buffer;
	unsigned int size;
	unsigned int bytesRead;
	void * overlapped = 0;
	bool result;
	PyObject * ret;

	if (!PyArg_ParseTuple(args, "ii:SFileReadFile", &file, &size)) {
		return NULL;
	}

	buffer = (char*)malloc(size);

	result = SFileReadFile(file, buffer, size, &bytesRead, &overlapped);

	if (!result) {
		PyErr_SetString(StormError, "Error reading file");
		return NULL;
	}

	ret = Py_BuildValue("s#", buffer, bytesRead);
	free(buffer);

	return ret;
}

static PyObject * Storm_SFileCloseFile(PyObject *self, PyObject *args) {
	HANDLE file = NULL;

	bool result;
	if (!PyArg_ParseTuple(args, "i:SFileCloseFile", &file)) {
		return NULL;
	}
	result = SFileCloseFile(file);

	if (!result) {
		PyErr_SetString(StormError, "Error closing file");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject * Storm_SFileHasFile(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	char *name;
	bool result;

	if (!PyArg_ParseTuple(args, "is:SFileHasFile", &mpq, &name)) {
		return NULL;
	}
	result = SFileHasFile(mpq, name);

	if (!result) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			Py_RETURN_FALSE;
		} else {
			PyErr_SetString(StormError, "Error searching for file");
			return NULL;
		}
	}

	Py_RETURN_TRUE;
}

static PyObject * Storm_SFileGetFileName(PyObject *self, PyObject *args) {
	HANDLE file = NULL;
	char name[MAX_PATH];
	bool result;

	if (!PyArg_ParseTuple(args, "i:SFileGetFileName", &file)) {
		return NULL;
	}
	result = SFileGetFileName(file, name);

	if (!result) {
		PyErr_SetString(StormError, "Error getting file name");
		return NULL;
	}

	return Py_BuildValue("s", name);
}

static PyObject * Storm_SFileGetFileInfo(PyObject *self, PyObject *args) {
	HANDLE file = NULL;
	int type;
	int value = 0;
	long longvalue = 0;
	int size;
	bool result;

	if (!PyArg_ParseTuple(args, "ii:SFileGetFileInfo", &file, &type)) {
		return NULL;
	}

	/* Avoid calls to obsolete stuff */
	if (type == SFILE_INFO_ARCHIVE_NAME || type == SFILE_INFO_HASH_TABLE || type == SFILE_INFO_BLOCK_TABLE) {
		PyErr_SetString(PyExc_NotImplementedError, "Unsupported INFO_TYPE queried");
		return NULL;
	} else if (type == SFILE_INFO_FILETIME) {
		size = sizeof(long);
		result = SFileGetFileInfo(file, type, &longvalue, size);
	} else {
		size = sizeof(int);
		result = SFileGetFileInfo(file, type, &value, size);
	}

	if (!result) {
		if (GetLastError() == ERROR_INVALID_PARAMETER) {
			PyErr_SetString(PyExc_TypeError, "Invalid INFO_TYPE queried");
			return NULL;
		} else {
			PyErr_SetString(StormError, "Error getting info");
			return NULL;
		}
	}

	if (type == SFILE_INFO_FILETIME) {
		return Py_BuildValue("l", longvalue);
	} else {
		return Py_BuildValue("i", value);
	}
}

static PyObject * Storm_SFileExtractFile(PyObject *self, PyObject *args) {
	HANDLE mpq = NULL;
	char *name;
	char *localName;
	bool result;

	if (!PyArg_ParseTuple(args, "iss:SFileExtractFile", &mpq, &name, &localName)) {
		return NULL;
	}
	result = SFileExtractFile(mpq, name, localName);

	if (!result) {
		PyErr_SetString(StormError, "Error extracting file");
		return NULL;
	}

	Py_RETURN_NONE;
}


static PyMethodDef StormMethods[] = {
	{"SFileOpenArchive",  Storm_SFileOpenArchive, METH_VARARGS, "Open an MPQ archive."},
	/* SFileCreateArchive */
	{"SFileAddListFile", Storm_SFileAddListFile, METH_VARARGS, "Adds an in-memory listfile to an open MPQ archive"},
	/* SFileSetLocale (unimplemented) */
	/* SFileGetLocale (unimplemented) */
	{"SFileFlushArchive", Storm_SFileFlushArchive, METH_VARARGS, "Flushes all unsaved data in an MPQ archive to the disk"},
	{"SFileCloseArchive",  Storm_SFileCloseArchive, METH_VARARGS, "Close an MPQ archive."},
	{"SFileCompactArchive", Storm_SFileCompactArchive, METH_VARARGS, "Compacts (rebuilds) the MPQ archive, freeing all gaps that were created by write operations"},
	/* SFileSetMaxFileCount */
	/* SFileSetCompactCallback (unimplemented) */

	{"SFileIsPatchedArchive",  Storm_SFileIsPatchedArchive, METH_VARARGS, "Determines if an MPQ archive has been patched"},
	{"SFileOpenPatchArchive", Storm_SFileOpenPatchArchive, METH_VARARGS, "Adds a patch archive to an MPQ archive"},

	{"SFileOpenFileEx", Storm_SFileOpenFileEx, METH_VARARGS, "Open a file from an MPQ archive"},
	{"SFileGetFileSize", Storm_SFileGetFileSize, METH_VARARGS, "Retrieve the size of a file within an MPQ archive"},
	{"SFileSetFilePointer", Storm_SFileSetFilePointer, METH_VARARGS, "Seeks to a position within archive file"},
	{"SFileReadFile", Storm_SFileReadFile, METH_VARARGS, "Reads bytes in an open file"},
	{"SFileCloseFile", Storm_SFileCloseFile, METH_VARARGS, "Close an open file"},
	{"SFileHasFile", Storm_SFileHasFile, METH_VARARGS, "Check if a file exists within an MPQ archive"},
	{"SFileGetFileName", Storm_SFileGetFileName, METH_VARARGS, "Retrieve the name of an open file"},
	{"SFileGetFileInfo", Storm_SFileGetFileInfo, METH_VARARGS, "Retrieve information about an open file or MPQ archive"},
	/* SFileVerifyFile (unimplemented) */
	/* SFileVerifyArchive (unimplemented) */
	{"SFileExtractFile", Storm_SFileExtractFile, METH_VARARGS, "Extracts a file from an MPQ archive to the local drive"},
	{NULL, NULL, 0, NULL} /* Sentinel */
};


#define DECLARE(x) PyObject_SetAttrString(m, #x, PyLong_FromLong((long) x));

PyMODINIT_FUNC initstorm(void) {
	PyObject *m;

	m = Py_InitModule("storm", StormMethods);
	if (m == NULL) return;

	StormError = PyErr_NewException((char *)"storm.error", NULL, NULL);
	Py_INCREF(StormError);
	PyModule_AddObject(m, "error", StormError);

	/* SFileOpenArchive */
	DECLARE(MPQ_OPEN_NO_LISTFILE);
	DECLARE(MPQ_OPEN_NO_ATTRIBUTES);
	DECLARE(MPQ_OPEN_FORCE_MPQ_V1);
	DECLARE(MPQ_OPEN_CHECK_SECTOR_CRC);
	DECLARE(MPQ_OPEN_READ_ONLY);

	/* SFileOpenFileEx */
	DECLARE(SFILE_OPEN_PATCHED_FILE);

	/* SFileGetFileInfo */
	DECLARE(SFILE_INFO_ARCHIVE_SIZE);
	DECLARE(SFILE_INFO_HASH_TABLE_SIZE);
	DECLARE(SFILE_INFO_BLOCK_TABLE_SIZE);
	DECLARE(SFILE_INFO_SECTOR_SIZE);
	DECLARE(SFILE_INFO_NUM_FILES);
	DECLARE(SFILE_INFO_STREAM_FLAGS);
	DECLARE(SFILE_INFO_HASH_INDEX);
	DECLARE(SFILE_INFO_LOCALEID);
	DECLARE(SFILE_INFO_BLOCKINDEX);
	DECLARE(SFILE_INFO_FILE_SIZE);
	DECLARE(SFILE_INFO_COMPRESSED_SIZE);
	DECLARE(SFILE_INFO_FLAGS);
	DECLARE(SFILE_INFO_POSITION);
	DECLARE(SFILE_INFO_KEY);
	DECLARE(SFILE_INFO_KEY_UNFIXED);
	DECLARE(SFILE_INFO_FILETIME);
}

#ifdef __cplusplus
}
#endif