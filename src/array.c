#include "array.h"
#include "postgres.h"


array_x *new_array_x(void) {
    array_x *arr;

    if ((arr = (array_x*)palloc0(sizeof(array_x))) == NULL) {
        return NULL;
    }

    if ((arr->data = (void**)palloc0(sizeof(void*) * 10)) == NULL) {
        pfree(arr);
        return NULL;
    }

    arr->cap = 10;
    arr->len = 0;

    return arr;
}

void free_array_x(array_x *arr) {
    if (arr != NULL) {
        if (arr->data != NULL) {
            int i;
            for (i = 0; i < arr->len; i++) {
                if (arr->data[i] != NULL) {
                    pfree(arr->data[i]);
                    arr->data[i] = NULL;
                }
            }
            pfree(arr->data);
            arr->data = NULL;
        }
        pfree(arr);
    }
}

int append_x(array_x *arr, void *datum) {
    if (arr->len == arr->cap) {
        int new_size = arr->cap*2;
        void **ndata = (void**)repalloc(arr->data, new_size*sizeof(void*));
        if (ndata == NULL) {
            return 0;
        }
        memset(&arr->data[arr->len], 0, sizeof(void*)*arr->len);
    }
    arr->data[arr->len++] = datum;
    return 1;
}

