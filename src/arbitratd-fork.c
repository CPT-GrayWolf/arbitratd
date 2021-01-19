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

#include "arbitratd-fork.h"

int pfork(int *fdm)
{
	int sock[2];
	socketpair(AF_UNIX, SOCK_DGRAM, 0, sock);

	pid_t id = fork();
	if(id < 0)
	{
		close(sock[0]);
		close(sock[1]);
		return -1;
	}
	else if(id == 0)
	{
		close(sock[0]);
		if(dup2(sock[1], STDIN_FILENO) < 0)
			exit(1);
		if(dup2(sock[1], STDOUT_FILENO) < 0)
			exit(1);
		if(dup2(sock[1], STDERR_FILENO) < 0)
			exit(1);
		close(sock[1]);

		return 0;
	}
	else
	{
		close(sock[1]);
		*fdm = sock[0];
		return id;
	}
}

int tfork(int *fdm)
{
	int ptm = posix_openpt(O_RDWR|O_NOCTTY);
	if(ptm < 0 || grantpt(ptm) < 0 || unlockpt(ptm) < 0)
		return -1;
	int pts = open(ptsname(ptm), (O_RDWR|O_NOCTTY));
	if(pts < 0)
		return -1;
	
	pid_t id = fork();
	if(id < 0)
	{
		close(ptm);
		close(pts);
		return -1;
	}
	else if(id == 0)
	{
		close(ptm);

		if(setsid() < 0)
			exit(1);

		if(dup2(pts, STDIN_FILENO) < 0)
			exit(1);
		if(dup2(pts, STDOUT_FILENO) < 0)
			exit(1);
		if(dup2(pts, STDERR_FILENO) < 0)
			exit(1);
		close(pts);
		
		if(ioctl(STDIN_FILENO, TIOCSCTTY, 0) < 0)
			exit(1);

		struct termios terms;

		if(tcgetattr(STDIN_FILENO, &terms) < 0)
			exit(1);

		terms.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL);
		terms.c_oflag &= ~(ONLCR);

		if(tcsetattr(STDIN_FILENO, TCSANOW, &terms) < 0)
			exit(1);

		return 0;
	}
	else
	{
		close(pts);
		*fdm = ptm;
		return id;
	}
}
