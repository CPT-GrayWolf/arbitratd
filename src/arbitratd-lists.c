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

#include "arbitratd-lists.h"

int list_add(link_list **restrict list, link_list **restrict new)
{
	if(list == NULL || new == NULL)
		return 0;

	if(*list == NULL)
	{
		*list = malloc(sizeof(link_list));
		if(*list == NULL)
			return 0;

		(*list)->last = NULL;
		(*list)->next = NULL;
		(*list)->item = NULL;

		*new = *list;
		return 1;
	}

	link_list *current = *list;

	int count;

	for(count = 0; current->next != NULL; count++)
		current = current->next;

	current->next = malloc(sizeof(link_list));
	if(current->next == NULL)
		return 0;

	current->next->last = current;
	current = current->next;
	current->next = NULL;
	current->item = NULL;

	*new = current;
	return 1;
}

int list_free(link_list **restrict which)
{
	if(which == NULL || *which == NULL)
		return 0;

	link_list *last = (*which)->last;
	link_list *next = (*which)->next;

	if(last != NULL)
		last->next = next;
	if(next != NULL)
		next->last = last;

	free(*which);
	if(last == NULL && next == NULL)
	{
		*which = NULL;
		return 1;
	}
	else if(last == NULL && next != NULL)
	{
		*which = next;
		return 1;
	}

	return 0;
}

void list_destroy(link_list **restrict list)
{
	if(list == NULL || *list == NULL)
		return;

	link_list *current = *list;

	while(current->last != NULL)
		current = current->last;

	link_list *next = current->next;

	while(current != NULL)
	{
		free(current);

		current = next;
		if(current != NULL)
			next = current->next;
	}

	*list = NULL;

	return;
}
