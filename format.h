/*
 * Copyright (C) 2023 TDT AG <development@tdt.de>
 *
 * This is free software, licensed under the
 * GNU Lesser General Public License, Version 2.1.
 * See https://www.gnu.org/licenses/lgpl-2.1.txt for more information.
 *
 */

#ifndef __FORMAT_H
#define __FORMAT_H

#include <stdbool.h>

#include <json-c/json.h>
#include <davici.h>

#define ERRORS_KEY "errors"
#define ERROR_MESSAGE_KEY "message"
#define DATA_KEY "data"

json_object *format_generate_response(bool is_event);
json_object *format_parse_error(const char *type, int err);
void format_dump_json(json_object *obj, FILE *out);
void format_add_error(json_object *root, json_object *error);
void format_set_data(json_object *root, json_object *data);
void format_add_data(json_object *root, json_object *data);
json_object *format_parse_davici_response(struct davici_response *res);

#endif
