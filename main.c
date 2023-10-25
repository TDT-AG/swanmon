/*
 * Copyright (C) 2023 TDT AG <development@tdt.de>
 *
 * This is free software, licensed under the
 * GNU Lesser General Public License, Version 2.1.
 * See https://www.gnu.org/licenses/lgpl-2.1.txt for more information.
 *
 */

#include <stdio.h>
#include <string.h>

#include "query.h"

struct command {
	const char *name;
	const char *event_name;
	const char *description;
} command;

static const struct command help_command = {"help", NULL, "Shows swanmon help"};
static const struct command commands[] = {
	{"version", NULL, "Returns daemon and system specific version information"},
	{"stats", NULL, "Returns IKE daemon statistics and load information"},
	{"list-sas", "list-sa", "Lists currently active IKE_SAs and associated CHILD_SAs"},
	{"get-conns", NULL, "Return a list of connection names loaded exclusively over vici"},
	{"list-conns", "list-conn", "List currently loaded connections. This call includes all connections known by the daemon, not only those loaded over vici"},
	{"get-authorities", NULL, "Return a list of currently loaded certification authority names"},
	{"list-authorities", "list-authority", "List currently loaded certification authority information"},
	{"list-policies", "list-policy", "List currently installed trap, drop and bypass policies"},
	{"list-certs", "list-cert", "List currently loaded certificates. This call includes all certificates known by the daemon, not only those loaded over vici"},
	{"get-keys", NULL, "Return a list of identifiers of private keys loaded exclusively over vici"},
	{"get-shared", NULL, "Return a list of unique identifiers of shared keys loaded exclusively over vici"},
	{"get-pools", NULL, "List the currently loaded pools"},
	{"get-algorithms", NULL, "List currently loaded algorithms and their implementation"},
	{"get-counters", NULL, "List global or connection-specific counters for several IKE events"},
	{NULL, NULL, NULL}
};

static int get_max_command_length()
{
	int max = 0;
	int i;

	for (i = 0; commands[i].name; i++) {
		int cmd_len = strlen(commands[i].name);
		if (cmd_len > max)
			max = cmd_len;
	}

	return max;
}

static void print_command(FILE *out, int indent, int col_width,
						  struct command cmd)
{
	int desc_indent = col_width - strlen(cmd.name);

	while (indent--) {
		fprintf(out, " ");
	}
	fprintf(out, "%s", cmd.name);

	while (desc_indent--) {
		fprintf(out, " ");
	}
	fprintf(out, "%s\n", cmd.description);
}

static const struct command *find_command(const char *command_name) {
	int i = 0;

	while (commands[i].name) {
		if (strcmp(commands[i].name, command_name) == 0) {
			return &commands[i];
		}

		i++;
	}

	return NULL;
}

static void usage()
{
	int indent = 2;
	int col_width = get_max_command_length() + 2;
	int i;

	fprintf(stderr, "Usage: swanmon [COMMAND]\n"
		"commands:\n");
	print_command(stderr, indent, col_width, help_command);

	for (i = 0; commands[i].name; i++) {
		print_command(stderr, indent, col_width, commands[i]);
	}
}

int main(int argc, const char *argv[])
{
	const char *command_name;
	const struct command *command;

	if (argc < 2) {
		usage();
		return 1;
	}

	command_name = argv[1];
	if (strcmp(help_command.name, command_name) == 0) {
		usage();
		return 0;
	}

	command = find_command(command_name);
	if (!command) {
		usage();
		return 1;
	}

	return query_command(command->name, command->event_name);
}
