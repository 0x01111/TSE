#ifndef _MISC_H_
#define _MISC_H_

/* Copy one word once and then copy one byte once. */
#define MEMCPY_PLUS_PLUS(dest, src, n) \
({													\
	void *__end = (char *)(dest) + (n);				\
	while ((long *)(dest) + 1 <= (long *)__end)		\
		*((long *)(dest))++ = *((long *)(src))++;	\
	while ((char *)(dest) + 1 <= (char *)__end)		\
		*((char *)(dest))++ = *((char *)(src))++;	\
	dest;											\
})

#define MEMCPY_PLUS(dest, src, n) \
({													\
	const void *__src = (src);						\
	MEMCPY_PLUS_PLUS(dest, __src, n);				\
})

#define FREE_NOT_NULL(ptr) \
do {												\
	if (ptr) free(ptr);								\
} while (0)

#define MIN(x, y)		((x) < (y) ? (x) : (y))
#define MAX(x, y)		((x) > (y) ? (x) : (y))

#ifdef __cplusplus
extern "C"
{
#endif

char *strdupn(const char *string, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif

