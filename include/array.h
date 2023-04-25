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
 * @file array.h
 * @brief Headers for array_x memory management
 */

#ifndef _GVMD_ARRAY_X_H
#define _GVMD_ARRAY_X_H

typedef struct array_x
{
  void **data;
  int len;
  int cap;
} array_x;

array_x *new_array_x(void);

void free_array_x(array_x *arr);

int append_x(array_x *arr, void *datum);

#endif
