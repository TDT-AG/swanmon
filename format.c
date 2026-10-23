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
#include <errno.h>
#include <wctype.h>

#include "format.h"
#include "json-stack.h"

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

	/* errors invalidate any (partial) data */
	json_object_object_add(root, DATA_KEY, json_object_new_array());
}

void format_set_data(json_object *root, json_object *data)
{
	if (json_object_object_get_ex(root, DATA_KEY, NULL)) {
		json_object_put(data);
		return;
	}
	json_object_object_add(root, DATA_KEY, data);
}

void format_add_data(json_object *root, json_object *data)
{
	json_object *data_arr;
	json_object_object_get_ex(root, DATA_KEY, &data_arr);
	json_object_array_add(data_arr, data);
}

static bool is_text(const char *s, size_t len)
{
	wchar_t wc;
	int n;

	while (len) {
		n = mbtowc(&wc, s, len);
		if (n <= 0 || !iswprint(wc))
			return false;
		s += n;
		len -= n;
	}

	return true;
}

static int format_value(struct davici_response *res, json_object **out)
{
	unsigned int len;
	const char *val = davici_get_value(res, &len);

	if (!is_text(val, len))
		return -EINVAL;

	*out = json_object_new_string_len(val, len);
	return 0;
}

int format_parse_davici_response(struct davici_response *res,
	json_object **out)
{
	json_stack *stack = json_stack_new();
	json_object *root = json_object_new_object();
	json_object *current;
	json_object *next;
	json_object *value;
	const char *name;
	int ret;
	int err = 0;

	json_stack_push(stack, root);

	while (true) {
		ret = davici_parse(res);
		switch (ret) {
			case DAVICI_END:
				goto cleanup;
			case DAVICI_SECTION_START:
				current = json_stack_peek(stack);
				if (current == 0) {
					err = -EBADMSG;
					goto cleanup;
				}
				next = json_object_new_object();
				name = davici_get_name(res);
				json_object_object_add(current, name, next);
				json_stack_push(stack, next);
				break;
			case DAVICI_SECTION_END:
				json_stack_pop(stack);
				break;
			case DAVICI_KEY_VALUE:
				err = format_value(res, &value);
				if (err < 0)
					goto cleanup;

				name = davici_get_name(res);
				current = json_stack_peek(stack);
				if (current == 0) {
					json_object_put(value);
					err = -EBADMSG;
					goto cleanup;
				}
				json_object_object_add(current, name, value);
				break;
			case DAVICI_LIST_START:
				current = json_stack_peek(stack);
				if (current == 0) {
					err = -EBADMSG;
					goto cleanup;
				}
				next = json_object_new_array();
				name = davici_get_name(res);
				json_object_object_add(current, name, next);
				json_stack_push(stack, next);
				break;
			case DAVICI_LIST_ITEM:
				err = format_value(res, &value);
				if (err < 0)
					goto cleanup;

				current = json_stack_peek(stack);
				if (current == 0) {
					json_object_put(value);
					err = -EBADMSG;
					goto cleanup;
				}
				json_object_array_add(current, value);
				break;
			case DAVICI_LIST_END:
				json_stack_pop(stack);
				break;
			default:
				err = ret < 0 ? ret : -EBADMSG;
				goto cleanup;
		}
	}

cleanup:
	json_stack_free(stack);
	if (err < 0) {
		json_object_put(root);
		*out = NULL;
	} else
		*out = root;

	return err;
}
