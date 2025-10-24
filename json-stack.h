/*
 * Copyright (C) 2025 TDT AG <development@tdt.de>
 *
 * This is free software, licensed under the
 * GNU Lesser General Public License, Version 2.1.
 * See https://www.gnu.org/licenses/lgpl-2.1.txt for more information.
 *
 */

#ifndef __JSON_STACK_H
#define __JSON_STACK_H

#include <stdbool.h>

#include <json-c/json.h>

typedef struct json_stack json_stack;

json_stack *json_stack_new();
void json_stack_free(json_stack *stack);
bool json_stack_empty(json_stack *stack);
bool json_stack_full(json_stack *stack);
void json_stack_push(json_stack *stack, json_object *obj);
json_object *json_stack_pop(json_stack *stack);
json_object *json_stack_peek(json_stack *stack);

#endif
