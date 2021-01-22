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
#include "arbitratd-client.h"
#include "arbitratd-messaging.h"

#ifdef LINUX
#define OPTS "+fs:"
#else
#define OPTS "fs:"
#endif

struct big
{
	const char       *socket_path;
	char             *message;
	char             *domain;
	char             *service;
	int               follow;
};

static struct big self = {NULL, NULL, NULL, NULL, 0};

int main(int argc, char **argv)
{
	int ac, expected = 3;

	opterr = 0;

	while((ac = getopt(argc, argv, OPTS)) != -1)
	{
		switch(ac)
		{
		case 's':
			self.socket_path = optarg;
			expected += 2;
			break;
		case 'f':
			self.follow = 1;
			expected++;
			break;
		case '?':
			fprintf(stderr, "%s: Invalid argument \"%c\"\n", argv[0], optopt);
			return 1;
		}
	}

	if(argc < expected)
	{
		fprintf(stderr, "%s: To few arguments.\n", argv[0]);
		return 1;
	}
	else if(argc > expected)
	{
		fprintf(stderr, "%s: To many arguments.\n", argv[0]);
		return 1;
	}

	if(self.socket_path == NULL)
		self.socket_path = DEFAULT_SOCKET;

	self.message = argv[argc - 1];
	self.domain = argv[argc - 2];

	
	for(ac = 0; self.domain[ac] != '.' && self.domain[ac] != '\0'; ac++);

	if(self.domain[ac] == '.')
	{
		self.domain[ac] = '\0';
		
		self.service = self.domain + ++ac;
	}
	else
	{
		self.service = self.domain;

		self.domain = NULL;
	}

	printf("Domain: %s\n", self.domain);
	printf("Service: %s\n", self.service);
	printf("Message: %s\n", self.message);

	int server_fd = un_client_connect(self.socket_path);
	if(server_fd < 0)
	{
		fprintf(stderr, "%s: Failed to connect to server \"%s\"\n", argv[0], self.socket_path);
		return 1;
	}

	char *buff = NULL;

	m_write(server_fd, INFO_DOMAIN, self.domain);
	m_read(server_fd, &buff);

	m_write(server_fd, INFO_SERVICE, self.service);
	m_read(server_fd, &buff);

	m_write(server_fd, INFO_MESSAGE, self.message);
	m_read(server_fd, &buff);
}
