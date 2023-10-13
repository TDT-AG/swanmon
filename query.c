/*
 * Copyright (C) 2023 TDT AG <development@tdt.de>
 *
 * This is free software, licensed under the
 * GNU Lesser General Public License, Version 2.1.
 * See https://www.gnu.org/licenses/lgpl-2.1.txt for more information.
 *
 */

#include <stdlib.h>
#include <errno.h>
#include <poll.h>

#include <json-c/json.h>
#include <davici.h>

#include "query.h"
#include "format.h"

struct dispatch_data {
	struct pollfd pfd;
	struct davici_conn *c;
	bool is_event;
	json_object *response;
	int pending;
};

static int fd_cb(struct davici_conn *conn, int fd, int ops, void *user)
{
	struct dispatch_data *dd = user;
	short events = 0;

	if (ops & DAVICI_READ)
		events |= POLLIN;
	if (ops & DAVICI_WRITE)
		events |= POLLOUT;

	dd->pfd.events = events;
	if (dd->pfd.events)
		dd->pfd.fd = fd;
	else
		dd->pfd.fd = -1;

	return 0;
}

static void command_cb(struct davici_conn *conn, int err, const char *name,
			 		   struct davici_response *res, void *user)
{
	struct dispatch_data *dd = user;

	dd->pending--;

	if (err < 0) {
		json_object *error = format_parse_error(name, -err);
		format_add_error(dd->response, error);
	} else if (res && !dd->is_event) {
		json_object *data = format_parse_davici_response(res);
		format_set_data(dd->response, data);
	}
}

static void stream_cb(struct davici_conn *conn, int err, const char *name,
			 		  struct davici_response *res, void *user)
{
	struct dispatch_data *dd = user;

	if (err < 0) {
		json_object *error = format_parse_error(name, -err);
		format_add_error(dd->response, error);
	} else if (res) {
		json_object *obj = format_parse_davici_response(res);
		format_add_data(dd->response, obj);
	}
}

static int execute(struct dispatch_data *dd, const char *cmd,
				   const char *event)
{
	struct davici_request *request;
	int err;

	err = davici_new_cmd(cmd, &request);
	if (err < 0)
		return err;

	if (!dd->is_event)
		err = davici_queue(dd->c, request, command_cb, dd);
	else
		err = davici_queue_streamed(dd->c, request, command_cb,
			event, stream_cb, dd);

	if (err < 0)
		return err;

	dd->pending++;

	return 0;
}

static int dispatch(struct dispatch_data *dd)
{
	int err, ret;

	while (dd->pending && dd->pfd.fd != -1) {
		ret = poll(&dd->pfd, 1, -1);
		if (ret < 0)
			return -errno;
		if (ret != 1)
			return -EIO;

		if (dd->pfd.revents & POLLIN) {
			err = davici_read(dd->c);
			if (err < 0)
				return err;
		}
		if (dd->pfd.revents & POLLOUT) {
			err = davici_write(dd->c);
			if (err < 0)
				return err;
		}
	}

	return 0;
}

int query_command(const char *command, const char *event)
{
	bool is_event = !!event;

	struct dispatch_data dd = {
		.pfd = {
			.fd = -1,
		},
		.is_event = is_event,
		.response = format_generate_response(is_event)
	};
	int ret = 0;
	int err;

	err = davici_connect_unix(VICI_SOCKET, fd_cb, &dd, &dd.c);
	if (err < 0) {
		json_object *error = format_parse_error(
			"Connecting failed", -err);
		format_add_error(dd.response, error);
		ret = -err;
		goto out;
	}

	err = execute(&dd, command, event);
	if (err < 0) {
		json_object *error = format_parse_error(
			"Executing failed", -err);
		format_add_error(dd.response, error);
		ret = -err;
		goto out;
	}

	err = dispatch(&dd);
	if (err < 0) {
		json_object *error = format_parse_error(
			"Dispatching failed", -err);
		format_add_error(dd.response, error);
		ret = -err;
		goto out;
	}

	davici_disconnect(dd.c);

	out:
		format_dump_json(dd.response, stdout);
		json_object_put(dd.response);

		return ret;
}
