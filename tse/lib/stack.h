#ifndef _STACK_H_
#define _STACK_H_

#define STACK_INITIAL_SIZE	256

/* Do NOT manipulate the __stack structure directly unless you know what
 * you are doning. */
struct __stack
{
	char *base;
	char *top;
	char *end;
};

typedef struct __stack stack_t;

#define stack_push(type, elem, stack) \
({															\
	(stack)->top + sizeof (type) <= (stack)->end ||			\
	__stack_expand(sizeof (type), stack) >= 0 ?				\
		*(type *)(stack)->top = (elem),						\
		(stack)->top += sizeof (type),						\
		(stack)->top - (stack)->base : -1;					\
})
#define stack_pop(type, stack)	(*(type *)((stack)->top -= sizeof (type)))
/*
#define stack_push(type, elem, stack) \
({																			\
	(stack)->top + sizeof (type) <= (stack)->end ||							\
		__stack_expand(sizeof (type), stack) >= 0 ?							\
	*((type *)(stack)->top)++ = (elem), (stack)->top - (stack)->base : -1;	\
})
#define stack_pop(type, stack)	(*--(type *)(stack)->top)
*/
#define stack_top(type, stack)	(*((type *)(stack)->top - 1))
#define stack_height(stack)		((stack)->top - (stack)->base)
#define stack_empty(stack)		((stack)->base == (stack)->top)

#ifdef __cplusplus
extern "C"
{
#endif

stack_t *stack_create(unsigned int init_size);
void stack_destroy(stack_t *stack);

/* Never call the following function directly. */
int __stack_expand(unsigned int elem_size, stack_t *stack);

#ifdef __cplusplus
}
#endif

#endif

