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

#ifndef AR_COMMON
#define AR_COMMON

#define _XOPEN_SOURCE 600
#define _POSIX_C_SOURCE 200809L
#include <libreadconf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <syslog.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <regex.h>
#include <time.h>
#include <poll.h>
#include <termios.h>

#define DEFAULT_SOCKET "/run/ar.sock"
#define M_DAEMON   0x00000001
#define M_VERBOSE  0x00000010

typedef enum ftype
{
	F_ACT,
	F_EXC,
	F_EXT,
	F_NONE
} ftype;

typedef enum cmode
{
	M_PTY,
	M_PIPE,
	M_LOOSE
} cmode;

typedef enum rmode
{
	R_ONFAIL,
	R_ALWAYS,
	R_NEVER
} rmode;

typedef uint64_t c_id;
typedef uint16_t s_id;

#endif
