
#ifndef _GVMD_ARRAY_X_H
#define _GVMD_ARRAY_X_H

typedef struct array_x {
    void **data;
    int len;
    int cap;
} array_x;

array_x *new_array_x(void);

void free_array_x(array_x *arr);

int append_x(array_x *arr, void *datum);

#endif
