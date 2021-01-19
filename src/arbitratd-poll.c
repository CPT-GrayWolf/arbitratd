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

#include "arbitratd-poll.h"

int poll_add(poll_o *polls, int fd, short events)
{
	if(polls == NULL)
		return 0;

	int c_flags = fcntl(fd, F_GETFL, 0);
	if(c_flags == -1)
		return 0;
	if(fcntl(fd, F_SETFL, (c_flags|O_NONBLOCK)) == -1)
		return 0;

	if(polls->poll_array == NULL)
	{
		polls->poll_array = malloc(sizeof(struct pollfd));
		if(polls->poll_array == NULL)
			return 0;

		polls->count = 1;

		polls->poll_array[0].fd = fd;
		polls->poll_array[0].events = events;
	}
	else
	{
		struct pollfd *tmp;
		
		tmp = realloc(polls->poll_array, (sizeof(struct pollfd) * ++(polls->count)));
		if(tmp == NULL)
			return 0;
		else
			polls->poll_array = tmp;

		polls->poll_array[polls->count - 1].fd = fd;
		polls->poll_array[polls->count - 1].events = events;
	}
	return polls->count;
}

void poll_free(poll_o *polls, int fd)
{
	if(polls == NULL)
		return;

	if(polls->count == 1 && polls->poll_array[0].fd == fd)
	{
		close(polls->poll_array[0].fd);
		free(polls->poll_array);
		polls->poll_array = NULL;
		polls->count = 0;
		return;
	}

	int i;

	for(i = 0; i < polls->count && polls->poll_array[i].fd != fd; i++);
	if(i == polls->count)
		return;

	close(polls->poll_array[i].fd);
	
	struct pollfd *tmp;

	if(i < (polls->count - 1))
	{
		memmove((polls->poll_array + i), (polls->poll_array+ (i + 1)), (sizeof(struct pollfd) * (--(polls->count) - i)));
	}
	else
	{
		--(polls->count);
	}

	tmp = realloc(polls->poll_array, (sizeof(struct pollfd) * polls->count));
	if(tmp == NULL)
		return;
	else
		polls->poll_array = tmp;

	return;
}
