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
#include "arbitratd-domains.h"
#include "arbitratd-poll.h"
#include "arbitratd-fork.h"
#include "arbitratd-server.h"
#include "arbitratd-comlang.h"
#include <pthread.h>
#include <ctype.h>

#ifdef LINUX
#define OPTS "+dc:s:"
#else
#define OPTS "dc:s:"
#endif

struct big
{
	const char        *name;
	const char        *socket_path;
	const char        *init_config_path;
	CONFIG 	          *init_config;
	domain_list       *domains;
	poll_o             polls;
	client_list       *clients;
	pthread_t          server_thread;
	pthread_barrier_t  thread_sync;
	int                daemon;
};

static struct big self = {NULL, NULL, NULL, NULL, NULL, {0, NULL}, NULL, 0, 1};

void *socket_thread(void *arg)
{
	sigset_t tss;
	sigfillset(&tss);
	pthread_sigmask(SIG_BLOCK, &tss, NULL);

	poll_o clients = {0, NULL};

	if(self.socket_path == NULL)
		self.socket_path = DEFAULT_SOCKET;

	int socket_main = un_sock_init(self.socket_path);
	if(socket_main < 0)
	{
		fprintf(stderr, "%s: Failed to bind socket at %s\n", self.name, self.socket_path);
		exit(1);
	}

	poll_add(&clients, socket_main, POLLIN);

	int ready;
	int readstat = 1;
	char *buff;

	printf("Waiting for client connections on socket: %s\n", self.socket_path);

	pthread_barrier_wait(&self.thread_sync);

	while(1)
	{
		ready = poll(clients.poll_array, clients.count, -1);

		if((clients.poll_array[0].revents & POLLIN) > 0)
		{
			client_o new_client = {0, -1, 0, U_NOBODY, 0, NULL};

			new_client.fd = un_sock_accept(socket_main, &new_client.user);

			poll_add(&clients, new_client.fd, POLLIN);
			client_add(&self.clients, &new_client);
			
			printf("New client: %lu\n", (client_fd_get(new_client.fd, self.clients))->id);
		}
		
		for(int i = 1; i < clients.count && ready > 0; i++)
		{
			if((clients.poll_array[i].revents & POLLIN) > 0)
			{
				while(readstat > 0)
				{
					readstat = m_read(clients.poll_array[i].fd, &buff);

					client_o *this_client = client_fd_get(clients.poll_array[i].fd, self.clients);

					if(readstat == CON_READY)
					{
						this_client->status = CON_READY;
						printf("Recieved \"ready\" from %lu\n", this_client->id);
					}
					else if(readstat == CON_REQUEST)
					{
						this_client->status = buff[0];
						printf("Recieved request \'%c\' from %lu\n", buff[0], this_client->id);
					}
					else if(readstat > 0)
					{
						if(readstat == INFO_DOMAIN)
						{
							if(domain_name_get(buff, self.domains) != NULL)
							{
								this_client->domain = buff;
								buff = NULL;
							}
						}
						else if(readstat == INFO_SERVICE)
						{
							if(this_client->domain == NULL && isdigit(buff[0]))
							{
								if(nds_id_get(atoi(buff), self.domains) != NULL)
								{
									this_client->service_id = atoi(buff);
									service_client_add(this_client->id, this_client->service_id, nds_id_get(this_client->service_id, self.domains));
									printf("Connection from %lu to NDS with ID %d ready\n", this_client->id, atoi(buff));
								}
							}
							else if(this_client->domain == NULL && !isdigit(buff[0]))
							{
								if(nds_name_get(buff, self.domains) != NULL)
								{
									this_client->service_id = ((service_o *)(nds_name_get(buff, self.domains))->services->item)->service_id;
									service_client_add(this_client->id, this_client->service_id, nds_id_get(this_client->service_id, self.domains));
									printf("Connection from %lu to NDS with ID %d ready\n", this_client->id, this_client->service_id);
								}
							}
							else
							{
								domain_o *domain_current = domain_name_get(this_client->domain, self.domains);
								
								if(isdigit(buff[0]) && service_id_get(atoi(buff), domain_current) != NULL) 
									this_client->service_id = atoi(buff);
								else if(service_name_get(buff, domain_current) != NULL)
									this_client->service_id = (service_name_get(buff, domain_current))->service_id;

								service_client_add(this_client->id, this_client->service_id, domain_current);

								printf("Connection from %lu to %d@%s ready\n", this_client->id, this_client->service_id, this_client->domain);
							}
						}
						else if(readstat == INFO_MESSAGE && this_client->service_id > 0 && buff != NULL)
						{
							char *tmp = malloc(strlen(buff) + 11);
							sprintf(tmp, "%010d:%s", this_client->user%INT32_MAX, buff);
							m_write((nds_id_get(this_client->service_id, self.domains))->control_pipe, INFO_MESSAGE, tmp);
							free(tmp);
						}
						printf("Recieved \'%c\' from %lu with data:\n%s\n", readstat, this_client->id, buff);
					}
				}

				readstat = 1;
				ready--;
			}
			
			if((clients.poll_array[i].revents & (POLLERR|POLLHUP|POLLNVAL)) > 0)
			{
				client_o *this_client = client_fd_get(clients.poll_array[i].fd, self.clients);

				printf("Client %lu disconnect\n", this_client->id);

				if(this_client->domain == NULL)
					service_client_drop(this_client->id, this_client->service_id, nds_id_get(this_client->service_id, self.domains));
				else
					service_client_drop(this_client->id, this_client->service_id, domain_name_get(this_client->domain, self.domains));
				poll_free(&clients, this_client->fd);
				client_free(this_client->id, &self.clients);

				ready--;
			}
		}
	}

	return NULL;
}

void msig_cld(int signo)
{
	pid_t dead_pid;
	int status;
	domain_o *dead;

	dead_pid = wait(&status);

	printf("Child service %d died\n", dead_pid);
		
	dead = domain_pid_get(dead_pid, self.domains);
	poll_free(&self.polls, dead->control_pipe);
	domain_free(dead_pid, &self.domains);
}

int main(int argc, char **argv, char **envp)
{
	int ac;
	self.name = argv[0];
	
	opterr = 0;

	while((ac = getopt(argc, argv, OPTS)) != -1)
	{
		switch(ac)
		{
		case 'd':
			self.daemon = 0;
			break;
		case 'c':
			self.init_config_path = optarg;
			break;
		case 's':
			self.socket_path = optarg;
			break;
		case '?':
			fprintf(stderr, "%s: Invalid argument \"%c\"\n", self.name, optopt);
		}
	}

	//if(self.daemon)
	//	daemonize();

	if(self.init_config_path == NULL)
		self.init_config_path = "/etc/arbitratd.conf";

	self.init_config = config_open(self.init_config_path);
	if(self.init_config == NULL)
	{
		fprintf(stderr, "%s: Failed to open %s\n", self.name, self.init_config_path);
		return 1;
	}

	if(config_read(self.init_config) < 0)
	{
		fprintf(stderr, "%s: Failed to read %s\n", self.name, self.init_config_path);
		return 1;
	}

	sigset_t c_set;
	sigset_t o_set;
	sigemptyset(&c_set);
	sigaddset(&c_set, SIGCHLD);
	pthread_sigmask(SIG_BLOCK, &c_set, &o_set);

	pthread_barrier_init(&self.thread_sync, NULL, 2);
	if(pthread_create(&self.server_thread, NULL, &socket_thread, NULL) > 0)
		return 1;

	domain_o next_domain;
	domain_o *selected;
	char *next_path;

	config_search_br(self.init_config, "NDS", &next_path);

	next_domain.name = NULL;
	next_domain.controller_pid = 0;
	next_domain.services = NULL;

	pthread_barrier_wait(&self.thread_sync);

	while(next_path != NULL)
	{
		next_domain.domain_config_path = next_path;

		domain_add(&self.domains, &next_domain);

		selected = domain_pid_get(0, self.domains);

		selected->controller_pid = pfork(&selected->control_pipe);
		if(selected->controller_pid < 0)
		{
			fprintf(stderr, "%s: Failed to fork controller!", self.name);
			return 1;
		}
		else if(selected->controller_pid == 0)
		{
			//execle("/usr/sbin/arb-uc", "arb-uc", selected->domain_config_path, NULL, envp);
			execle("./arb-uc", "arb-uc", selected->domain_config_path, NULL, envp);

			fprintf(stderr, "Failed to exec controller!");
			return 1;
		}

		poll_add(&self.polls, selected->control_pipe, POLLIN);
		
		printf("Started new non-domain service.  PID: %d\n", selected->controller_pid);
		
		config_search_br(self.init_config, "NDS", &next_path);
	}

	struct sigaction s_act;
	s_act.sa_handler = &msig_cld;
	sigaction(SIGCHLD, &s_act, NULL);

	pthread_sigmask(SIG_SETMASK, &o_set, NULL);

	int ready;
	int readstat = 1;
	char *buff = NULL;

	while(self.domains != NULL)
	{
		ready = poll(self.polls.poll_array, self.polls.count, -1);

		for(int i = 0; i < self.polls.count && ready > 0; i++)
		{
			if((self.polls.poll_array[i].revents & POLLIN) > 0)
			{
				while(readstat > 0)
				{
					readstat = m_read(self.polls.poll_array[i].fd, &buff);
					
					domain_o *tmp = domain_fd_get(self.polls.poll_array[i].fd, self.domains);
					if(readstat == INFO_DOMAIN)
					{
						tmp->name = buff;
						buff = NULL;
					}
					else if(readstat == INFO_SERVICE)
					{
						if(!isdigit(buff[0]) || buff[5] != '@')
						{
							fprintf(stderr, "Bad service id recieved in domain %s", tmp->name);
							kill(tmp->controller_pid, SIGTERM);
							break;
						}

						buff[5] = '\0';

						s_id new_id = atoi(buff);

						if(service_id_get(new_id, tmp) != NULL)
						{
							fprintf(stderr, "Duplicate service id recieved in domain %s", tmp->name);
							kill(tmp->controller_pid, SIGTERM);
							break;
						}

						char *new_name = malloc(strlen(buff + 6) + 1);
						if(new_name == NULL)
							return 1;
						strcpy(new_name, buff + 6);

						service_o new_service = {new_name, new_id, NULL, 0};

						service_add(tmp, &new_service);

						printf("New service %s with ID %05d in domain %s\n", new_name, new_id, tmp->name);
					}
					else if(readstat == INFO_MESSAGE)
					{
						if(buff != NULL)
							write(STDOUT_FILENO, buff + 5, strlen(buff + 5) + 1);

						char srvstr[6];
						strncpy(srvstr, buff, 5);
						srvstr[5] = '\0';
						s_id which = atoi(srvstr);

						service_o *service = service_id_get(which, tmp);
						if(service != NULL)
						{
							for(uint64_t i = 0; i < service->client_count; i++)
								m_write(service->clients[i], INFO_MESSAGE, buff + 5);
						}
					}
					else if(readstat != '\0')
					{
						if(buff != NULL)
							fprintf(stderr, "Unexpected message \"%s\" from %s", buff, tmp->name);
						else
							fprintf(stderr, "Unexpected message \'%c\' from %s", readstat, tmp->name);
					}
				}

				readstat = 1;
				ready--;
			}
		}
	}
}
