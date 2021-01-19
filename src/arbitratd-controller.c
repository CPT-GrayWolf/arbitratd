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
#include "arbitratd-filters.h"
#include "arbitratd-poll.h"
#include "arbitratd-fork.h"
#include "arbitratd-comlang.h"

typedef struct servinfo
{
	char *name;
	char *exec;
	char  id[5];
	cmode mode;
	rmode restart;
	pid_t pid;
	int cfd;
	filter *filters;
} servinfo;

int execs(char *command, char **envp)
{
	char *argv[50];
	char *argn;
	int argc = 1;

	if(strtok(command, " ") == NULL)
		return -1;

	argv[0] = command;
	
	argn = strtok(NULL, " ");
	while(argn != NULL)
	{
		argv[argc] = argn;
		
		argc++;
		if(argc >= 49)
			return -1;
		
		argn = strtok(NULL, " ");
	}
	argv[argc] = NULL;

	execve(argv[0], argv, envp);
	
	return -1;
}

int main(int argc, char **argv, char **envp)
{
	setvbuf(stdin, NULL, _IOLBF, 0);
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	char errbuff[128];

	CONFIG *init_config;
	char *init_config_path = argv[1];

	init_config = config_open(init_config_path);

	if(init_config == NULL)
	{
		sprintf(errbuff, "Failed to open %s", init_config_path);
		m_write(STDOUT_FILENO, ERR_FAILED, errbuff);
		return 1;
	}	
	
	if(config_read(init_config) < 0)
	{
		sprintf(errbuff, "Failed to read %s", init_config_path);
		m_write(STDOUT_FILENO, ERR_FAILED, errbuff);
		return 1;
	}

	char *field_name;
	char *field_data;
	
	servinfo service;
	service.filters = NULL;
	service.mode = M_PTY;
	
	config_next_br(init_config, &field_name, &field_data);
	while(field_data[0] != '\n' && field_data != NULL)
	{
		if(strcmp(field_name, "name") == 0)
		{
			if(field_data[0] != '\0' && field_data[0] != '\n')
				service.name = field_data;
			else
			{
				sprintf(errbuff, "Field %s in file %s is invalid", field_name, init_config_path);
				m_write(STDOUT_FILENO, ERR_WARNING, errbuff);
			}
		}
		else if(strcmp(field_name, "exec") == 0)
		{
			if(field_data[0] != '\0' && field_data[0] != '\n')
				service.exec = field_data;
			else
			{
				sprintf(errbuff, "Field %s in file %s is invalid", field_name, init_config_path);
				m_write(STDOUT_FILENO, ERR_WARNING, errbuff);
			}
		}
		else if(strcmp(field_name, "mode") == 0)
		{
			if(strcmp(field_data, "pty") == 0)
				service.mode = M_PTY;
			if(strcmp(field_data, "pipe") == 0)
				service.mode = M_PIPE;
			if(strcmp(field_data, "loose") == 0)
				service.mode = M_LOOSE;
		}
		config_next_br(init_config, &field_name, &field_data);
	}

	sprintf(service.id, "%05d", getpid() % USHRT_MAX);
	char *buff = malloc(6 + strlen(service.name + 1));
	sprintf(buff, "%s@%s", service.id, service.name);

	m_write(STDOUT_FILENO, INFO_DOMAIN, buff);
	m_write(STDOUT_FILENO, INFO_SERVICE, buff);

	config_rewind(init_config);

	int addstat;

	if(config_search_br(init_config, "[ Filters ]", NULL) > 0)
	{
		filter new_filter;

		config_next_br(init_config, &field_name, &field_data);
		while(field_name != NULL && strcmp(field_name, "[ End Filters ]") != 0)
		{
			if(strcmp(field_name, "new") == 0 && field_data[0] == '\n')
			{
				new_filter.exp.str = NULL;
				new_filter.type = F_NONE;
				new_filter.action = NULL;
			}
			else if(strcmp(field_name, "end") == 0 && field_data[0] == '\n')
			{
				if(new_filter.exp.str == NULL ||
				   new_filter.type == F_NONE ||
				  (new_filter.action == NULL &&
				   new_filter.type == F_ACT))
				{
					sprintf(errbuff, "Missing filter operand in %s\n", init_config_path);
					m_write(STDOUT_FILENO, ERR_FAILED, errbuff);
					return 1;
				}
				else
				{

					addstat = filter_add(&service.filters, &new_filter);
					if(addstat < 0)
					{
						char errbuff[50];
						regerror((addstat * -1), &new_filter.exp.reg, errbuff, 50);
						sprintf(errbuff, "Regex: %s in %s\n", errbuff, init_config_path);
						m_write(STDOUT_FILENO, ERR_FAILED, errbuff);
						return 1;
					}
				}
			}
			else if(strcmp(field_name, "expresion") == 0)
			{
				if(field_data[0] != '\0' && field_data[0] != '\n')
					new_filter.exp.str = field_data;
			}
			else if(strcmp(field_name, "type") == 0)
			{
				if(strcmp(field_data, "exclude") == 0)
					new_filter.type = F_EXC;
				if(strcmp(field_data, "action") == 0)
					new_filter.type = F_ACT;
			}
			else if(strcmp(field_name, "action") == 0)
			{
				if(field_data[0] != '\0' && field_data[0] != '\n')
					new_filter.action = field_data;
			}
			else
			{
				sprintf(errbuff, "Invalid option %s in %s\n", field_name, init_config_path);
				m_write(STDOUT_FILENO, ERR_FAILED, errbuff);
				return 1;
			}

			config_next_br(init_config, &field_name, &field_data);
		}
	}

	int cp[2];
	if(service.mode != M_LOOSE)
	{
		socketpair(AF_UNIX, SOCK_DGRAM, 0, cp);
	}

	if(service.mode == M_PTY)
		service.pid = tfork(&service.cfd);
	if(service.mode == M_PIPE)
		service.pid = pfork(&service.cfd);
	if(service.mode == M_LOOSE)
		service.pid = fork();
	if(service.pid < 0)
		return 1;
	else if(service.pid == 0)
	{
		if(service.mode != M_LOOSE)
		{
			close(cp[1]);

			char *wait = NULL;
			if(m_read(cp[0], &wait) != CON_READY)
				return 1;
		}
		execs(service.exec, envp);
		return 1;
	}

	if(service.mode == M_LOOSE)
	{
		pid_t dead_pid;
		while(dead_pid != service.pid)
		{
			dead_pid = wait(NULL);
		}
		return 0;

	}

	char *readstat;
	char sbuff[4096];
	char *rbuff = NULL;
	int  ind;
	FILE *tiny = fdopen(service.cfd, "r+");
	if(service.mode == M_PIPE)
		setvbuf(tiny, NULL, _IOLBF, 0);

	close(cp[0]);
	if(m_write(cp[1], CON_READY, NULL) < 0)
	{
		sprintf(errbuff, "Failed to send message to child.");
		m_write(STDOUT_FILENO, ERR_FAILED, errbuff);
		return 1;
	}

	struct pollfd either[2];
	either[0].fd = service.cfd;
	either[0].events = POLLIN;
	either[1].fd = STDIN_FILENO;
	either[1].events = POLLIN;

	while(1)
	{
		poll(either, 2, -1);

		if((either[0].revents & POLLIN) > 0)
		{
			if(service.filters == NULL)
				readstat = fgets(sbuff + 5, 4091, tiny);
			else
				readstat = filter_gets(sbuff + 5, 4091, tiny, service.filters);

			if(strlen(sbuff + 5) > 0)
			{
				strncpy(sbuff, service.id, 5);
				m_write(STDOUT_FILENO, INFO_MESSAGE, sbuff);
			}

			sbuff[5] = '\0';
		}

		if((either[1].revents & POLLIN) > 0)
		{
			ind = m_read(STDIN_FILENO, &rbuff);
			if(ind == INFO_MESSAGE && rbuff != NULL)
			{
				fprintf(tiny, "%s\n", rbuff + 11);
			}
		}

		if(waitpid(-1, NULL, WNOHANG) < 0)
			break;
	}

	return 0;
}
