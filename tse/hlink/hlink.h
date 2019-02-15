#ifndef _HLINK_H_
#define _HLINK_H_

#include <uri.h>

typedef int (*onfind_t)(const char *, const char *, struct uri *, void *);

#ifdef __cplusplus

#include <iostream>
using namespace std;
int HLinkDetect(istream *PageFile, const struct uri *PageURI,
				onfind_t OnFind, void *arg);

#else

#include <stdio.h>
int hlink_detect(FILE *pg_file, const struct uri *pg_uri,
				 onfind_t onfind, void *arg);

#endif

#ifdef __cplusplus
extern "C"
{
#endif

int hlink_detect_string(const char *string, const struct uri *pg_uri,
						onfind_t onfind, void *arg);

#ifdef __cplusplus
}
#endif

#endif

