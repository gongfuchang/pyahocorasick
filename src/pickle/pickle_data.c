#include "pickle.h"
#include "pickle_data.h"


static void
pickle_data__init_default(PickleData* data) {
	ASSERT(data != NULL);

	data->bytes_list	= NULL;
	data->size			= 0;
	data->data			= NULL;
	data->count			= NULL;
	data->top			= 0;
	data->values		= 0;
	data->error			= false;
}


static void
pickle_data__cleanup(PickleData* data) {
	ASSERT(data != NULL);

	Py_XDECREF(data->bytes_list);
	Py_XDECREF(data->values);
}


static bool
pickle_data__add_next_buffer(PickleData* data) {

	PyObject* bytes;

	ASSERT(data != NULL);

	bytes = F(PyBytes_FromStringAndSize)(NULL, data->size);
	if (UNLIKELY(bytes == NULL)) {
		return false;
	}

	if (UNLIKELY(F(PyList_Append)(data->bytes_list, bytes) < 0)) {
		Py_DECREF(bytes);
		return false;
	}

	void* raw = PyBytes_AS_STRING(bytes);

	data->count 	= (Py_ssize_t*)raw;
	(*data->count)	= 0;

	data->data  	= (uint8_t*)raw;
	data->top   	= PICKLE_CHUNK_COUNTER_SIZE;

	return true;
}


static int
pickle_data__init(PickleData* data, KeysStore store, size_t total_size, size_t max_array_size) {

	pickle_data__init_default(data);

	ASSERT(total_size > 0);
	ASSERT(max_array_size > PICKLE_TRIENODE_SIZE * 1024);

	data->bytes_list = F(PyList_New)(0);
	if (UNLIKELY(data->bytes_list == NULL)) {
		return false;
	}

	if (store == STORE_ANY) {
		data->values = F(PyList_New)(0);
		if (UNLIKELY(data->values == NULL)) {
			Py_DECREF(data->bytes_list);
			return false;
		}
	}

	if (total_size <= max_array_size) {
		data->size = total_size + PICKLE_CHUNK_COUNTER_SIZE;
	} else {
		data->size = max_array_size;
	}

	return pickle_data__add_next_buffer(data);
}

