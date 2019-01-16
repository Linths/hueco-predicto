/* process text input from stdin
   write parsed output to stdout
   utterances terminated with newline
   type "quit" to exit
*/

#ifdef WIN
#include "stdafx.h"
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>	  
#include "phoenix.h"


#define OUTBUFLEN	10000

int set_config(Config *cnfg);
int init_parse(Config *cnfg);
void strip_line();
void parse(char *line, Gram *gram, Config *cnfg);
int print_parse(int parse_num, char *out_str, char *buf_end, char extract,
		Gram *gram, Config *cnfg);
void reset_parse(Config *cnfg);

/* external globals defined in pase.c */
extern char	*outbuf,
		*outbuf_end;

int main(argc, argv)
	int argc;
	char **argv;
{
    /* structure for configuration parameters */
    Config  cnfg;

    FILE	*fp;
    char	*s;
    int		i;
    char	line[LINE_LEN];		/* input line buffer */
    char	*out_ptr= outbuf;


    /* set command line or config file parms */
    set_config(&cnfg);

    /* read grammar, initialize parser, malloc space, etc */
    init_parse(&cnfg);

    /* terminal input */
    fp= stdin;

    /* for each utterance */
    for( ; fgets(line, LINE_LEN-1, fp);  ) {
	/* if printing comment */
	if (*line == ';' ) { printf("%s\n", line); continue; }
	/* if non-printing comment */
	if (*line == '#' ) { continue; }
	/* if blank line */
	for(s= line; isspace((int)*s); s++); if( strlen(s) < 2 ) continue;

        /* strip out punctuation, comments, etc, to uppercase */
        strip_line(line);

	/* check for terminate */
        if( !strncmp(line, "QUIT", 4) ) exit(1);

	/* clear output buffer */
	out_ptr= outbuf; *out_ptr= 0;

        /* echo the line */
        if (cnfg.VERBOSE > 1){
	    sprintf(out_ptr, ";;; %s\n", line);
	    out_ptr += strlen(out_ptr);
	}
    
        /* assign word strings to slots in frames */
        parse(line, cnfg.gram, &cnfg);


	/* print parses to buffer */
        if( cnfg.num_parses < 1 ) {
		strcpy(out_ptr, "No parse");
                out_ptr += strlen(out_ptr);
	}
	else if( cnfg.ALL_PARSES ) {
            for(i= 0; i < cnfg.num_parses; i++ ) {
		if( (outbuf_end - out_ptr) < MAX_PARSELEN ) {
		    fprintf(stderr, "ERROR: overflow OutBufSize %d\n",
			cnfg.OutBufSize);
		    break;
		}
	    	sprintf(out_ptr, "PARSE_%d:\n", i);
	    	out_ptr += strlen(out_ptr);
	    	print_parse(i, out_ptr, outbuf_end,cnfg.EXTRACT, cnfg.gram, &cnfg);
	    	out_ptr += strlen(out_ptr);
	    	sprintf(out_ptr, "END_PARSE\n");
	    	out_ptr += strlen(out_ptr);
            }
	}
	else {
	    	print_parse(0, out_ptr, outbuf_end,cnfg.EXTRACT, cnfg.gram, &cnfg);
	    	out_ptr += strlen(out_ptr);
	}
	sprintf(out_ptr, "\n");
	out_ptr += strlen(out_ptr);


	if( cnfg.VERBOSE ) {
	    printf("%s", outbuf);
	    fflush(stdout);
	}

        /* clear parser temps */
        reset_parse(&cnfg);

    }

    return(1);
}



void strip_line(line)
char	*line;
{
  char	*from, *to;

  for(from= to= line; ;from++ ) {
    if( !(*from) ) break;


    switch(*from) {

      /* filter these out */
    case '(' :
    case ')' :
    case '[' :
    case ']' :
    case ':' :
    case ';' :
    case '?' :
    case '!' :
    case '\n' :
      break;

      /* replace with space */
    case ',' :
    case '\\' :
      *to++ = ' ';
      break;

    case '#' :
	for( ++from; *from != '#' && *from; from++);
	if( *from == '#' ) from++;
	break; 

    case '-' :
      /* if partial word, delete word */
      if( isspace( (int) *(from+1) ) ) {
	while( (to != line) && !isspace( (int) *(--to) ) ) ;
	/* replace with space */
	*to++ = ' ';
      }
      else {
	/* copy char */
	*to++ = *from;
      }
      break;


    default:
      /* copy char */
      *to++ = *from;
#ifdef old
      *to++ = (islower((int)*from)) ? (char) toupper((int)*from) : *from;
#endif
    }
    if( !from ) break;

  }
  *to= 0;

}
