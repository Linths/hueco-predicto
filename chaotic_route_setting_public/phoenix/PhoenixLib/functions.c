#ifdef WIN
#include "stdafx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "phoenix.h"


#define scmp(s1,s2)	strncmp(s1, s2, strlen(s1) )
#define get_slot(tree, slot)   sscanf(tree, "%[^.\n]", slot)


/* Array of slot mapping functions */
struct x_fun {
  const char *name;
  void (*fun)(char *str);
};

struct s_fun {
  const char *name;
  void (*fun)(char *slot, char *token, char *val);
};



int find_fun(char *slot);
int get_token(char *str, char *tok);

/* transform functions */



/*  state manipulation functions */

void yes(char *slot, char *token, char *val)
{

}

void help(char *slot, char *token, char *val)
{

}

void null() {};

/** Function Table **/

/* array to associate nets with their mapping function */
const struct x_fun xform[] = {
  {NULL, 0}
};
  
/* array to associate slots with their state change function */
const struct s_fun state_fun[] = {
  {"[Yes]", 		yes},
  {"[Help]", 		help},
  {NULL, 0}
};
  

/* check to see if any string transform should apply */
void check_transform( char *tree)
{
    int	i;
    char token[LABEL_LEN];

    get_token(tree, token);

    /* see if xform function associated with token */
    for(i=0; 1; i++) if( !xform[i].name || !strcmp(xform[i].name, token)) break;
    if( !xform[i].name ) return;

    /* execute transform function */
    if( xform[i].fun ) { (*xform[i].fun)(tree); }
}

/* check to see if any extraction function should be executed */
void check_function(char *frame_name, char *key,  char *val)
{
    int i;
    char    slot[LABEL_LEN];

    get_slot(key, slot);
    if( (i= find_fun(slot)) < 0 ) return;
    /* execute function for slot */
    if( state_fun[i].fun ) { (*state_fun[i].fun)(frame_name, key,  val); }
}



int find_fun(char *slot)
{
    int i;

    for(i=0; 1; i++) {
	if( !state_fun[i].name ) break;
	if( !strcmp(state_fun[i].name, slot) ) break;
    }
    if( !state_fun[i].name ) return(-1);
    return(i);
}

/* copy next token string into tok */
int get_token(char *str, char *tok)
{
    char *c, *t;

    /* find first token */
    if( !(t= strchr(str,'[')) ) return(-1);

    for(c= tok; *t && (*t != ']'); t++) *c++ = *t;
    *c++= ']';
    *c=0; 
    return(0);
}
