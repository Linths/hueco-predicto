/*-----------------------------------------------------------------------*
   phoenix.c
   process_input(text, response, cnfg)
      takes an input word string and passes it to parse()
   it then takes the extracted parse and passes it to process_parse()
      which generates the next response
   writes next response sequence to specified buffer

*-----------------------------------------------------------------------*/

#ifdef WIN
#include "stdafx.h"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "phoenix.h"
#include "dm.h"

/*** functions defined here ***/
int process_input( char *line, char *response, Config *cnfg );
int process_actions( char* line, char *response, Config *cnfg);
int process_cmd_set(char* args, Config* cnfg);
int process_cmd_clear(char* args, Config* cnfg);
int process_cmd_push(char* args, Config* cnfg);
int process_cmd_pop(char* args, Config* cnfg);
void strip_dm_actions(char *line, char *new_response, Config *cnfg);
int split_action(char* action, char** cmd, char** args, Config* cnfg);
int initialize_task( Config *cnfg);
int set_config(Config *cnfg);


/*** functions called ***/
int set_config(Config *cnfg);
int init_parse(Config *cnfg);
void reset_parse(Config *cnfg);
void parse(char *line, Gram *gram, Config *cnfg);
int print_parse(int parse_num, char *out_str, char *buf_end, char extract,
		Gram *gram, Config *cnfg);
int process_parse(char *parse_str, char *response, Config *cnfg);
void init_dm(char *response, Config *cnfg);

/* external globals defined in pase.c */
extern char	*outbuf,
		*outbuf_end,
		*response;

extern aFrame    *context[MAX_FRAME_STACK],
/* Currently commented out.  Someday to be used for anaphora resolution
                 *history[MAX_FRAME_STACK],
 */
                 *completed[MAX_FRAME_STACK];
extern int	fc_idx; /* index of top of context stack */
    

/* external functions defined in frames.c */
extern aFrame *getFrame(aFrame **list, char *name, char *ctl, int *fidx);
extern char    *getVal(aFrame *frame, char *key, Config *cnfg);
extern void    setKey(aFrame *frame, char *key, char *val, Config *cnfg);
extern aFrame *pushFrame(char *fname, aFrame **context, aFrame **completed, 
                  int* context_idx, aFrame *rtn_frame, int rtn_element, Config *cnfg);
extern void    popFrame();

/* external functions defined in exec.c */

/*** external functions in util.c ***/
extern char* skip_whitespace(char *s);

/* input line of text
   call parse() to generate parse
   pass parse to process_parse() to generate response
   response wriiten to buffer pointed to by parameter response
*/
int process_input( char *line, char *response, Config *cnfg )
{
	int	ret,
		i;
	char  *out_ptr;
	char* cmd_prefix = "cmd:";

	ret= 0;

	/* if non-printing comment */
	if (*line == '#' ) return(0);
	/* if blank line */
	if( strlen(line) < 2 ) return(0);

	/* if processing a command directive */
	if( !strncmp(line, cmd_prefix, strlen(cmd_prefix) ) ) { 
	    return process_actions(line + strlen(cmd_prefix), response, cnfg);
	}

	/* check for terminate */
        if( !strncmp(line, "QUIT", 4) ) return(-1);

	if( cnfg->fp_debug ) {
	    fprintf(cnfg->fp_debug, "\n%s\n", line); fflush(cnfg->fp_debug);
	}

    

	/* parse() assigns word strings to slots in frames */
	outbuf[0]= (char)0;
	parse(line, cnfg->gram, cnfg);

	/*  put parse results in outbuf */
	out_ptr= outbuf;
        if( cnfg->num_parses < 1 ) {
		strcpy(out_ptr, "No parse");
                out_ptr += strlen(out_ptr);
	}
        else if( cnfg->ALL_PARSES ) {
            for(i= 0; i < cnfg->num_parses; i++ ) {
		if( (outbuf_end - out_ptr) < MAX_PARSELEN ) {
		    fprintf(stderr, "ERROR: overflow OutBufSize %d\n",
			cnfg->OutBufSize);
		    break;
		}
                sprintf(out_ptr, "PARSE_%d:\n", i);
                out_ptr += strlen(out_ptr);
                print_parse(i, out_ptr, outbuf_end, cnfg->EXTRACT,
				cnfg->gram, cnfg);
                out_ptr += strlen(out_ptr);
                sprintf(out_ptr, "END_PARSE\n");
                out_ptr += strlen(out_ptr);
            }
        }
        else {
                print_parse(0, out_ptr, outbuf_end, cnfg->EXTRACT,
				cnfg->gram, cnfg);
                out_ptr += strlen(out_ptr);
        }
        sprintf(out_ptr, "\n");
        out_ptr += strlen(out_ptr);

	/*  put parse results in response */
	strcpy(response, outbuf);
	if( cnfg->fp_debug ) {
	    fprintf(cnfg->fp_debug, "Parse:\n%s\n", outbuf); 
	    fflush(cnfg->fp_debug);
	}

	/* integrate parse into context and generate response */
	ret= process_parse(outbuf, response, cnfg);

	/* reset parser structures */
	reset_parse(cnfg);

	if( cnfg->fp_debug )
		fprintf(cnfg->fp_debug, "Response:\n%s\n", response);
	
	return(ret);
}


/* given string pointed to by line, strips all but interface actions.
   puts results in buffer pointed to by new_response.
 */
void strip_dm_actions(char *line, char *new_response, Config *cnfg) {
    char  *curr_cmd;
    char  line_copy[LINE_LEN];
    char  *cmd_sep = ";";
    char* cmd;
    char* args;

    if( new_response == NULL ) return;

    strcpy(line_copy, line);
    new_response[0] = '\0';

    /* Iterate through multiple commands on one line */
    for (curr_cmd = strtok(line_copy, cmd_sep);
         curr_cmd;
         curr_cmd = strtok(NULL, cmd_sep)) {

        if (!curr_cmd) { continue; }

        // parse out command and arguments
        curr_cmd = skip_whitespace(curr_cmd);
        split_action(curr_cmd, &cmd, &args, cnfg);

        // keep only interface actions
        // synth(), flash(), speak(), clear_screen()
        if (!strcmp(cmd, "speak") ||
            !strcmp(cmd, "synth") ||
            !strcmp(cmd, "clear_screen") ||
            !strcmp(cmd, "flash")) {

                strcat(new_response, cmd);
                strcat(new_response, "(");
                strcat(new_response, args);
                strcat(new_response, "); ");
        } 
    }
}


int split_action(char* action, char** cmd, char** args, Config* cnfg) {
    // Regular expression variables
    regex_t cmd_re;
    regmatch_t result[100];
    int ret;
    int eflag = 0;
    int no_sub;
    int len;

    if (!cmd) { return 0; }
    if (!args) { return 0; }

    // Compile command/action regular expression 
    ret = regcomp(&cmd_re, "([[:alnum:]_-]*)[(](.*)[)]", REG_EXTENDED);
    no_sub = cmd_re.re_nsub + 1;
    if( ret ){ fprintf(cnfg->fp_debug, "Could not compile regex key_re\n"); return 0; }

    // match regex
    if (regexec(&cmd_re, action, no_sub, result, eflag) == REG_NOMATCH) {
        fprintf(cnfg->fp_debug, "Badly formed action: %s\n", action);
    }

    // replace commands opening and terminating parenthesis with null terminators
    // to save off the parts of the command string
    *cmd = action;
    len = result[1].rm_eo - result[1].rm_so + 1;
    (*cmd)[len-1] = '\0';

    *args = action + len;
    len = result[2].rm_eo - result[2].rm_so + 1;
    (*args)[len-1] = '\0';

    return 1;

}

/* 
 * used for both manual command overrides and for changing of internal state
 * based on actions in the task file
 *    - iterates over all commands in an action sequence
 *    - execute dm_actions
 *    - appends to a new response string for interface actions
 */
int process_actions( char *line, char *new_response, Config *cnfg) {

    char* curr_cmd;
    char  line_copy[LINE_LEN];
    char* cmd_sep = ";";
    char* cmd;
    char* args;


    // Save of a copy of the line
    strcpy(line_copy, line);

    /* Iterate through multiple commands on one line */
    for (curr_cmd = strtok(line_copy, cmd_sep);
         curr_cmd;
         curr_cmd = strtok(NULL, cmd_sep)) {

        if (!curr_cmd) { continue; }

        // parse out command and arguments
        curr_cmd = skip_whitespace(curr_cmd);
        split_action(curr_cmd, &cmd, &args, cnfg);
        
	/* if dm action, process it */
        if (!strcmp(cmd, "set")) {
            // set command
            // set(frame:[key] = 'value')
            process_cmd_set(args, cnfg);
        } else if (!strcmp(cmd, "push")) {
            // push command
            // push(frame)
            process_cmd_push(args, cnfg);
        } else if (!strcmp(cmd, "pop")) {
            // pop command
            // pop()
            process_cmd_pop(args, cnfg);
        } else if (!strcmp(cmd, "clear_key")) {
            // clear command
            // clear_key(frame:[key])
            process_cmd_clear(args, cnfg);          
        } else if (!strcmp(cmd, "reset")) {
            // reset command
            // reset()
        }

	// if interface action, add to new_response buffer
        // synth(), flash(), speak(), clear_screen()
	else if (!strcmp(cmd, "speak") ||
                   !strcmp(cmd, "synth") ||
                   !strcmp(cmd, "clear_screen") ||
                   !strcmp(cmd, "flash")) {
            if (new_response != NULL) {
                strcat(new_response, cmd);
                strcat(new_response, "(");
                strcat(new_response, args);
                strcat(new_response, "); ");
            }
        } else if (!strcmp(cmd, "send")) {
            // strip the send()
        } else {
            if ( cnfg->VERBOSE ) 
		        printf("%s: No such command: \n", cmd);
            if ( cnfg->fp_debug )
		        fprintf(cnfg->fp_debug, "%s: No such command: \n", cmd);
        }
    }

    return 0;


}

int process_cmd_set(char* args, Config* cnfg) {
    char  frame_arg[LINE_LEN];
    char  key_arg[LINE_LEN];
    char  val_arg[LINE_LEN];
    char* frame;
    char* key;
    char* val;
    int i;
    int fidx;
    aFrame	*fr;

    fr = 0;
    fidx = -1;

    args = skip_whitespace(args);
    i = sscanf(args, "%[^:]:%[^ =]%*[ =']%[^']", frame_arg, key_arg, val_arg);
    frame = skip_whitespace(frame_arg);
    key = skip_whitespace(key_arg);
    if (i == 3) {
        val = skip_whitespace(val_arg);
    } else if (i == 2) {
        strcpy(val_arg, "TRUE");
        val = val_arg;
    } else {
        if ( cnfg->VERBOSE ) {
            printf("set: Wrong number of arguments.\n");
            printf("set: set(frame:key = 'value')\n");
            printf("set: set(frame:key)\n");
        }
    }

    // Find frame
    if (!fr) { fr= getFrame(context, frame, "next", &fidx); }
/* Currently commented out.  Someday to be used for anaphora resolution
    if (!fr) { fr= getFrame(history, frame, "next", &fidx); }
 */
    if (!fr) { fr= getFrame(completed, frame, "next", &fidx); }

    if (!fr) {
        if ( cnfg->VERBOSE ) {
            printf("set: frame %s not found.\n", frame);
        }
        if ( cnfg->fp_debug ) {
            fprintf(cnfg->fp_debug, "setKey: frame %s not found.\n", frame);
        }
        return 0;

    }
	setKey(fr, key, val, cnfg);

  return 0;
}

int process_cmd_clear(char* args, Config* cnfg) {
    char  frame_arg[LINE_LEN];
    char  key_arg[LINE_LEN];
    char  val_arg[LINE_LEN];
    char* frame;
    char* key;
    char* val;
    int i;
    int fidx;
    aFrame	*fr;

    fr = 0;
    fidx = -1;

    args = skip_whitespace(args);
    i = sscanf(args, "%[^:]:%[^ \n]", frame_arg, key_arg);
    frame = skip_whitespace(frame_arg);
    key = skip_whitespace(key_arg);
    if (i == 2) {
        val = skip_whitespace(val_arg);
    } else {
        if ( cnfg->VERBOSE ) {
            printf("set: Wrong number of arguments.\n");
            printf("set: set(frame:key = 'value')\n");
            printf("set: set(frame:key)\n");
        }
    }

    // Find frame
    if (!fr) { fr= getFrame(context, frame, "next", &fidx); }
/* Currently commented out.  Someday to be used for anaphora resolution
    if (!fr) { fr= getFrame(history, frame, "next", &fidx); }
 */
    if (!fr) { fr= getFrame(completed, frame, "next", &fidx); }
    /*
    if (!fr) { fr= getFrame(history, frame, "last", &fidx); }
    if (!fr) { fr= getFrame(context, frame, "last", &fidx); }
    if (!fr) { fr= getFrame(completed, frame, "last", &fidx); }
    */

    if (!fr) {
        if ( cnfg->VERBOSE ) {
            printf("set: frame %s not found.\n", frame);
        }
        if ( cnfg->fp_debug ) {
            fprintf(cnfg->fp_debug, "clear frame %s not found.\n", frame);
        }
        return 0;

    }
	setKey(fr, key, NULL, cnfg);

  return 0;
}

int process_cmd_push(char* args, Config* cnfg) {
    char  frame_arg[LINE_LEN];
    char* frame;
    int i;
    int fidx;
    aFrame	*fr;

    fr = 0;
    fidx = -1;

    i = sscanf(args, "%[^ \n]", frame_arg);

    if (i != 1) {
        if ( cnfg->fp_debug ) {
            fprintf(cnfg->fp_debug, "push: Too many arguments.\n");
            fprintf(cnfg->fp_debug, "push: push(frame)\n");
        }
        return 0;
    }

    frame = skip_whitespace(frame_arg);
    pushFrame(frame, context, completed, &fc_idx, 0, 0, cnfg);

  return 0;
}

int process_cmd_pop(char* args, Config* cnfg) {
  popFrame();
  return 0;
}


/* read all files and initialize buffers */
int initialize_task( Config *cnfg)
{

    /* read config file to set parameters */
    if( set_config(cnfg) ) {
	fprintf(stderr, "Configuration failed\n");
	return(-1);
    }

    /* read grammar, initialize parser, malloc space, etc */
    if( init_parse(cnfg) ) return(-1);

    /* read task files */
    init_dm(response, cnfg);

    return(0);
}


/* read configuration file and set parameters */
int set_config(Config *cnfg)
{
	FILE	*fp;
	char	parm[LABEL_LEN],
			value[PATH_LEN];
	char	*s;
	static char	sym1[] = "<s>",	/* default start of utt symbol */
			sym2[] = "</s>"; /* default end of utt symbol */

	/* open config file */
	if( !(fp = fopen(CONFIG_FILE, "r") )) {
	    fprintf(stderr, "Cannot open configuration file %s\n",
				CONFIG_FILE);
	    return(-1);
	}

	/* clear config struct */
#ifndef WIN
	strncpy( (char *) cnfg, "", sizeof( Config ) );
#endif	

	cnfg->start_sym= sym1;
	cnfg->end_sym= sym2;

	/* SET DEFAULT VALUES */
        strcpy(cnfg->nets_file, "nets");
        strcpy(cnfg->dict_file, "base.dic");
        strcpy(cnfg->frames_file, "frames");
	cnfg->EdgeBufSize=	EDGEBUFSIZE;
	cnfg->ChartBufSize=	CHARTBUFSIZE;/* max number of paths in beam */
	cnfg->PeBufSize=	PEBUFSIZE;/* number of Val slots for trees  */
	cnfg->InputBufSize=	INPUTBUFSIZE;/* max words in line of input */
	cnfg->SlotSeqLen=	SLOTSEQLEN;/* max slots in a sequence */
	cnfg->SymBufSize=	SYMBUFSIZE;/* buffer to hold char strings */
	cnfg->ParseBufSize=	PARSEBUFSIZE;	/* buffer for parses */
	cnfg->OutBufSize=	OUTBUFSIZE;	/* buffer for parses */
	cnfg->SeqBufSize=	SEQBUFSIZE;	/* buffer for sequence nodes */
	cnfg->FidBufSize=	FIDBUFSIZE;	/* buffer for frame ids */
	cnfg->MaxPrompt=	MAX_ACTION;
	cnfg->SymBufSize=   50000;    /* buffer size to hold char strings */
	cnfg->TokBufSize=   10000;    /* buffer size to hold char strings */
	cnfg->MaxNonTerm=    100000;  /* max number non-terminal pointers */
	cnfg->MaxSymbol=     10000;   /* max number of nonterminal symbols */
	cnfg->MaxNfa=        150000;  /* max number of nodes in network */
	cnfg->MaxSucLink=    500000;  /* max number of arcs in network */



	while( fscanf(fp, "%[^:]:%[^\n]\n", parm, value) == 2 ) {

		/* remove leading blanks */
		if( isspace( (int) value[0]) ) {
		    for(s= value; *s && isspace((int) *s); s++);
		    strcpy(value, s);
		}

		/* remove trailing blanks */
		for(s= value + strlen(value)-1;
			isspace((int) *s) && (s>value); s--) *s= (char)0;

		if( !strcmp(parm, "VERBOSE") ) {
			cnfg->VERBOSE= atoi(value);
		}
		else if( !strcmp(parm, "DEBUG") ) {
			if( !strcmp(value, "n") ) {
			}
			else if( !(cnfg->fp_debug= fopen(value,"w")) ) {
				fprintf(stderr,
					"can't open debug file: %s\n", value);
			}
		}
		else if( !strcmp(parm, "EXTRACT") ) {
			cnfg->EXTRACT= *value;
		}
		else if( !strcmp(parm, "ALL_PARSES") ) {
			cnfg->ALL_PARSES= *value;
		}
		else if( !strcmp(parm, "IGNORE_OOV") ) {
			cnfg->IGNORE_OOV= *value;
		}
		else if( !strcmp(parm, "PROFILE") ) {
			cnfg->PROFILE= *value;
		}
		else if( !strcmp(parm, "TASK_DIR") ) {
			strcpy(cnfg->task_dir, value);
		}
		else if( !strcmp(parm, "DICT_FILE") ) {
			strcpy(cnfg->dict_file, value);
		}
		else if( !strcmp(parm, "GRAMMAR_FILE") ) {
			strcpy(cnfg->grammar_file, value);
		}
		else if( !strcmp(parm, "GRAMMAR_RULES") ) {
			strcpy(cnfg->grammar_rules, value);
		}
		else if( !strcmp(parm, "NETS_FILE") ) {
			strcpy(cnfg->nets_file, value);
		}
		else if( !strcmp(parm, "FRAMES_FILE") ) {
			strcpy(cnfg->frames_file, value);
		}
		else if( !strcmp(parm, "TASK_FILE") ) {
			strcpy(cnfg->task_file, value);
		}
		else if( !strcmp(parm, "EDGEBUFSIZE") ) {
			cnfg->EdgeBufSize= atoi(value);
		}		
		else if( !strcmp(parm, "CHARTBUFSIZE") ) {
			cnfg->ChartBufSize= atoi(value);
		}		
		else if( !strcmp(parm, "PEBUFSIZE") ) {
			cnfg->PeBufSize= atoi(value);
		}		
		else if( !strcmp(parm, "INPUTBUFSIZE") ) {
			cnfg->InputBufSize= atoi(value);
		}		
		else if( !strcmp(parm, "SLOTSEQLEN") ) {
			cnfg->SlotSeqLen= atoi(value);
		}		
		else if( !strcmp(parm, "SYMBUFSIZE") ) {
			cnfg->SymBufSize= atoi(value);
		}		
		else if( !strcmp(parm, "TOKBUFSIZE") ) {
			cnfg->TokBufSize= atoi(value);
		}		
		else if( !strcmp(parm, "PARSEBUFSIZE") ) {
			cnfg->ParseBufSize= atoi(value);
		}		
		else if( !strcmp(parm, "OUTBUFSIZE") ) {
			cnfg->OutBufSize= atoi(value);
		}		
		else if( !strcmp(parm, "SEQBUFSIZE") ) {
			cnfg->SeqBufSize= atoi(value);
		}		
		else if( !strcmp(parm, "FIDBUFSIZE") ) {
			cnfg->FidBufSize= atoi(value);
		}
		else if( !strcmp(parm, "MAXACTION") ) {
			cnfg->MaxPrompt= atoi(value);
		}		
		else if( !strcmp(parm, "MAXNONTERM") ) {
			cnfg->MaxNonTerm= atoi(value);
		}		
		else if( !strcmp(parm, "MAXSYMBOL") ) {
			cnfg->MaxSymbol= atoi(value);
		}		
		else if( !strcmp(parm, "MAXNFA") ) {
			cnfg->MaxNfa= atoi(value);
		}		
		else if( !strcmp(parm, "MAXSUCLINK") ) {
			cnfg->MaxSucLink= atoi(value);
		}		


	}
#ifdef DEBUG
	printf("VERBOSE:%d\n", cnfg->VERBOSE);
	printf("EXTRACT:%c\n", cnfg->EXTRACT);
	printf("ALL_PARSES:%c\n", cnfg->ALL_PARSES);
	printf("IGNORE_OOV:%c\n", cnfg->IGNORE_OOV);
	printf("PROFILE:%c\n", cnfg->PROFILE);
	printf("task_dir:%s\n", cnfg->task_dir);
	printf("dict_file:%s\n", cnfg->dict_file);
	printf("grammar_file:%s\n", cnfg->grammar_file);
	printf("frames_file:%s\n", cnfg->frames_file);
	printf("task_file:%s\n", cnfg->task_file);
#endif
	return(0);
}
