/*
 * Copyright (C) 2023 TDT AG <development@tdt.de>
 *
 * This is free software, licensed under the
 * GNU Lesser General Public License, Version 2.1.
 * See https://www.gnu.org/licenses/lgpl-2.1.txt for more information.
 *
 */

#ifndef __QUERY_H
#define __QUERY_H

#define VICI_SOCKET "/var/run/charon.vici"

int query_command(const char *command, const char *event);

#endif
