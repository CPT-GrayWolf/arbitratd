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

#include "arbitratd-server.h"
#include <sys/stat.h>
#include <time.h>

#define STALE_SOCKET  60

int un_sock_init(const char *path)
{
	struct sockaddr_un sock_main;

	if(strlen(path) >= sizeof(sock_main.sun_path))
		return -1;

	memset(&sock_main, 0, sizeof(sock_main));
	sock_main.sun_family = AF_UNIX;
	strcpy(sock_main.sun_path, path);
	socklen_t len = offsetof(struct sockaddr_un, sun_path) + strlen(sock_main.sun_path);

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0)
		return -2;

	int c_flags = fcntl(fd, F_GETFL, 0);
	if(c_flags == -1)
		goto fail;
	if(fcntl(fd, F_SETFL, (c_flags|O_NONBLOCK)) == -1)
		goto fail;

	if(bind(fd, (struct sockaddr *)&sock_main, len) < 0)
		goto fail;

	if(listen(fd, 20) != 0)
		goto fail;

	return fd;

	fail:
		close(fd);
		return -3;
}

int un_sock_accept(int server_fd, uid_t *userid)
{
	int client_fd, err;
	time_t maxt;
	struct sockaddr_un sock_new;
	struct stat statbuff;

	char *path = malloc(sizeof(sock_new.sun_path) + 1);
	if(path == NULL)
		return -1;

	socklen_t len = sizeof(sock_new);

	client_fd = accept(server_fd, (struct sockaddr *)&sock_new, &len);
	if(client_fd < 0)
	{
		err = -2;
		goto fail;
	}

	len -= offsetof(struct sockaddr_un, sun_path);
	memcpy(path, sock_new.sun_path, len);
	path[len] = '\0';

	if(stat(path, &statbuff) < 0)
	{
		err = -3;
		goto fail;
	}

	if(!S_ISSOCK(statbuff.st_mode))
	{
		err = -4;
		goto fail;
	}

	if((statbuff.st_mode & (S_IRWXG | S_IRWXO)) || ((statbuff.st_mode & S_IRWXU) != S_IRWXU))
	{
		err = -5;
		goto fail;
	}

	maxt = time(NULL) - STALE_SOCKET;
	if(statbuff.st_atime < maxt || statbuff.st_ctime < maxt || statbuff.st_mtime < maxt)
	{
		err = -6;
		goto fail;
	}

	if(userid != NULL)
		*userid = statbuff.st_uid;

	int c_flags = fcntl(client_fd, F_GETFL, 0);
	if(c_flags == -1)
	{
		err = -7;
		goto fail;
	}
	if(fcntl(client_fd, F_SETFL, (c_flags|O_NONBLOCK)) == -1)
	{
		err = -7;
		goto fail;
	}

	unlink(path);
	free(path);
	return client_fd;

	fail:
		free(path);
		return err;
}

static c_id next_id(client_list *restrict list)
{
	static c_id next;
	
	next++;

	for(int i; i < 100 && client_id_get(next, list) != NULL && next != 0; i++)
	{
		next++;
	}

	if(client_id_get(next, list) != NULL)
		return 0;
	else
		return next;
}

int client_add(client_list **restrict list, client_o *restrict item)
{
	if(item == 0)
		return 0;
	
	client_o *new_item = malloc(sizeof(client_o));
	if(new_item == NULL)
		return 0;

	*new_item = *item;

	client_list *list_new;
	int count = list_add(list, &list_new);
	if(count == 0)
		goto fail;

	new_item->id = next_id(*list);
	list_new->item = new_item;

	return count;

	fail:

		free(new_item);
		new_item = NULL;
		return 0;
}

void client_free(c_id which, client_list **restrict list)
{
	if(list == NULL || *list == NULL)
		return;

	client_o *current_item = (*list)->item;
	client_list *current = *list;
	client_list *next = current->next;

	while(current_item->id != which)
	{
		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
		else
			return;
	}
	
	if(current_item->domain != NULL)
		free(current_item->domain);
	free(current_item);

	if(list_free(&current))
		*list = current;

	return;
}

void client_destroy(client_list **restrict list)
{
	if(list == NULL || *list == NULL)
		return;

	client_o *current_item = (*list)->item;
	client_list *current = *list;
	client_list *next = current->next;

	while(current != NULL)
	{
		free(current_item);

		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	list_destroy(list);

	*list = NULL;

	return;
}

client_o *client_id_get(c_id which, client_list *restrict list)
{
	if(list == NULL)
		return NULL;

	client_o *current_item = list->item;
	client_list *current = list;
	client_list *next = current->next;

	if(current_item == NULL)
		return NULL;

	while(current_item->id != which)
	{
		current = next;
		if(current != NULL && current->item != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
		else
			return NULL;
	}

	return current_item;
}

client_o *client_fd_get(int which, client_list *restrict list)
{
	if(list == NULL)
		return NULL;

	client_o *current_item = list->item;
	client_list *current = list;
	client_list *next = current->next;

	if(current_item == NULL)
		return NULL;

	while(current_item->fd != which)
	{
		current = next;
		if(current != NULL && current->item != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
		else
			return NULL;
	}

	return current_item;
}
