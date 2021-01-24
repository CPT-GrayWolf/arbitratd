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

#include <arpa/inet.h>
#include "arbitratd-messaging.h"

int32_t m_read(int fd, char **data)
{
	struct timespec delay = {0, 0};

	if(data == NULL)
		return '\0';

	if(*data != NULL)
		free(*data);

	int32_t ind = '\0';
	char *tmp = NULL;

	int readstat = read(fd, &ind, 1);
	if(readstat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
	{
		for(int i = 1; i <= 5 && readstat <= 0; i++)
		{
			delay.tv_nsec = i * 100000000L;
			nanosleep(&delay, NULL);
			readstat = read(fd, &ind, 1);
		}
	}

	if(readstat <= 0)
		goto fail;

	if(ind == CON_READY)
		return CON_READY;
	else if(ind == CON_REQUEST)
	{
		readstat = read(fd, &ind, 1);
		if(readstat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			for(int i = 1; i <= 5 && readstat <= 0; i++)
			{
				delay.tv_nsec = i * 100000000L;
				nanosleep(&delay, NULL);
				readstat = read(fd, &ind, 1);
			}
		}

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

		readstat = read(fd, &ind, sizeof(int32_t));
		if((readstat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) || (readstat < sizeof(int32_t) && (fcntl(fd, F_GETFL, 0) & O_NONBLOCK) > 0))
		{
			// Ternary statements are trash, but this is fine.
			short total = (readstat < 0) ? 0 : readstat;
			do
			{
				for(int i = 1; i <= 5 && readstat <= 0; i++)
				{
					delay.tv_nsec = i * 100000000L;
					nanosleep(&delay, NULL);
					if(total > 0)
						readstat = read(fd, ((char *)&ind) + (total), sizeof(int32_t) - total);
					else
						readstat = read(fd, &ind, sizeof(int32_t));
				}

				if(readstat <= 0)
					break;
				else
					total += readstat;
			}
			while(total < sizeof(int32_t));
		}

		if(readstat <= 0)
			goto fail;

		ind = ntohl(ind);
		if(ind > 0)
		{
			tmp = malloc(ind);
			readstat = read(fd, tmp, ind);
			if((readstat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) || (readstat < ind && (fcntl(fd, F_GETFL, 0) & O_NONBLOCK) > 0))
			{
				short total = (readstat < 0) ? 0 : readstat;
				do
				{
					for(int i = 1; i <=5 && readstat <= 0; i++)
					{
						delay.tv_nsec = i * 100000000L;
						nanosleep(&delay, NULL);
						if(total > 0)
							readstat = read(fd, tmp + (total), ind - total);
						else
							readstat = read(fd, tmp, ind);
					}

					if(readstat <= 0)
						break;
					else
						total += readstat;
				}
				while(total <= 0);
			}

			if(readstat <= 0)
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

int32_t m_write(int fd, int32_t ind, const char *data)
{
	struct timespec delay = {0, 0};

	int64_t len = 0;
	int32_t len_ind = 0;
	if(data != NULL)
	{
		len = strnlen(data, INT32_MAX);
		if(len >= INT32_MAX)
			return -1;
		else
			len++;

		len_ind = htonl((uint32_t)len);
	}

	int writestat = 0;
	if(ind == CON_READY)
	{
		writestat = write(fd, &ind, 1);
		if(writestat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			for(int i = 1; i <= 5 && writestat <= 0; i++)
			{
				delay.tv_nsec = i * 100000000L;
				nanosleep(&delay, NULL);
				writestat = write(fd, &ind, 1);
			}
		}
	}
	else if(ind == CON_REQUEST && data != NULL)
	{
		writestat = write(fd, &ind, 1);
		if(writestat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			for(int i = 1; i <= 5 && writestat <= 0; i++)
			{
				delay.tv_nsec = i * 100000000L;
				nanosleep(&delay, NULL);
				writestat = write(fd, &ind, 1);
			}
		}
		writestat = write(fd, data, 1);
		if(writestat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			for(int i = 1; i <= 5 && writestat <= 0; i++)
			{
				delay.tv_nsec = i * 100000000L;
				nanosleep(&delay, NULL);
				writestat = write(fd, &ind, 1);
			}
		}
	}
	else
	{
		writestat = write(fd, &ind, 1);
		if(writestat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		{
			for(int i = 1; i <= 5 && writestat <= 0; i++)
			{
				delay.tv_nsec = i * 100000000L;
				nanosleep(&delay, NULL);
				writestat = write(fd, &ind, 1);
			}
		}

		writestat = write(fd, &len_ind, sizeof(int32_t));
		if((writestat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) || (writestat < sizeof(int32_t) && (fcntl(fd, F_GETFL, 0) & O_NONBLOCK) > 0))
		{
			short total = (writestat < 0) ? 0 : writestat;
			do
			{
				for(int i = 1; i <= 5 && writestat <= 0; i++)
				{
					delay.tv_nsec = i * 100000000L;
					nanosleep(&delay, NULL);
					if(total > 0)
						writestat = write(fd, ((char *)&len_ind) + (total), sizeof(int32_t) - total);
					else
						writestat = write(fd, &len_ind, sizeof(int32_t));
				}

				if(writestat <= 0)
					break;
				else
					total += writestat;
			}
			while(total < sizeof(int32_t));
		}
		if(len > 0)
		{
			writestat = write(fd, data, len);
			if((writestat <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) || (writestat < len && (fcntl(fd, F_GETFL, 0) & O_NONBLOCK) > 0))
			{
				short total = (writestat < 0) ? 0 : writestat;
				do
				{
					for(int i = 1; i <= 5 && writestat <= 0; i++)
					{
						delay.tv_nsec = i * 100000000L;
						nanosleep(&delay, NULL);
						if(total > 0)
							writestat = write(fd, data + (total), len - total);
						else
							writestat = write(fd, data, len);
					}

					if(writestat <= 0)
						break;
					else
						total += writestat;
				}
				while(total < len);
			}
		}
	}

	return len;
}
