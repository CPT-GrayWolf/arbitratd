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
#include "arbitratd-lists.h"

#define U_NOBODY 65534

typedef link_list client_list;

typedef struct client_o
{
	c_id                     id;
    int                      fd;
	short                    status;
	uid_t                    user;
	s_id                     service_id;
	char                    *domain;
} client_o;

int un_sock_init(const char *path);
int un_sock_accept(int server_fd, uid_t *userid);

int client_add(client_list **restrict list, client_o *restrict item);
void client_free(c_id which, client_list **restrict list);
void client_destroy(client_list **restrict list);
client_o *client_id_get(c_id which, client_list *restrict list);
client_o *client_fd_get(int which, client_list *restrict list);
