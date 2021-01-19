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

#include "arbitratd-domains.h"

int domain_add(domain_list **restrict list, domain_o *restrict item)
{
	if(item == NULL)
		return 0;

	domain_o *new_item = malloc(sizeof(domain_o));
	if(new_item == NULL)
		return 0;

	*new_item = *item;

	domain_list *list_new;
	int count = list_add(list, &list_new);
	if(count == 0)
		goto fail;

	list_new->item = new_item;

	return count;

	fail:
		free(new_item);
		return 0;
		
}

void domain_free(pid_t which, domain_list **restrict list)
{
	if(list == NULL || *list == NULL)
		return;

	domain_o *current_item = (*list)->item;
	domain_list *current = *list;
	domain_list *next = current->next;

	while(current_item->controller_pid != which)
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

	if(current_item->services != NULL)
	{
		service_destroy(current_item);
	}

	if(current_item->name != NULL)
		free(current_item->name);

	free(current_item);

	if(list_free(&current))
		*list = current;

	return;
}

void domain_destroy(domain_list **restrict list)
{
	if(list == NULL || *list == NULL)
		return;

	domain_o *current_item = (*list)->item;
	domain_list *current = *list;
	domain_list *next = current->next;

	while(current != NULL)
	{
		if(current_item->services != NULL)
			service_destroy(current_item);

		if(current_item->name != NULL)
			free(current_item->name);

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

domain_o *domain_pid_get(pid_t which, domain_list *restrict list)
{
	if(list == NULL)
		return NULL;

	domain_o *current_item = list->item;
	domain_list *next = list->next;
	domain_list *current = list;

	while(current_item->controller_pid != which && current != NULL)
	{
		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	if(current != NULL)
		return current_item;
	else
		return NULL;
}

domain_o *domain_name_get(const char *which, domain_list *restrict list)
{
	if(list == NULL || which == NULL)
		return NULL;

	domain_o *current_item = list->item;
	domain_list *next = list->next;
	domain_list *current = list;

	while(strcmp(current_item->name, which) != 0 && current != NULL)
	{
		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	if(current != NULL)
		return current_item;
	else
		return NULL;
}

domain_o *domain_fd_get(int which, domain_list *restrict list)
{
	if(list == NULL)
		return NULL;

	domain_o *current_item = list->item;
	domain_list *next = list->next;
	domain_list *current = list;

	while(current_item->control_pipe != which && current != NULL)
	{
		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	if(current != NULL)
		return current_item;
	else
		return NULL;
}

domain_o *nds_name_get(const char *which, domain_list *restrict list)
{
	if(list == NULL || which == NULL)
		return NULL;

	domain_o *current_item = list->item;
	domain_list *next = list->next;
	domain_list *current = list;

	while(strcmp(current_item->name + 6, which) != 0 && current != NULL)
	{
		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	if(current != NULL)
		return current_item;
	else
		return NULL;
}

domain_o *nds_id_get(s_id which, domain_list *restrict list)
{
	if(list == NULL)
		return NULL;

	char id_str[6];
        sprintf(id_str, "%05d", which);

	domain_o *current_item = list->item;
	domain_list *next = list->next;
	domain_list *current = list;

	while(strncmp(current_item->name, id_str, 5) != 0 && current != NULL)
	{
		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	if(current != NULL)
		return current_item;
	else
		return NULL;
}

int service_add(domain_o *restrict domain, service_o *restrict item)
{
	if(domain == NULL || item == NULL)
		return 0;

	service_o *new_item = malloc(sizeof(service_o));
	if(new_item == NULL)
		return 0;

	*new_item = *item;

	service_list *list_new;
	int count = list_add(&domain->services, &list_new);
	if(count == 0)
		goto fail;

	list_new->item = new_item;

	return count;

	fail:
		free(new_item);
		return 0;
}

void service_free(s_id which, domain_o *restrict domain)
{
	if(domain == NULL || domain->services == NULL)
		return;

	service_list *current = domain->services;
	service_list *next = current->next;
	service_o *current_item = current->item;
	
	while(current_item->service_id != which)
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

	if(current_item->clients != NULL && current_item->client_count != 0)
		free(current_item);

	if(current_item->name != NULL)
		free(current_item->name);

	free(current_item);

	if(list_free(&current))
		domain->services = current;

	return;
}

void service_destroy(domain_o *restrict domain)
{
	if(domain == NULL || domain->services == NULL)
		return;

	service_list *current = domain->services;
	service_list *next = current->next;
	service_o *current_item = current->item;

	while(current != NULL)
	{
		if(current_item->clients != NULL && current_item->client_count != 0)
			free(current_item->clients);

		if(current_item->name != NULL)
			free(current_item->name);

		free(current_item);

		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	list_destroy(&domain->services);

	domain->services = NULL;

	return;
}

service_o *service_id_get(s_id which, domain_o *restrict domain)
{
	if(domain == NULL || domain->services == NULL)
		return NULL;

	service_list *current = domain->services;
	service_list *next = current->next;
	service_o *current_item = current->item;

	while(current_item->service_id != which && current != NULL)
	{
		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	if(current != NULL)
		return current_item;
	else
		return NULL;
}

service_o *service_name_get(const char *which, domain_o *restrict domain)
{
	if(domain == NULL || domain->services == NULL || which == NULL)
		return NULL;

	service_list *current = domain->services;
	service_list *next = current->next;
	service_o *current_item = current->item;

	while(strcmp(current_item->name, which) != 0 && current != NULL)
	{
		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	if(current != NULL)
		return current_item;
	else
		return NULL;
}

service_o *service_client_get(c_id which, domain_o *restrict domain)
{
	if(domain == NULL || domain->services == NULL)
		return NULL;

	service_list *current = domain->services;
	service_list *next = current->next;
	service_o *current_item = current->item;

	int id = 0;

	while(current != NULL)
	{
		if(current_item->clients != NULL || current_item->client_count == 0)
		{
			int i;

			for(i = 0; i < current_item->client_count && current_item->clients[i] != which; i++);
			if(i < current_item->client_count)
				id = current_item->clients[i];
		}

		if(id == which)
			break;

		current = next;
		if(current != NULL)
		{
			current_item = current->item;
			next = current->next;
		}
	}

	if(current != NULL)
		return current_item;
	else
		return NULL;
}

int service_client_add(c_id client_id, s_id which, domain_o *restrict domain)
{
	if(domain == NULL || domain->services == NULL)
		return 0;

	service_o *client_service = service_id_get(which, domain);
	if(client_service == NULL)
		return 0;
	
	if(client_service->clients == NULL || client_service->client_count == 0)
	{
		client_service->clients = malloc(sizeof(c_id));
		if(client_service->clients == NULL)
			return 0;

		client_service->client_count = 1;

		client_service->clients[0] = client_id;
	}
	else
	{
		c_id *tmp = realloc(client_service->clients, (sizeof(c_id) * ++(client_service->client_count)));
		if(tmp == NULL)
			return 0;
		else
			client_service->clients = tmp;

		client_service->clients[client_service->client_count - 1] = client_id;
	}
	return client_service->client_count;
}

void service_client_drop(c_id client_id, s_id which, domain_o *restrict domain)
{
	if(domain == NULL || domain->services == NULL)
		return;

	service_o *client_service = service_id_get(which, domain);
	if(client_service == NULL)
		return;

	if(client_service->client_count == 1 && client_service->clients[0] == client_id)
	{
		free(client_service->clients);
		client_service->clients = NULL;
		client_service->client_count = 0;
		return;
	}

	int i;

	for(i = 0; i < client_service->client_count && client_service->clients[i] != client_id; i++);
	if(i == client_service->client_count)
		return;

	if(i < (client_service->client_count - 1))
	{
		memmove((client_service->clients + i), 
			(client_service->clients + (i + 1)), 
			(sizeof(c_id) * (--(client_service->client_count) - i)));
	}
	else
	{
		--(client_service->client_count);
	}

	c_id *tmp = realloc(client_service->clients, (sizeof(c_id) * client_service->client_count));
	if(tmp == NULL)
		return;
	else
		client_service->clients = tmp;

	return;
}
