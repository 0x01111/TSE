//#include <stdlib.h>
//#include <stdio.h>
//#include <fcntl.h>
//#include <string.h>
//#include <errno.h>
//#include <sys/param.h>
//#include <sys/stat.h>

#include <iostream>
#include <cstring>

#define MAXDIR	40
#define USER_AGENT  "TSE/1.0"


char *my_getword( char *word, char *line, char stop)
{
	/* skip blanks */
	int i;
	while ( *line == ' ' || *line == '\b' ) line++;
	i = 0;
	while ( *line && *line != stop && *line != '\0') {
		if ( i < 255 ) *word++ = *line++;
		else line++;
		i++;
	}
	*word = '\0';
	return line+1; /* skip the stop char */
}

char *my_getline( char *line, char *data )
{
	return my_getword ( line, data, '\n');
}

char **ParseRobot( char *data, char len)
{
	static char *dir [ MAXDIR ];
	char		mline [ 256 ], word [ 256 ];
	int 		num = 0, match = 0;
	char		*line = mline;

	for (;;){
		line = mline;
		data = my_getline( line, data );

		/* process a line */
		line = my_getword( word, line , ':');
		if ( word[0] == '\0' ) continue;
		if ( !strncmp( word, "User-agent",10) ){
			line = my_getword( word, line,' ');
			/*Commented by :'*' matches all kinds of agents*/
			if ( word[0] == '*' ) match = 1;
			else match = 0;
			if ( !strcmp( word, USER_AGENT ) ){ 
				match = 1;
				num = 0;   /* For special user agent */
			}
		}
		else  if ( !strncmp( word, "Disallow" ,8) ){
			if (match == 1 ) {
				line = my_getword( word, line , ' ');
				if ( num < MAXDIR ) 
					dir[ num++ ] = strdup( word ); 
			}
		}
		if ( strlen( data ) == 0 ) break;
	}

	if ( num < MAXDIR ) dir[num] = NULL;
	else dir[num - 1] = NULL;
	
	return dir;
}

