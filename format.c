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
#include <stdlib.h>

#include <glib.h>

#include "format.h"

json_object *format_generate_response(bool is_event)
{
	json_object *root = json_object_new_object();

	json_object_object_add(root, ERRORS_KEY, json_object_new_array());
	if (is_event)
		json_object_object_add(root, DATA_KEY, json_object_new_array());

	return root;
}

json_object *format_parse_error(const char *type, int err)
{
	json_object *error = json_object_new_object();

	if (type) {
		char *message;
		asprintf(&message, "%s: %s", type, strerror(err));

		json_object_object_add(error, ERROR_MESSAGE_KEY,
			json_object_new_string(message));

		free(message);
	} else
		json_object_object_add(error, ERROR_MESSAGE_KEY,
			json_object_new_string(strerror(err)));

	return error;
}

void format_dump_json(json_object *obj, FILE *out)
{
	fprintf(out, "%s\n", json_object_to_json_string_ext(
		obj, JSON_C_TO_STRING_PLAIN));
}

void format_add_error(json_object *root, json_object *error)
{
	json_object *error_arr;

	json_object_object_get_ex(root, ERRORS_KEY, &error_arr);
	json_object_array_add(error_arr, error);
}

void format_set_data(json_object *root, json_object *data)
{
	json_object *data_obj;

	json_object_object_get_ex(root, DATA_KEY, &data_obj);

	if (!data_obj)
		json_object_object_add(root, DATA_KEY, data);
}

void format_add_data(json_object *root, json_object *data)
{
	json_object *data_arr;
	json_object_object_get_ex(root, DATA_KEY, &data_arr);
	json_object_array_add(data_arr, data);
}

json_object *format_parse_davici_response(struct davici_response *res)
{
	GQueue* queue = g_queue_new();
	json_object *root = json_object_new_object();
	json_object *current;
	json_object *next;
	const char *name;
	char buf[4096];
	int ret;

	g_queue_push_head(queue, root);

	while (true) {
		ret = davici_parse(res);
		switch (ret) {
			case DAVICI_END:
				g_queue_free(queue);
				return root;
			case DAVICI_SECTION_START:
				current = g_queue_peek_head(queue);
				next = json_object_new_object();
				name = davici_get_name(res);
				json_object_object_add(current, name, next);
				g_queue_push_head(queue, next);
				break;
			case DAVICI_SECTION_END:
				g_queue_pop_head(queue);
				break;
			case DAVICI_KEY_VALUE:
				ret = davici_get_value_str(res, buf, sizeof(buf));
				if (ret < 0) {
					g_queue_free(queue);
					return root;
				}

				name = davici_get_name(res);
				current = g_queue_peek_head(queue);
				json_object_object_add(current, name, json_object_new_string(buf));
				break;
			case DAVICI_LIST_START:
				current = g_queue_peek_head(queue);
				next = json_object_new_array();
				name = davici_get_name(res);
				json_object_object_add(current, name, next);
				g_queue_push_head(queue, next);
				break;
			case DAVICI_LIST_ITEM:
				ret = davici_get_value_str(res, buf, sizeof(buf));
				if (ret < 0) {
					g_queue_free(queue);
					return root;
				}

				current = g_queue_peek_head(queue);
				json_object_array_add(current, json_object_new_string(buf));
				break;
			case DAVICI_LIST_END:
				g_queue_pop_head(queue);
				break;
			default:
				g_queue_free(queue);
				return root;
		}
	}
}
