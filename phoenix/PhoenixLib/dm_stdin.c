//  main for testing dm 
// reads input from stdin, output to stdout
//


#ifdef WIN
#include "stdafx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include "phoenix.h"

/* Global variables */
extern Config    cnfg;    /* structure for configuration parameters */
extern char    *response; /* response from phoenix */
extern char    action[LINE_LEN];   /* action to be executed */

/* forward function declarations */
int initialize_task( Config *cnfg);
int process_input( char *line, char *response, Config *cnfg );
int scmp(char *s1, char *s2);
char *readCommand();
void writeCommand( char *cmd );


int main(int argc, char* argv[])
{
    char    line[LINE_LEN],        /* input line buffer */
            buf[LINE_LEN],
            *s;
    FILE    *fp;

    /* read task configuration parms */
    if( initialize_task( &cnfg) ) return(-1);
    //printf("%s\n", response);
    /* read input from stdin */
    fp= stdin;

    while (1) {

        /* readCommand */
        if( (s= readCommand()) ) {
            if( !strcmp("noaction", s) ) {
            }
	    else {
                printf("%s\n", s);
	    }
	}
        else {
            printf("ERROR: nothing returned\n");
        }

        /* input read and write commands from stdin */
        if( !fgets(line, LINE_LEN-1, fp) ) { break; }
        sprintf(buf, "input(%s)", line);
        writeCommand(buf);
    }
    return 0;
}

