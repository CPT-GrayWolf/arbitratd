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

typedef union reg_u
{
	char *str;
	regex_t reg;
} reg_u;

typedef struct filter
{
	reg_u exp;
	ftype type;
	char *action;
	struct filter *next;
} filter;

int filter_add(filter **list, filter *new);
void filter_destroy(filter **list);
char *filter_gets(char *buff, int limit, FILE *stream, filter *list);
