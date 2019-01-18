/* Interface between Interface Manager and Dialog Manager

   api functions readCommand() and writeCommand()
       read and write are from the point of view of the interface manager
   sends interfaces actions via readCommand()

   readCommand() commands (Interface commands):
	frame(frame_name) - name of the current frame. this should be the
		first command in each sequence sent to the interface manager.
	flash(file)
	speak(file)
	synth(string)
	clear_screen()
	log(message)

   writeCommand() commands
      switch_frame(frame_name) - push specified frame on stack
      input(string) - process user input
           calls process_input() which generates action seq in response buffer
      started()/ended() notices
         these contain entire initial command in parens, ie.
         started(flash(file))
         ended(flash(file))
         started(speak(file))
         ended(speak(file))
         started(synth(string))
         ended(synth(string))

    DM actions:
	push(frame name)
	pop()
	set(frame:key = 'constant' | frame:key)
	clear_key(frame:key)
	reset()

*/

#ifdef WIN
#include "stdafx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "phoenix.h"
#include "dm.h"

/* Global variables */
Config	cnfg;			/* structure for configuration parameters */
extern char    *response; 	/* response from phoenix */
extern char    *action_seq; 	/* response from phoenix */


/***  external variables   ***/
extern aFrame    *context[MAX_FRAME_STACK],
        	     *completed[MAX_FRAME_STACK],
        	*focus;
extern	int    fc_idx,
        	fe_idx,
        	fh_idx;

/* forward function declarations */
void action_switch(char *response, Config *cnfg);
int get_field(char *buf, char *str, char start, char end);
aFrame *getFrame(aFrame **list, char *name, char *ctl, int *fidx);
int initialize_task( Config *cnfg);
void initTmp();
int process_input( char *line, char *response, Config *cnfg );
char *readCommand();
int scmp(char *s1, char *s2);
void    setKey(aFrame *frame, char *key, char *val, Config *cnfg);
void writeCommand( char *cmd );
aFrame *pushFrame(char *fname, aFrame **context, aFrame** history,
	 int *context_idx, aFrame *rtn_frame, int rtn_element, Config *cnfg);
void    setKey(aFrame *frame, char *key, char *val, Config *cnfg);




char *readCommand()
{

	/* no action in queue */
	if( !*response ) {
		strcpy(action_seq, "no_action");
	}
	else {
	    strcpy(action_seq, response);
	    *response= (char)0;
	}

	return( action_seq );
}

void writeCommand( char *line )
{
    char    arg[LINE_LEN];	  /* buffer for arg string */
    char    cmd[LINE_LEN];	  /* buffer for command string */
    char *s;

	/* extract command */
	if( !(s= strchr(line, (int)'(')) ) return;
	strncpy(cmd, line, (int) (s-line));
	cmd[(int)(s-line)]= (char)0;

	/* extract argument from string */
	s++;
	strcpy(arg, s);
	/* get rid of ')' */
	if( (s= strrchr(arg, (int)')')) ) *s= (char)0;

	/* input from user */
	if( !strcmp(cmd, "input") ) {
		if( cnfg.VERBOSE > 2 ) {
		    printf("input: %s\n", arg);
		}

		// process input with parser and dialog manager
		// put output in response buffer
		response[0]= (char)0;
		process_input(arg, response, &cnfg);
	}

	/* push specified frame and generate new response */
	else if( !strcmp(cmd, "switch_frame") ) {
	    pushFrame(arg, context, completed, &fc_idx, 0, 0, &cnfg);
	    *response= (char)0;
	    action_switch(response, &cnfg);
	}

	/* action started */
	else if( !strcmp(cmd, "started") ) {
	}
	else if( !strcmp(cmd, "ended") ) {
	}
}


/* copy char sting between delimiters */
int get_field(char *buf, char *str, char start, char end)
{
    char *c, *e;

    /*  find start delimiter */
    if( !(c= strchr(str, (int)start)) ) return(1);

    for( c++; *c && isspace( (int) *c); c++) ;
    if( !*c ) return(1);

    /* find end delimiter */
    if( !(e= strchr(c, (int)end)) ) return(1);

    strncpy(buf, c, e-c);
    buf[e-c]= (char)0;

    /* strip blanks */
    for( c=buf; *c && isspace((int)*c); c++);
    if( c > buf ) {
	strncpy(buf, c, strlen(c));
	buf[strlen(c)]= (char)0;
    }
    for( c= buf + strlen(buf) -1; (c >= buf) && isspace((int)*c); c--)
	*c= (char)0;

    return(0);
}

int init(Config *cnfg)
{

	int ret = initialize_task( cnfg);

	initTmp();

	return ret;
}
