/**
 * stack.c -- Implementation of stack that can store anything.
 * Create: Xie Han, net lab of Peking University <e@pku.edu.cn>
 *
 * Simple is the best!
 *
 * Created: Sep 30 1:10am 2003. version 1.0.0
 * Last updated: Apr 6 9:05pm 2005. version 2.2.1
 */
#include <stdlib.h>
#include "stack.h"

stack_t *stack_create(unsigned int init_size)
{
	stack_t *stack = (stack_t *)malloc(sizeof (stack_t));

	if (stack)
	{
		if (stack->base = (char *)malloc(init_size))
		{
			stack->top = stack->base;
			stack->end = stack->base + init_size;
		}
		else
		{
			free(stack);
			stack = NULL;
		}
	}

	return stack;
}

int __stack_expand(unsigned int elem_size, stack_t *stack)
{
	unsigned int new_size = stack->end - stack->base;
	char *new_base;

	if (new_size > 0)
		new_size <<= 1;
	else
		new_size = STACK_INITIAL_SIZE;

	while (stack->top + elem_size > stack->base + new_size)
		new_size <<= 1;

	if (new_base = (char *)realloc(stack->base, new_size))
	{
		stack->top = stack->top - stack->base + new_base;
		stack->base = new_base;
		stack->end = stack->base + new_size;
		return new_size;
	}

	return -1;
}

void stack_destroy(stack_t *stack)
{
	free(stack->base);
	free(stack);
}

