/*
 * hlink.l -- Function for detecting hyper links in html file.
 * Created: Xie Han, net lab of Peking University. <me@pku.edu>
 */
blank			[ \t\r\n]
cdata			[A-Za-z][A-Za-z0-9\-_:.]*
hex				{digit}|[A-Fa-f]
digit			[0-9]

%option stack

%s COMMENT SCRIPT ATTRIBUTE IGNORE_ALL IGNORE IGNORE_DOUBLE_QUOTED
%s IGNORE_SINGLE_QUOTED IGNORE_UNQUOTED URI UNQUOTED DOUBLE_QUOTED
%s SINGLE_QUOTED ENTITY

%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <uri.h>
#include "hlink.h"

#define URI_LEN_MAX			1024
#define HLINK_ISBLANK(c) \
({																\
	char tmp = (c);												\
	tmp == ' ' || tmp == '\n' || tmp == '\r' || tmp == '\t';	\
})

static struct uri *__base_uri;
static int __is_our_base;
static onfind_t __onfind;
static void *__arg;
static char __buffer[URI_LEN_MAX + 2];
static char *__curpos;
static int __refresh;

struct __elem
{
	char *name;
	char **attrs;
};

static char *__elem_a_attr[] = {"href", NULL};
static char *__elem_area_attr[] = {"href", NULL};
static char *__elem_base_attr[] = {"href", NULL};
static char *__elem_frame_attr[] = {"src", NULL};
static char *__elem_iframe_attr[] = {"src", NULL};
static char *__elem_img_attr[] = {"src", NULL};
static char *__elem_link_attr[] = {"href", NULL};
static char *__elem_meta_attr[] = {"http-equiv", "content", NULL};

static struct __elem __elems[] = {
	{"A", __elem_a_attr},
	{"AREA", __elem_area_attr},
	{"BASE", __elem_base_attr},
	{"FRAME", __elem_frame_attr},
	{"IFRAME", __elem_iframe_attr},
	{"IMG", __elem_img_attr},
	{"LINK", __elem_link_attr},
	{"META", __elem_meta_attr},
	{NULL, }
};

static const struct __elem *__cur_elem;
static char *__cur_attr;
%}

%%

<INITIAL>"<!--"		BEGIN COMMENT;

<INITIAL>"</"{cdata}">"

<INITIAL>.|\n

<INITIAL><<EOF>>	{
	if (__is_our_base)
	{
		uri_destroy(__base_uri);
		free(__base_uri);
	}

	return 0;
}

<COMMENT,SCRIPT>.|\n

<SCRIPT>"</"{cdata}">"	{
	yytext[yyleng - 1] = '\0';
	if (strcasecmp(yytext + 2, "SCRIPT") == 0)
		BEGIN INITIAL;
}

<COMMENT>"-->"				|
<COMMENT,SCRIPT><<EOF>>		BEGIN INITIAL;

<INITIAL>"<"{cdata}/{blank}|">"		{
	/* Element names are case-insensitive. */
	for (yyleng = 0; __elems[yyleng].name; yyleng++)
	{
		if (strcasecmp(yytext + 1, __elems[yyleng].name) == 0)
		{
			__cur_elem = __elems + yyleng;
			break;
		}
	}

	if (strcasecmp(yytext + 1, "SCRIPT") == 0)
		BEGIN SCRIPT;
	else
		BEGIN INITIAL;

	if (__elems[yyleng].name)
	{
		if (strcasecmp(__elems[yyleng].name, "META") == 0)
			__refresh = 0;
		yy_push_state(ATTRIBUTE);
	}
	else
		yy_push_state(IGNORE_ALL);
}

<ATTRIBUTE>{cdata}{blank}{0,512}"="{blank}{0,512}	{
	/* Atrribute names are case-insensitive. */
	yyleng = 0;
	while (!HLINK_ISBLANK(yytext[yyleng]) && yytext[yyleng] != '=')
		 yyleng++;
	yytext[yyleng] = '\0';

	for (yyleng = 0; __cur_elem->attrs[yyleng]; yyleng++)
	{
		if (strcasecmp(yytext, __cur_elem->attrs[yyleng]) == 0)
		{
			__curpos = __buffer;
			__cur_attr = __cur_elem->attrs[yyleng];
			break;
		}
	}

	if (__cur_elem->attrs[yyleng])
		BEGIN URI;
	else
		yy_push_state(IGNORE);
}

<IGNORE_ALL>{cdata}{blank}{0,512}"="{blank}{0,512}	yy_push_state(IGNORE);

<ATTRIBUTE,IGNORE_ALL>[^<>]

<ATTRIBUTE,IGNORE_ALL>.|\n		|
<ATTRIBUTE,IGNORE_ALL><<EOF>>	{
	if (*yytext == '<')
		yyless(0);
	yy_pop_state();
}

<IGNORE>\"			BEGIN IGNORE_DOUBLE_QUOTED;

<IGNORE>"'"			BEGIN IGNORE_SINGLE_QUOTED;

<IGNORE>.|\n		{
	yyless(0);
	BEGIN IGNORE_UNQUOTED;
}

<IGNORE_DOUBLE_QUOTED>\"			|
<IGNORE_SINGLE_QUOTED>"'"		|
<IGNORE_UNQUOTED>{blank}|">"	{
	if (*yytext == '>')
		yyless(0);
	yy_pop_state();
}

<IGNORE_DOUBLE_QUOTED,IGNORE_SINGLE_QUOTED,IGNORE_UNQUOTED>.|\n

<IGNORE,IGNORE_DOUBLE_QUOTED,IGNORE_SINGLE_QUOTED,IGNORE_UNQUOTED><<EOF>>	{
	yy_pop_state();
}

<URI>\"{blank}{0,512}	BEGIN DOUBLE_QUOTED;

<URI>"'"{blank}{0,512}	BEGIN SINGLE_QUOTED;

<URI>.|\n			{
	yyless(0);
	BEGIN UNQUOTED;
}

<URI><<EOF>>		BEGIN ATTRIBUTE;

<DOUBLE_QUOTED,SINGLE_QUOTED>\r|\n

<DOUBLE_QUOTED>{blank}{0,512}\"		|
<SINGLE_QUOTED>{blank}{0,512}"'"	|
<UNQUOTED>{blank}|">"				{
	struct uri uri;
	struct uri *result;
	char *ptr;
	int n;

	BEGIN ATTRIBUTE;

	/* If a URI is unquoted, put back the trailing '>'. */
	if (*yytext == '>')
		yyless(0);

	/* Last two characters MUST be "\0". */
	*(__curpos + 1) = *__curpos = '\0';

	/* Element META is a special case. */
	if (strcasecmp(__cur_elem->name, "META") == 0)
	{
		if (strcasecmp(__cur_attr, "http-equiv") == 0)
		{
			if (strcasecmp(__buffer, "refresh") == 0)
				__refresh = 1;

			YY_BREAK
		}

		if (__refresh && strcasecmp(__cur_attr, "content") == 0)
		{
			if (ptr = strchr(__buffer, '='))
				ptr++;
			else
				YY_BREAK
		}
		else
			YY_BREAK
	}
	else
		ptr = __buffer;

	yyleng = uri_parse_buffer(ptr, __curpos - ptr + 2, &uri);
	if (yyleng >= 0)
	{
		if (yyleng == __curpos - ptr)
		{
			if (result = (struct uri *)malloc(sizeof (struct uri)))
				n = uri_merge(&uri, __base_uri, result);
		}

		uri_destroy(&uri);
		if (yyleng == __curpos - ptr)
		{
			if (result)
			{
				if (n >= 0)
				{
					if (strcasecmp(__cur_elem->name, "BASE") == 0 &&
						strcasecmp(__cur_attr, "href") == 0)
					{
						if (__is_our_base)
						{
							uri_destroy(__base_uri);
							free(__base_uri);
						}
						else
							__is_our_base = 1;

						__base_uri = result;
						YY_BREAK
					}

					if (__onfind(__cur_elem->name, __cur_attr,
								 result, __arg) >= 0)
						YY_BREAK
				}
				else
					free(result);
			}
		}
		else
			YY_BREAK
	}

	/* Failed! Stop scanning and return -1. Possibilities of failure:
	 * failed to parse URI; failed to allocate memory for "result";
	 * failed to merge the relative URI with the base URI; "onfind"
	 * function return negative number. */
	yy_pop_state();
	if (__is_our_base)
	{
		uri_destroy(__base_uri);
		free(__base_uri);
	}

	return -1;
}

<UNQUOTED,DOUBLE_QUOTED,SINGLE_QUOTED>"&#"{digit}{1,10}";"		|
<UNQUOTED,DOUBLE_QUOTED,SINGLE_QUOTED>"&#"(X|x){hex}{1,8}";"	{
    unsigned int code;

	yytext[yyleng - 1] = '\0';
	if (yytext[2] == 'X' || yytext[2] == 'x')
		sscanf(yytext + 3, "%x", &code);
	else
		code = atoi(yytext + 2);

	do
	{
		unput(code & 0xff);
		yy_push_state(ENTITY);
	} while ((code >>= 8) > 0);
}

<UNQUOTED,DOUBLE_QUOTED,SINGLE_QUOTED>"&lt;"		{
	unput('<');
	yy_push_state(ENTITY);
}

<UNQUOTED,DOUBLE_QUOTED,SINGLE_QUOTED>"&gt;"		{
	unput('>');
	yy_push_state(ENTITY);
}

<UNQUOTED,DOUBLE_QUOTED,SINGLE_QUOTED>"&amp;"		{
	unput('&');
	yy_push_state(ENTITY);
}

<UNQUOTED,DOUBLE_QUOTED,SINGLE_QUOTED>"&quot;"		{
	unput('"');
	yy_push_state(ENTITY);
}

<UNQUOTED,DOUBLE_QUOTED,SINGLE_QUOTED,ENTITY>.|\n	{
	if (YY_START == ENTITY)
		yy_pop_state();

	if ((is_uri_chr(*yytext) || *yytext == '%') &&
			__curpos < __buffer + URI_LEN_MAX)
		*__curpos++ = *yytext;
	else if (__curpos + 2 < __buffer + URI_LEN_MAX)
	{
		sprintf(__curpos, "%%%X%X", (unsigned char)*yytext >> 4,
				*yytext & 0x0f);
		__curpos += 3;
	}
	else
	{
		/* The URI is soooooooo long! It's more likely than the page has
		 * grammar error. Return to initial state and go on scanning */
		while (YY_START == ENTITY)
		{
#ifdef __cplusplus
			yyinput();
#else
			input();
#endif
			yy_pop_state();
		}

		yy_pop_state();
		BEGIN INITIAL;
	}
}

<UNQUOTED,DOUBLE_QUOTED,SINGLE_QUOTED><<EOF>>	BEGIN ATTRIBUTE;

%%

int yywrap(void)
{
	return 1;
}

#ifdef __cplusplus

#include <iostream>
using namespace std;

int HLinkDetect(istream *PageFile, const struct uri *PageURI,
				onfind_t OnFind, void *arg)
{
	FlexLexer* lexer = new yyFlexLexer;
	int n = -1;

	if (lexer)
	{
		__base_uri = (struct uri *)PageURI;
		__is_our_base = 0;
		__onfind = OnFind;
		__arg = arg;

		n = lexer->yylex(PageFile);
		delete lexer;
	}

	return n;
}

#else

int hlink_detect(FILE *pg_file, const struct uri *pg_uri,
				 onfind_t onfind, void *arg)
{
	yyin = pg_file;
	__base_uri = (struct uri *)pg_uri;
	__is_our_base = 0;
	__onfind = onfind;
	__arg = arg;

	BEGIN INITIAL;
	return yylex();
}

#endif

int hlink_detect_string(const char *string, const struct uri *pg_uri,
						onfind_t onfind, void *arg)
{
	YY_BUFFER_STATE buf;
	int n = -1;

	if (buf = yy_scan_string(string))
	{
		yy_switch_to_buffer(buf);
		__base_uri = (struct uri *)pg_uri;
		__is_our_base = 0;
		__onfind = onfind;
		__arg = arg;

		BEGIN INITIAL;
		n = yylex();
		yy_delete_buffer(buf);
	}

	return n;
}
