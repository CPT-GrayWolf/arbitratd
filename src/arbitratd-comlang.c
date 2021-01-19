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

#include "arbitratd-comlang.h"

int m_read(int fd, char **data)
{
	if(data == NULL)
		return '\0';

	if(*data != NULL)
		free(*data);

	int ind = '\0';
	char *tmp = NULL;

	int readstat = read(fd, &ind, 1);
	if(readstat <= 0)
		goto fail;

	if(ind == CON_READY)
		return CON_READY;
	else if(ind == CON_REQUEST)
	{
		readstat = read(fd, &ind, 1);
		if(readstat <= 0)
			goto fail;
		tmp = malloc(1);
		*tmp = ind;
		*data = tmp;
		return CON_REQUEST;
	}
	else
	{
		char type = ind;

		readstat = read(fd, &ind, sizeof(int));
		if(readstat <= 0)
			goto fail;
		if(ind > 0)
		{
			tmp = malloc(ind);
			readstat = read(fd, tmp, ind);
			if(readstat <= 0 || readstat != ind)
				goto fail;
			*data = tmp;
			return type;
		}
		else
		{
			*data = NULL;
			return type;
		}
	}


	fail:
		free(tmp);
		*data = NULL;
		return '\0';
}

int m_write(int fd, int ind, const char *data)
{
	int len = 0;
	if(data != NULL)
	{
		len = strnlen(data, INT_MAX);
		if(len == INT_MAX)
			return -1;
		else
			len++;
	}

	if(ind == CON_READY)
		write(fd, &ind, 1);
	else if(ind == CON_REQUEST && data != NULL)
	{
		write(fd, &ind, 1);
		write(fd, data, 1);
	}
	else
	{
		write(fd, &ind, 1);

		write(fd, (char *)&len, sizeof(int));
		if(len > 0)
			write(fd, data, len);
	}

	return len;
}
