/*
 * This file is a part of the arbitratd service management interface.
 * Copyright (C) 2021 Ian "Luna" Ericson & Finity Software Group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include "arbitratd-common.h"
#ifndef ARB_LISTS
#define ARB_LISTS

typedef struct link_list
{
	void               *item;
	struct link_list   *last;
	struct link_list   *next;
} link_list;

int list_add(link_list **list, link_list **new);
int list_free(link_list **which);
void list_destroy(link_list **list);
#endif
