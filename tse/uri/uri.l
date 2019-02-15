/**
 * uri.l -- Routines dealing with URI, mainly parsing and merging.
 * Created: Xie Han, net lab of Peking University. <e@pku.edu.cn>
 *
 * This is the first module of the web crawler. Used widely.
 * Created: Sep 25 04:15am 2003. version 0.1.1
 * Last updated: Oct 13 04:15am 2005. version 1.6.3
 */

/* The followings are BNFs generating URI-refernce, taken from RFC 2396. */
URI-reference	({absoluteURI}|{relativeURI})?("#"{fragment})?
absoluteURI		{scheme}":"({hier_part}|{opaque_part})
relativeURI		({net_path}|{abs_path}|{rel_path})("?"{query})?
hier_part		({net_path}|{abs_path})("?"{query})?
opaque_part		{uric_no_slash}{uric}*
uric_no_slash	{unreserved}|{escaped}|";"|"?"|":"|"@"|"&"|"="|"+"|"$"|","
net_path		"//"{authority}{abs_path}?
abs_path		"/"{path_segments}
rel_path		{rel_segment}{abs_path}?
rel_segment		({unreserved}|{escaped}|";"|"@"|"&"|"="|"+"|"$"|",")+
scheme			{alpha}({alpha}|{digit}|"+"|"-"|".")*
authority		{server}|{reg_name}
reg_name		({unreserved}|{escaped}|"$"|","|";"|":"|"@"|"&"|"="|"+")+
server			(({userinfo}"@")?{hostport})?
userinfo		({unreserved}|{escaped}|";"|":"|"&"|"="|"+"|"$"|",")*
hostport		{host}(":"{port})?
host			{hostname}|{IPv4address}
hostname		({domainlabel}".")*{toplabel}"."?
domainlabel		{alphanum}|{alphanum}({alphanum}|"-")*{alphanum}
toplabel		{alpha}|{alpha}({alphanum}|"-")*{alphanum}
IPv4address		{digit}+"."{digit}+"."{digit}+"."{digit}+
port			{digit}*

path			({abs_path}|{opaque_part})?
path_segments	{segment}("/"{segment})*
segment			{pchar}*(";"{param})*
param			{pchar}*
pchar			{unreserved}|{escaped}|":"|"@"|"&"|"="|"+"|"$"|","

query			{uric}*

fragment		{uric}*

uric			{reserved}|{unreserved}|{escaped}
reserved		";"|"/"|"?"|":"|"@"|"&"|"="|"+"|"$"|","
unreserved		{alphanum}|{mark}
mark			"-"|"_"|"."|"!"|"~"|"*"|"'"|"("|")"

escaped			"%"{hex}{hex}
hex				{digit}|[A-Fa-f]

alphanum		{alpha}|{digit}
alpha			{lowalpha}|{upalpha}

lowalpha		[a-z]
upalpha			[A-Z]
digit			[0-9]

%option stack

%s SCHEME REL_PATH AUTHORITY USERINFO HOST PORT REG_NAME ABS_PATH
%s OPAQUE_PART QUERY FRAGMENT ACCEPT

%{
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stack.h>
#include "uri.h"

#define URI_INIT(uri) \
do {											\
	(uri)->scheme = NULL;						\
	(uri)->authority = NULL;					\
	(uri)->path = NULL;							\
	(uri)->query = NULL;						\
	(uri)->fragment = NULL;						\
} while (0)

#define AUTH_INIT(auth, at) \
do {											\
	if (((auth)->type = (at)) == AT_SERVER)		\
	{											\
		(auth)->userinfo = NULL;				\
		(auth)->host = NULL;					\
		(auth)->port = NULL;					\
	}											\
	else										\
		(auth)->reg_name = NULL;				\
} while (0)

#define AUTH_DESTROY(auth) \
do {											\
	if ((auth)->type == AT_SERVER)				\
	{											\
		free((auth)->userinfo);					\
		free((auth)->host);						\
		free((auth)->port);						\
	}											\
	else										\
		free((auth)->reg_name);					\
} while (0)

static int __length;
static struct uri *__uri;

char *__memtostr(const void *s, int n)
{
	char *str = (char *)malloc((n + 1) * sizeof (char));

	if (str)
	{
		memcpy(str, s, n);
		*(str + n) = '\0';
	}

	return str;
}

%}

%%

<SCHEME>{scheme}":"		{
	if (__uri->scheme = __memtostr(yytext, yyleng - 1))
	{
		__length += yyleng;
		yy_push_state(AUTHORITY);
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<SCHEME>.|\n		{
	yyless(0);
	BEGIN REL_PATH;
}

<SCHEME><<EOF>>		BEGIN REL_PATH;

<REL_PATH>{rel_path}	{
	if (__uri->path = __memtostr(yytext, yyleng))
	{
		__length += yyleng;
		BEGIN QUERY;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<REL_PATH>.|\n		{
	yyless(0);
	yy_push_state(AUTHORITY);
}

<REL_PATH><<EOF>>	yy_push_state(AUTHORITY);

	/* Authority and abs_path have conflict! If the following is "//",
	 * we assume that it's an authority; if the following is "/", it's
	 * an abs_path. */
<AUTHORITY>"//"		{
	yy_pop_state();
	__uri->authority = (struct authority *)malloc(sizeof (struct authority));
	if (__uri->authority)
	{
		AUTH_INIT(__uri->authority, AT_SERVER);
		__length += yyleng;
		BEGIN USERINFO;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<AUTHORITY>.|\n		{
	yyless(0);
	yy_push_state(ABS_PATH);
}

<AUTHORITY><<EOF>>	yy_push_state(ABS_PATH);

<USERINFO>{userinfo}"@"		{
	if (__uri->authority->userinfo = __memtostr(yytext, yyleng - 1))
	{
		__length += yyleng;
		BEGIN HOST;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<USERINFO>.|\n		{
	yyless(0);
	BEGIN HOST;
}

<USERINFO><<EOF>>	BEGIN HOST;

<HOST>{host}		{
	if (__uri->authority->host = __memtostr(yytext, yyleng))
	{
		__length += yyleng;
		BEGIN PORT;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<HOST>.|\n			{
	yyless(0);
	BEGIN REG_NAME;
}

<HOST><<EOF>>		BEGIN REG_NAME;

<PORT>":"{port}		{
	if (__uri->authority->port = __memtostr(yytext + 1, yyleng - 1))
	{
		__length += yyleng;
		BEGIN REG_NAME;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<PORT>.|\n			{
	yyless(0);
	BEGIN REG_NAME;
}

<PORT><<EOF>>		BEGIN REG_NAME;

<REG_NAME>{reg_name}	{
	/* We have assumed that the authority is a server, but it seems that
	 * we are wrong: it's a reg_name. We should join the userinfo, host
	 * and the port together with this yytext into a reg_name. */
	char *reg_name;
	int len = yyleng;
	char *curpos;

	if (__uri->authority->userinfo)
		len += strlen(__uri->authority->userinfo) + 1;
	if (__uri->authority->host)
		len += strlen(__uri->authority->host);
	if (__uri->authority->port)
		len += strlen(__uri->authority->port) + 1;

	if (reg_name = (char *)malloc((len + 1) * sizeof (char)))
	{
		curpos = reg_name;
		if (__uri->authority->userinfo)
		{
			len = strlen(__uri->authority->userinfo);
			memcpy(curpos, __uri->authority->userinfo, len);
			curpos += len;
			*curpos++ = '@';
		}

		if (__uri->authority->host)
		{
			len = strlen(__uri->authority->host);
			memcpy(curpos, __uri->authority->host, len);
			curpos += len;
		}

		if (__uri->authority->port)
		{
			*curpos++ = ':';
			len = strlen(__uri->authority->port);
			memcpy(curpos, __uri->authority->port, len);
			curpos += len;
		}

		len = strlen(yytext);
		memcpy(curpos, yytext, len);
		curpos += len;
		*curpos = '\0';

		AUTH_DESTROY(__uri->authority);
		AUTH_INIT(__uri->authority, AT_REG_NAME);
		__uri->authority->reg_name = reg_name;

		__length += yyleng;
		yy_push_state(ABS_PATH);
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<REG_NAME>.|\n		{
	yyless(0);
	yy_push_state(ABS_PATH);
}

<REG_NAME><<EOF>>	yy_push_state(ABS_PATH);

<ABS_PATH>{abs_path}	{
	yy_pop_state();
	if (YY_START == AUTHORITY)
		yy_pop_state();
	if (__uri->path = __memtostr(yytext, yyleng))
	{
		__length += yyleng;
		BEGIN QUERY;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<ABS_PATH>.|\n		|
<ABS_PATH><<EOF>>	{
	/* When encountered an EOF we can not yyless. */
	if (yyleng == 1)
		yyless(0);
	yy_pop_state();

	/* The previous state is "AUTHORITY" indicates the URI
	 * has NO authority. */
	if (YY_START == AUTHORITY)
	{
		yy_pop_state();
		/* The previous state is "SCHEME" indicates the URI
		 * HAS a scheme. It's a little confusing. */
		if (YY_START == SCHEME)
			BEGIN OPAQUE_PART;
		else
			BEGIN FRAGMENT;
	}
	else
		BEGIN QUERY;
}

<OPAQUE_PART>{opaque_part}	{
	if (__uri->path = __memtostr(yytext, yyleng))
	{
		__length += yyleng;
		BEGIN FRAGMENT;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<OPAQUE_PART>.|\n		{
	yyless(0);
	BEGIN FRAGMENT;
}

<OPAQUE_PART><<EOF>>	BEGIN FRAGMENT;

<QUERY>"?"{query}	{
	if (__uri->query = __memtostr(yytext + 1, yyleng - 1))
	{
		__length += yyleng;
		BEGIN FRAGMENT;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<QUERY>.|\n			{
	yyless(0);
	BEGIN FRAGMENT;
}

<QUERY><<EOF>>		BEGIN FRAGMENT;

<FRAGMENT>"#"{fragment}		{
	if (__uri->fragment = __memtostr(yytext + 1, yyleng - 1))
	{
		__length += yyleng;
		BEGIN ACCEPT;
	}
	else
	{
		uri_destroy(__uri);
		return -1;
	}
}

<FRAGMENT>.|\n		{
	yyless(0);
	BEGIN ACCEPT;
}

<FRAGMENT><<EOF>>	BEGIN ACCEPT;

<ACCEPT>.|\n		{
	yyless(0);
	return __length;
}

<ACCEPT><<EOF>>		return __length;

<INITIAL>{URI-reference}	return yyleng;

<INITIAL>.|\n		{
	yyless(0);
	return 0;
}

<INITIAL><<EOF>>	return 0;

%%

int yywrap(void)
{
	return 1;
}

char __hex2char[] = {
/*  00 nul  01 soh   02 stx  03 etx   04 eot  05 enq   06 ack  07 bel   */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  08 bs   09 ht    0a nl   0b vt    0c np   0d cr    0e so   0f si    */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  10 dle  11 dc1   12 dc2  13 dc3   14 dc4  15 nak   16 syn  17 etb   */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  18 can  19 em    1a sub  1b esc   1c fs   1d gs    1e rs   1f us    */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  20 sp   21 !     22 "    23 #     24 $    25 %     26 &    27 '     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  28 (    29 )     2a *    2b +     2c ,    2d -     2e .    2f /     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  30 0    31 1     32 2    33 3     34 4    35 5     36 6    37 7     */
    0,      1,       2,      3,       4,      5,       6,      7,    
/*  38 8    39 9     3a :    3b ;     3c <    3d =     3e >    3f ?     */
    8,      9,       '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  40 @    41 A     42 B    43 C     44 D    45 E     46 F    47 G     */
    '\0',   10,      11,     12,      13,     14,      15,     '\0',    
/*  48 H    49 I     4a J    4b K     4c L    4d M     4e N    4f O     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  50 P    51 Q     52 R    53 S     54 T    55 U     56 V    57 W     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  58 X    59 Y     5a Z    5b [     5c \    5d ]     5e ^    5f _     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  60 `    61 a     62 b    63 c     64 d    65 e     66 f    67 g     */
    '\0',   10,      11,     12,      13,     14,      15,     '\0',    
/*  68 h    69 i     6a j    6b k     6c l    6d m     6e n    6f o     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  70 p    71 q     72 r    73 s     74 t    75 u     76 v    77 w     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  78 x    79 y     7a z    7b {     7c |    7d }     7e ~    7f del   */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
};

char __char2hex[] = "0123456789ABCDEF";

char __uri_chr[] = {
	0x00, 0x00, 0x00, 0x00,
	0x5b, 0xff, 0xff, 0xf5,
	0xff, 0xff, 0xff, 0xe1,
	0x7f, 0xff, 0xff, 0xe2
};

static int __uri_parse(struct uri *uri)
{
	__uri = uri;
	__length = 0;
	URI_INIT(__uri);
	BEGIN SCHEME;
	return yylex();
}

/* Scan a string ('\0' terminated) and return the length of the uri.
 * Return negative number when and only when failed to allocate memory. */
int uri_parse_string(const char *string, struct uri *uri)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_string(string))
	{
		yy_switch_to_buffer(buf);
		n = __uri_parse(uri);
		yy_delete_buffer(buf);
	}

	return n;
}

/* Scan some memory bytes. */
int uri_parse_bytes(const char *bytes, int len, struct uri *uri)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_bytes(bytes, len))
	{
		yy_switch_to_buffer(buf);
		n = __uri_parse(uri);
		yy_delete_buffer(buf);
	}

	return n;
}

/* Scan some memory bytes. The last two bytes of the memory MUST be '\0', or
 * the function will return -1 indicating a failure. This function has better
 * performance than "uri_parse_bytes", but note there is NO "const" key
 * word before the "base" argument, which means the content of memory may
 * be changed. */
int uri_parse_buffer(char *base, unsigned int size, struct uri *uri)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_buffer(base, size))
	{
		yy_switch_to_buffer(buf);
		n = __uri_parse(uri);
		yy_delete_buffer(buf);
	}

	return n;
}

void uri_destroy(struct uri *uri)
{
	free(uri->scheme);
	if (uri->authority)
	{
		AUTH_DESTROY(uri->authority);
		free(uri->authority);
	}
	free(uri->path);
	free(uri->query);
	free(uri->fragment);
}

static int __uri_length(void)
{
	BEGIN INITIAL;
	return yylex();
}

int uri_length_string(const char *string)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_string(string))
	{
		yy_switch_to_buffer(buf);
		n = __uri_length();
		yy_delete_buffer(buf);
	}

	return n;
}

int uri_length_bytes(const char *bytes, int len)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_bytes(bytes, len))
	{
		yy_switch_to_buffer(buf);
		n = __uri_length();
		yy_delete_buffer(buf);
	}

	return n;
}

int uri_length_buffer(char *base, unsigned int size)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_buffer(base, size))
	{
		yy_switch_to_buffer(buf);
		n = __uri_length();
		yy_delete_buffer(buf);
	}

	return n;
}

/* Merge two path. It sounds easy but indeed quite troublesome if you take
 * everything into consideration. Core of merging two URIs. */
int __path_merge(const char *rel_path, const char *base_path, char **result)
{
	stack_t *stack;
	const char *curpos;
	const char *next_slash;
	int len, seglen;

	/* This merging algorithm is different from RFC 2396, which uses string,
	 * while this algorithm uses stack. */
	if (!(stack = stack_create(STACK_INITIAL_SIZE)))
		return -1;

	/* The "base_path" and the "rel_path" are divided into segments and push
	 * all these segments and their length into the stack. If a segment
	 * is ".", ignore it; if a segment is "..", pop one segment out. */
	len = 0;
	for (seglen = 0; seglen < 2; seglen++)
	{
		/* Both "rel_path" and "base_path" can be NULL. */
		if (curpos = base_path)
		{
			while (next_slash = strchr(curpos, '/'))
			{
				if (strncmp(curpos, "../", next_slash - curpos + 1) == 0)
				{
					if (stack_height(stack) > sizeof (char *) + sizeof (int) ||
						!stack_empty(stack) && stack_top(int, stack) != 1)
					{
						len -= stack_pop(int, stack);
						stack_pop(const char *, stack);
					}
				}
				else if (strncmp(curpos, "./", next_slash - curpos + 1) != 0)
				{
					len += next_slash - curpos + 1;
					if (stack_push(const char *, curpos, stack) < 0 ||
						stack_push(int, next_slash - curpos + 1, stack) < 0)
					{
						stack_destroy(stack);
						return -1;
					}
				}

				curpos = next_slash + 1;
			}
	
			base_path = rel_path;
		}
	}

	/* This part deals with the "filename", which may be empty, may be "..",
	 * may be ".", or may be something else like "index.html". */
	if (curpos)
	{
		if (strcmp(curpos, "..") == 0)
		{
			if (stack_height(stack) > sizeof (char *) + sizeof (int) ||
				!stack_empty(stack) && stack_top(int, stack) != 1)
			{
				len -= stack_pop(int, stack);
				stack_pop(const char *, stack);
			}
		}
		else if (strcmp(curpos, ".") != 0)
		{
			len += strlen(curpos);
			if (stack_push(const char *, curpos, stack) < 0 ||
				stack_push(int, strlen(curpos), stack) < 0)
			{
				stack_destroy(stack);
				return -1;
			}
		}
	}

	/* Example:
	 * rel_path: "../././../game/../document/rfc/rfc2616.pdf"
	 * base_path: "/pub/incoming/./software/linux/nasm.tar.gz",
	 * Now the stack is:
	 *
	 *	+---------------+		<-- stack top
	 *	|	11			|
	 *	|---------------|
	 *	|	rfc2616.pdf	|
	 *	|---------------|
	 *	|	4			|
	 *	|---------------|
	 *	|	rfc/		|
	 *	|---------------|
	 *	|	9			|
	 *	|---------------|
	 *	|	document/	|
	 *	|---------------|
	 *	|	9			|
	 *	|---------------|
	 *	|	incoming/	|
	 *	|---------------|
	 *	|	4			|
	 *	|---------------|
	 *	|	pub/		|
	 *	|---------------|
	 *	|	1			|
	 *	|---------------|
	 *	|	/			|
	 *	+---------------+		<-- stack base
	 *
	 * len = 1 + 4 + 9 + 9 + 4 + 11 = ??
	 *
	 * Note that we do NOT copy the segments into the stack, we just push the
	 * pointers into the stack.
	 *
	 * All the information we need to compose the result path has been here.
	 */

	/* The result path is an "empty path". We should turn it into "no path".
	 * "no path" is allowed while "empty path" is illegal. */
	if (len == 0)
		*result = NULL;
	else if (*result = (char *)malloc((len + 1) * sizeof (char)))
	{
		*result += len;
		**result = '\0';
		while (!stack_empty(stack))
		{
			seglen = stack_pop(int, stack);
			*result -= seglen;
			memcpy(*result, stack_pop(const char *, stack), seglen);
		}
	}
	else
		len = -1;

	stack_destroy(stack);
	return len;
}

int uri_merge(const struct uri *rel_uri, const struct uri *base_uri,
			  struct uri *result)
{
	struct authority *tmp;
	int len, n;

	/* I am lazy. */
	#define __STRDUP(str) \
	({																	\
		char *__res;													\
		int __n;														\
		if (str)														\
		{																\
			__n = strlen(str);											\
			if (__res = __memtostr(str, __n))							\
				len += __n;												\
			else														\
				break;													\
		}																\
		else															\
			__res = NULL;												\
		__res;															\
	})

	/* The following macro is sooooooo big but it's extended only once
	 * and does not matter much. */
	#define __AUTH_DUP(auth) \
	({																	\
		struct authority *__res;										\
		if (auth)														\
		{																\
			if (__res = (struct authority *)							\
						malloc(sizeof (struct authority)))				\
			{															\
				AUTH_INIT(__res, (auth)->type);							\
				if ((auth)->type == AT_SERVER)							\
				{														\
					if (__res->userinfo = __STRDUP((auth)->userinfo))	\
						len++;											\
					__res->host = __STRDUP((auth)->host);				\
					if (__res->port = __STRDUP((auth)->port))			\
						len++;											\
				}														\
				else													\
					__res->reg_name = __STRDUP((auth)->reg_name);		\
				len += 2;												\
			}															\
			else														\
				break;													\
		}																\
		else															\
			__res = NULL;												\
		__res;															\
	})

	URI_INIT(result);
	len = 0;
	do {
		/* If the relative URI has a scheme, take it; else take the scheme
		 * of the base URI. */
		if (rel_uri->scheme)
		{
			result->scheme = __STRDUP(rel_uri->scheme);
			len++;
		}
		else if (result->scheme = __STRDUP(base_uri->scheme))
			len++;

		/* If the relative URI has a scheme or an authority, take it's
		 * authority; else take the authority of the base URI. */
		tmp = rel_uri->scheme || rel_uri->authority ? rel_uri->authority :
													  base_uri->authority;
		result->authority = __AUTH_DUP(tmp);

		/* If the relative URI has a scheme or an authority or an absolute
		 * path, take it's path; else if the relative URI does not have a
		 * path, take the base URI's path; else if base URI has a path,
		 * merge the relative URI's path with the base URI's path, and take
		 * the result; else if the base URI has no path, merge the relative
		 * URI's path with path "/" and take the result; no else. */
		if (rel_uri->scheme || rel_uri->authority ||
				rel_uri->path && *rel_uri->path == '/')
			result->path = __STRDUP(rel_uri->path);
		else if (!rel_uri->path)
			result->path = __STRDUP(base_uri->path);
		else if ((n = __path_merge(rel_uri->path, base_uri->path ?
								   base_uri->path : "/",
								   &result->path)) >= 0)
			len += n;
		else
			break;

		/* Query is taken from relative URI. */
		if (result->query = __STRDUP(rel_uri->query))
			len++;

		/* Fragment is taken from relative URI. */
		if (result->fragment = __STRDUP(rel_uri->fragment))
			len++;

		return len;
	} while (0);

	#undef __AUTH_DUP
	#undef __STRDUP
	uri_destroy(result);
	return -1;
}

/* Recombine a URI structure into a URI string. "flags" indicates what
 * component(s) you would like to appear in the result string. Note that
 * the result string is NOT necessarily a legal URI string (When you mask
 * some components) though the second argument has the name "uristr". */
int uri_recombine(const struct uri *uri, char *uristr, unsigned int n,
				  int flags)
{
	char *curpos = uristr;
	char *end = curpos + n;

	do {
		if (flags & C_SCHEME && uri->scheme)
		{
			n = strlen(uri->scheme);
			if (curpos + n + 1 < end)
			{
				memcpy(curpos, uri->scheme, n);
				curpos += n;
				*curpos++ = ':';
			}
			else
				break;
		}

		if (flags & C_AUTHORITY && uri->authority)
		{
			if (curpos + 2 < end)
			{
				*curpos++ = '/';
				*curpos++ = '/';
			}
			else
				break;

			if (uri->authority->type == AT_SERVER)
			{
				if (flags & C_USERINFO && uri->authority->userinfo)
				{
					n = strlen(uri->authority->userinfo);
					if (curpos + n + 1 < end)
					{
						memcpy(curpos, uri->authority->userinfo, n);
						curpos += n;
						*curpos++ = '@';
					}
					else
						break;
				}

				if (flags & C_HOST && uri->authority->host)
				{
					n = strlen(uri->authority->host);
					if (curpos + n < end)
					{
						memcpy(curpos, uri->authority->host, n);
						curpos += n;
					}
					else
						break;
				}

				if (flags & C_PORT && uri->authority->port)
				{
					n = strlen(uri->authority->port);
					if (curpos + n + 1 < end)
					{
						*curpos++ = ':';
						memcpy(curpos, uri->authority->port, n);
						curpos += n;
					}
					else
						break;
				}
			}
			else if (flags & C_REG_NAME && uri->authority->reg_name)
			{
				n = strlen(uri->authority->reg_name);
				if (curpos + n < end)
				{
					memcpy(curpos, uri->authority->reg_name, n);
					curpos += n;
				}
				else
					break;
			}
		}

		if (flags & C_PATH && uri->path)
		{
			n = strlen(uri->path);
			if (curpos + n < end)
			{
				memcpy(curpos, uri->path, n);
				curpos += n;
			}
			else
				break;
		}

		if (flags & C_QUERY && uri->query)
		{
			n = strlen(uri->query);
			if (curpos + n + 1 < end)
			{
				*curpos++ = '?';
				memcpy(curpos, uri->query, n);
				curpos += n;
			}
			else
				break;
		}

		if (flags & C_FRAGMENT && uri->fragment)
		{
			n = strlen(uri->fragment);
			if (curpos + n + 1 < end)
			{
				*curpos++ = '#';
				memcpy(curpos, uri->fragment, n);
				curpos += n;
			}
			else
				break;
		}

		if (curpos < end)
			*curpos = '\0';
		else
			break;

		return curpos - uristr;
	} while (0);

	errno = ENOSPC;
	return -1;
}

/* Turn some bytes into a string of escaped form. */
int uri_escape(const char *bytes, unsigned int len,
			   char *escstr, unsigned int n)
{
	const char *tmp = bytes + len;
	char *curpos = escstr;
	char *end = escstr + n;

	while (1)
	{
		if (bytes == tmp)
		{
			if (curpos < end)
			{
				*curpos = '\0';
				return curpos - escstr;
			}

			break;
		}

		if (curpos < end)
		{
			if (is_uri_chr(*bytes) || *bytes == '%' && bytes + 2 < tmp &&
					 isxdigit(*(bytes + 1)) && isxdigit(*(bytes + 2)))
				*curpos++ = *bytes;
			else if (curpos + 2 < end)
			{
				*curpos++ = '%';
				*curpos++ = char2hex((unsigned char)*bytes >> 4);
				*curpos++ = char2hex(*bytes & 0x0f);
			}
			else
				break;
		}
		else
			break;

		bytes++;
	}

	errno = ENOSPC;
	return -1;
}

