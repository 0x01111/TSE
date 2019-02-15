#include <string.h>
#include "misc.h"

char *strdupn(const char *string, unsigned int n)
{
	char *res = (char *)malloc((n + 1) * sizeof (char));

	if (res)
	{
		memcpy(res, string, n);
		*(res + n) = '\0';
	}

	return res;
}

