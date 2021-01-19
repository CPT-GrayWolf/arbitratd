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

#include "arbitratd-client.h"
#include <sys/stat.h>

int un_client_connect(const char *path)
{
	int err;
	struct sockaddr_un sock_new, sock_server;

	if(strlen(path) >= sizeof(sock_new.sun_path))
		return -1;

	int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
		return -1;

	memset(&sock_new, 0, sizeof(sock_new));
	sock_new.sun_family = AF_UNIX;
	sprintf(sock_new.sun_path, "%s%05ld.sock", CLIENT_SOCK_PATH, (long)getpid());
	int len = offsetof(struct sockaddr_un, sun_path) + strlen(sock_new.sun_path);

	unlink(sock_new.sun_path);
	if(bind(socket_fd, (struct sockaddr *)&sock_new, len) < 0)
	{
		err = -2;
		goto fail;
	}

	if(chmod(sock_new.sun_path, CLIENT_SOCK_PERM) < 0)
	{
		err = -3;
		goto fail;
	}

	memset(&sock_server, 0, sizeof(sock_server));
	sock_server.sun_family = AF_UNIX;
	strcpy(sock_server.sun_path, path);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(path);
	if(connect(socket_fd, (struct sockaddr *)&sock_server, len) < 0)
	{
		err = -4;
		goto fail;
	}

	return socket_fd;

	fail:
		close(socket_fd);
		if(err <= -3)
			unlink(sock_new.sun_path);
		return err;
}
