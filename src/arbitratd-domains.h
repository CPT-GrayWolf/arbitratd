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

typedef link_list service_list;
typedef link_list domain_list;

typedef struct service_o
{
	char                    *name;
	s_id                     service_id;
	c_id                    *clients;
	uint64_t                 client_count;
} service_o;

typedef struct domain_o
{
	char                    *name;
	pid_t                    controller_pid;
	int                      control_pipe;
	service_list            *services;
	char                    *domain_config_path;
} domain_o;

int domain_add(domain_list **list, domain_o *item);
void domain_free(pid_t which, domain_list **list);
void domain_destroy(domain_list **list);
domain_o *domain_pid_get(pid_t which, domain_list *list);
domain_o *domain_name_get(const char *which, domain_list *list);
domain_o *domain_fd_get(int which, domain_list *list);

domain_o *nds_name_get(const char *which, domain_list *list);
domain_o *nds_id_get(s_id which, domain_list *list);

int service_add(domain_o *restrict domain, service_o *item);
void service_free(s_id which, domain_o *domain);
void service_destroy(domain_o *domain);
service_o *service_id_get(s_id which, domain_o *domain);
service_o *service_name_get(const char *which, domain_o *domain);
service_o *service_client_get(c_id which, domain_o *domain);
int service_client_add(c_id client_id, s_id which, domain_o *domain);
void service_client_drop(c_id client_id, s_id which, domain_o *domain);
