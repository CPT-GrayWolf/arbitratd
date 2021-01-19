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

#define CON_REQUEST     'r'
#define CON_CLIENT      'C'
#define CON_READY       'R'
#define CON_DISCONNECT  'd'
#define INFO_ALL        'A'
#define INFO_CONECTION  'I'
#define INFO_DOMAIN     'D'
#define INFO_SERVICE    'S'
#define INFO_MESSAGE    'M'
#define ERR_WARNING     'w'
#define ERR_RECOVERED   'e'
#define ERR_FAILED      'E'

int m_read(int fd, char **data);
int m_write(int fd, int ind, const char *data);
