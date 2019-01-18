#ifdef WIN
#include "stdafx.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


void cp_str(char **to, char *from)
{
    char *s;

    if( !to ) return;

    /* free old string */
    if( *to ) free(*to);


    if( !from ) {
		*to= (char *)0;
		return;
    }

    /* malloc space for new token */
    if( !(s=(char *)malloc(strlen(from)+1 ))) {
		fprintf(stderr,"ERROR: can't allocate space for string\n");
		exit(-1);
    }

    *to= s;
    strcpy(s, from);
}

int scmp(char *s1, char *s2)
{
    if( !s1 ) return( 1 );
    if( !s2 ) return( 1 );
    return( strncmp(s1, s2, strlen(s1)) );
}

/* finds first non-whitespace character*/
char* skip_whitespace(char *s) {
      if (s == NULL) { return s; }
          for ( ; isspace((int) *s); s++);
              return s;
}

