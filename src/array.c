/* Copyright (C) 2020 Greenbone AG
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file array.c
 * @brief Memory management for arrays based on postgres functions
 *
 * Functions to have transaction safe memory allocation using palloc0 and pfree. 
 */

#include "array.h"
#include "postgres.h"

/**
 * @brief Allocate memory for new array_x
 *
 * @return The new array_x
 */
array_x
*new_array_x(void)
{
  array_x *arr;

  if ((arr = (array_x*)palloc0(sizeof(array_x))) == NULL)
    return NULL;

  if ((arr->data = (void**)palloc0(sizeof(void*) * 10)) == NULL)
    {
      pfree(arr);
      return NULL;
    }

  arr->cap = 10;
  arr->len = 0;

  return arr;
}

/**
 * @brief Free the memory of a previously created array_x
 *
 * @param[in]   arr   The array
 */
void
free_array_x(array_x *arr)
{
  if (arr != NULL)
    {
      if (arr->data != NULL)
        {
          int i;
          for (i = 0; i < arr->len; i++)
            {
              if (arr->data[i] != NULL)
                {
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

/**
 * @brief Append a new item to an existing array
 *
 * @param[in]   arr     The Array
 * @param[in]   datum   The new item
 *
 * @return 1 if item appended, else 0
 */
int
append_x(array_x *arr, void *datum)
{
  if (arr->len == arr->cap)
    {
      int new_size = arr->cap*2;
      void **ndata = (void**)repalloc(arr->data, new_size*sizeof(void*));
      if (ndata == NULL)
        return 0;

      memset(&arr->data[arr->len], 0, sizeof(void*)*arr->len);
    }
    arr->data[arr->len++] = datum;
    return 1;
}

