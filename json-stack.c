/*
 * Copyright (C) 2025 TDT AG <development@tdt.de>
 *
 * This is free software, licensed under the
 * GNU Lesser General Public License, Version 2.1.
 * See https://www.gnu.org/licenses/lgpl-2.1.txt for more information.
 *
 */

#include "json-stack.h"

#define MAX_SIZE 64

struct json_stack {
	json_object *objs[MAX_SIZE];
	int head;
};

json_stack *json_stack_new()
{
	json_stack *stack = malloc(sizeof(json_stack));
	stack->head = -1;
	return stack;
}

void json_stack_free(json_stack *stack)
{
	free(stack);
}

bool json_stack_empty(json_stack *stack)
{
	return stack->head == -1;
}

bool json_stack_full(json_stack *stack)
{
	return stack->head == MAX_SIZE - 1;
}

void json_stack_push(json_stack *stack, json_object *obj)
{
	if (json_stack_full(stack))
		return;

	stack->head++;
	stack->objs[stack->head] = obj;
}

json_object *json_stack_pop(json_stack *stack)
{
	if (json_stack_empty(stack))
		return 0;

	json_object *obj = stack->objs[stack->head];
	stack->head--;
	return obj;
}

json_object *json_stack_peek(json_stack *stack)
{
	if (json_stack_empty(stack))
		return 0;

	return stack->objs[stack->head];
}
