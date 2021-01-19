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

#include "arbitratd-filters.h"

int filter_add(filter **restrict list, filter *restrict new)
{
	char *expstr = malloc(strlen(new->exp.str) + 1);
	if(expstr == NULL)
		return 0;
	strcpy(expstr, new->exp.str);

	int regstat;

	if(*list == NULL)
	{
		*list = malloc(sizeof(filter));
		if(*list == NULL)
			goto fail;

		**list = *new;
		(*list)->next = NULL;
		regstat = regcomp(&(*list)->exp.reg, expstr, (REG_EXTENDED|REG_NOSUB));
		free(expstr);
		if(regstat != 0)
		{
			new->exp.reg = (*list)->exp.reg;
			return(regstat * -1);
		}
		return 1;
	}

	filter *current = *list;

	int count = 0;

	while(current->next != NULL)
	{
		count++;
		current = current->next;
	}

	current->next = malloc(sizeof(filter));
	if(current->next == NULL)
		goto fail;

	current = current->next;
	current->next = NULL;
	regstat = regcomp(&current->exp.reg, expstr, (REG_EXTENDED|REG_NOSUB));
	free(expstr);
	if(regstat != 0)
	{
		new->exp.reg = current->exp.reg;
		return(regstat * -1);
	}
	
	return count;

	fail:
	free(expstr);
	return 0;
}

void filter_destroy(filter **restrict list)
{
	if(list == NULL)
		return;

	filter *current = *list;
	filter *next = current->next;

	while(current != NULL)
	{
		regfree(&current->exp.reg);
		free(current);
		current = NULL;

		current = next;
		if(current != NULL)
			next = current->next;
	}

	*list = NULL;

	return;
}

char *filter_gets(char *buff, int limit, FILE *stream, filter *restrict list)
{
	if(buff == NULL || list == NULL)
		return NULL;

	filter *current = list;
	filter *next = current->next;
	
	char *readstat = fgets(buff, limit, stream);
	if(readstat == NULL)
		return NULL;
	int regstat;

	while(current != NULL)
	{
		regstat = regexec(&current->exp.reg, buff, 0, NULL, 0);
		if(regstat == 0)
		{
			if(current->type == F_EXC)
			{
				buff[0] = '\0';
				return buff;
			}
			else if(current->type == F_ACT)
			{
				fputs(current->action, stream);
				return buff;
			}
		}

		current = next;
		if(current != NULL)
			next = current->next;
	}

	return buff;
}
