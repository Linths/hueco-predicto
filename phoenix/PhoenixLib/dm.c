/*-----------------------------------------------------------------------*
        Dialogue Manager
  dm.c
  process_parse(parse, response, cnfg)
     Takes a char string, which is the extracted form parse
     produced by the parser and writes the system response to the
     specified buffer.

  The general architecture for process_parse() is:
    int process_parse(char *parse_str, char *response, Config *cnfg) {

    # extract information from current parse
    extract_values(parse, cnfg);

    #  merge extracted frames with current context frames
    merge_context(context, extracts, cnfg);

    #  generate response and execute DM actions
    action_switch(response, cnfg);

 *-----------------------------------------------------------------------*/

#ifdef WIN
#include "stdafx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rules_grammar.h"
#include "phoenix.h"
#include "dm.h"


/***  context variables   ***/
aFrame    *context[MAX_FRAME_STACK],   // current context stack
        *extracts[MAX_FRAME_STACK],    // frames extracted from parse
/* Commented out for now.  To be used for anaphora resolution
        *history[MAX_FRAME_STACK],
*/
        *completed[MAX_FRAME_STACK],   // stack of retired frames
        *focus;
int    fc_idx,
        fe_idx,
        fh_idx;
Prompt    prompted;
char    prompt_text[LINE_LEN],
    action_text[LINE_LEN],
    *prompt;

aFrameDef    *ontology;

static    int     hist_idx;
static    char    *action;
static    int    done;
static    int    end_done;


/*** functions defined here */
void    action_switch(char *response, Config *cnfg);
int    extract_values(char *parse_string, Config *cnfg);
aFrame    *findContext(aFrame *extract, Config *cnfg);
void    flatten(char *s);
char	*get_best_parse(char *parse_str, Config* cnfg);
void    init_dm(char *response, Config *cnfg);
void    merge_context(aFrame **context, aFrame **extracts, Config *cnfg);
char    *next_line(char *p);
void    nl_request(char *speech_act, char *prompt, char *response);
int     overlap(aFrame *f1, aFrame *f2, Config *cnfg);
void    print_context(aFrame **context, Config *cnfg);
int     process_parse(char *parse_str, char *response, Config *cnfg);
void    read_task(char *dir, char *task_file, Config *cnfg);
void    write_task(aFrameDef* task, FILE* fp, Config* cnfg);
void    reset_dm();
void    strip_blanks(char *s);
char**  get_frame_prompts(aFrameDef* frame, int* num_prompts_tot, Config* cnfg);

/*** functions in frames.c ***/
void    addFrame(aFrame **list, aFrame *frame);
void    clear_all_frames();
void    cp_frame(aFrame *to, aFrame *from, Config *cnfg);
void    copy_stack(aFrame **src, aFrame **tgt, int* tgt_idx);
void    reset_stack(aFrame **stack);
void    cp_values(aFrame *to, aFrame *from, Config *cnfg);
aFrame    *createFrame(char *name, Config *cnfg);
int frame_complete( aFrame *frame, char *prompt, Config *cnfg);
int    frame_empty(aFrame *frame, Config *cnfg);
aFrame    *getFrame(aFrame **list, char *name, char *ctl, int *fidx);
aFrameDef *getFrameDef( char *name, Config *cnfg );
char    **getKeyPtr(aFrame *frame, char *key, Config *cnfg);
char    *getVal(aFrame *frame, char *key, Config *cnfg);
char    *is_child( aFrame *frame, char *slot, char *child, Config *cnfg );
void    popFrame();
void    print_frame( aFrame *frame, Config *cnfg );
aFrame *pushFrame(char *fname, aFrame **context, aFrame** history, int *content_idx, 
                aFrame *rtn_frame, int rtn_element, Config *cnfg);
void    setKey(aFrame *frame, char *key, char *val, Config *cnfg);
void    set_prompted(aFrame *frame, char *key, char *type);

/* functions in phoenix.c */
int process_actions(char *line, char *new_response, Config *cnfg );
void strip_dm_actions(char *line, char *stripped, Config *cnfg );


/* functions in functions */
void check_transform( char *tree);
void check_function(char *frame_name, char *key,  char *val);

/* functions in util */
int    scmp(char *s1, char *s2);
void    cp_str(char **to, char *from);


/*-----------------------------------------------------------------------*
  Receive parse, merge with context, generate actions
 *-----------------------------------------------------------------------*/
int process_parse(char *parse_str, char *response, Config *cnfg)
{
    char *s;

    prompt= (char *)0;
    action= (char *)0;
    s= parse_str;
    *response= (char)0;

    if( !s || !scmp("No parse", s) ) {

    }
    /* extract information and set global variables */
    else {
	/* get best parse given context */
	s= get_best_parse(parse_str, cnfg);

        if( extract_values(s, cnfg) < 0 ) {
            nl_request("continue", prompt, response);
            return(0);
        }
        if( cnfg->VERBOSE > 2 ) { 
            printf("Extracts: \n");
            print_context(extracts, cnfg);
        }

        /* merge extracted frames with current context frames */
        merge_context(context, extracts, cnfg);
    }

    if ( focus ) action= focus->name;

    /* generate response and execute system actions */
    action_switch(response, cnfg);
    return(0);
}

 
/*-----------------------------------------------------------------------*
  Check for context flags and execute associted functions
  Try to complete frame on top of context stack
 *-----------------------------------------------------------------------*/
void action_switch(char *response, Config *cnfg)
{
    aFrameDef   *fd;
    int         fdone,
                send;
    char        action_seq[LINE_LEN];
    char        *s;
    char        new_response[LINE_LEN];
    char        focus_frame[LABEL_LEN];


    if( cnfg->VERBOSE > 2 ) print_context(context, cnfg);

    /******** Actions Indicated by Flags **************/

    /* if done,  say bye and end conversation */
    if (done ) {
    set_prompted((aFrame *)0, (char *)0, (char *)0);
    prompt= "";
    nl_request("prompt_user", prompt, response);
    return;
    }
    
    /* start over */
    if ( action && !scmp("reset", action) ) {
        action= (char *)0;
        reset_dm();
        prompt= "";
        set_prompted( (aFrame *)0, "[start]", (char *)0);
        nl_request("prompt_user", prompt, response);
        action= "prompt";
        return;
    }

    /* repeat last output */
    if( action && !scmp("repeat", action) ) {
        nl_request("repeat", "", response);
        return;
    }

#ifdef OLD
    /* check for clarification or error prompt */
    if( prompt ) {
        nl_request("prompt_user", prompt, response);
        prompt= (char *)0;
        action= "prompt";
        return;
    }
#endif

    /******** Process frame on top of stack **************/

    /* clear old prompt */
    prompt= (char *)0; set_prompted((aFrame *)0, (char *)0, (char *)0);
    prompt_text[0]= action_text[0]= (char)0;
    focus_frame[0]= (char)0;

    /* if frame on context stack */
    while( fc_idx >= 0 ) {

        /* check frame on top of stack for completion */
        action_seq[0]= (char) NULL;
        fdone = frame_complete(context[fc_idx], action_seq, cnfg);
	/* save name of frame in focus */
	strcpy( focus_frame, context[fc_idx]->name);
    
        /* pop frame if done */
        if( fdone ) {
            popFrame();
            /* if no active frames */
            if( fc_idx < 0 ) {
                if( !end_done ) {
                    /* push End frame if one exists */
                    if( (fd= getFrameDef("End", cnfg)) ) {
                        pushFrame("End",context, completed, &fc_idx, (aFrame *)0, 0, cnfg);
                    }
                    end_done= 1;
                }
            }
        }


        if( action_seq[0] ) {
			
            // Execute dm actions in action_seq and buffer interface actions
	    // into new_response
	    new_response[0] = '\0';
            process_actions(action_seq, new_response, cnfg);
            
            /* see if send() */
            if( (s= strstr(action_seq, "send()") ) ) {
                *s= (char)0;
                send= 1;
            } else { send= 0; }
    
            /* append action_seq to response buffer */
            if( (strlen(new_response) + strlen(action_seq) + 3) > LINE_LEN ) {
                fprintf(stderr,"ERROR: response too long\n");
                exit(-1);
            }

	    /* append focus frame name */
	    if( !focus_frame[0] ) {
		printf("WARNING: No focus frame\n");
	    }
	    else {
	        strcat( response, "frame(");
	        strcat( response, focus_frame);
	        strcat( response, "); ");
	    }

	    /* append new action seq */
            strcat(response, new_response);
            prompt= (char *)0;
            action= "prompt";
            if( send )  return;
        }

    }

    return;
}



/* extract values in parse string to temporary frames */
int extract_values(char *parse_string, Config *cnfg)
{
    char    frame_name[LABEL_LEN],
            val[LINE_LEN],
            key[LINE_LEN],
            key_val[LINE_LEN],
            line[LINE_LEN];
    int        i, cnt;
    char    *p,
            *kv,
            *full_key,
            **key_ptr,
            *vv,
            *ps;
    aFrame    *frame;
    aFrameDef    *fd;
    int        xx;


    /* process each line in parse */
    /* map strings to canonical form, create frame and fill in from parse */
    line[0]= (char)0;
    for( ps= parse_string; sscanf(ps, "%[^\n]", line) > 0; ps= next_line(ps) ){
        if( !line[0] ) continue;

        /* get frame name, and key-val string */
        frame_name[0]= (char)0;
        key_val[0]= (char)0;
        if( sscanf(line, "%[^:]:%[^\n]", frame_name, key_val) < 1 ) break;

        /* if no root net, move ptr to start of next line */
        if( *key_val != '[' ) continue;

        /* get temporary frame */
        if( !(frame= getFrame(extracts, frame_name, "last", &xx)) ) {
            if( !(frame= createFrame(frame_name, cnfg)) ) return(-1);
            addFrame(extracts, frame);
        }

        /* apply transform functions */
        for( p=key_val;  (p= strchr(p, '[')); p++ ) { check_transform(p); }

        /* flatten the representation */
        flatten(key_val);

        full_key= (char *)0;
        /* for each  key - value */
        for( kv= key_val; sscanf(kv, "%[^\n]", line) >0; kv= next_line(kv) ){

            /* extract key and value from line of parse */
            if( !(p= strrchr( line, (int) ']')) ) break;
            p++;
            if( *p != '.' ) break;
            *p= ':';
            key[0]= val[0]= (char)0;
            cnt= sscanf(line, "%[^:]:%[^\n]", key, val);
            strip_blanks(val); 
    
            /* resolve under-specified keys */
            if( !(key_ptr= getKeyPtr(frame, key, cnfg)) ) {
    
                /* if responsive to prompted key */
                if( prompted.key && strstr(prompted.key, key) ) {
                    full_key= prompted.key;
                }
                else if( prompted.key &&
                    (full_key= is_child(frame,prompted.key, key, cnfg)) ) {
                }
                else {
                    Slot *s;
                    if( !(fd= getFrameDef(frame->name, cnfg)) ) return(-1);
        
                    /* use first relevant slot */
                    for(i=0; i < fd->n_slot; i++ ) {
                        s= (fd->slot)+i;

                        // If key matches beginning of the key, then accept it
                        // even though it is under-specified.
                        if( s->key && !scmp(s->key, key) ) break;
                    }
    
                    if( i < fd->n_slot ) { full_key= s->key; }
                    /* if no relevant slot in frame */
                    else { full_key= key; }
               }
            }
            else full_key= key;

            /* whw - for now, don't overwrite */
            if( (vv= getVal(frame, full_key, cnfg)) ) { continue; }
    
            /* execute a function associated with slot */
            check_function(frame_name, full_key,  val);
    
            /* fill slot in frame */
            if( full_key && frame ) setKey(frame, full_key, val, cnfg);

        }
    }
    return(0);
}


void init_dm(char *response, Config *cnfg)
{
    aFrameDef   *fd;

    /* read ontology */
    read_task(cnfg->task_dir, cnfg->task_file, cnfg);
    // if (cnfg->VERBOSE > 7) { 
	//   write_task(ontology, stdout, cnfg);
    // }

    reset_dm();

    /* push start frame if one exists */
    if( !(fd= getFrameDef("Start", cnfg)) ) return;
    pushFrame("Start", context, completed, &fc_idx, (aFrame *)0, 0, cnfg);
    action_switch(response, cnfg);
}


void reset_dm()
{

    fc_idx= -1;
    fe_idx= -1;
    fh_idx= -1;
    focus= (aFrame *)0;
    action= (char *)0;
    set_prompted((aFrame *)0, (char *)0, (char *)0);
    prompt_text[0]= (char)0;
    done= 0;
 
    /* free all frames */
    clear_all_frames();
    hist_idx = 0;
    end_done= 0;
}




char *next_line(char *p)
{
    for( ; *p && (*p != '\n' ); p++);
    if( *p == '\n') p++;
    return(p);
}



/* strip trailing whitespace */
void strip_blanks(char *s)
{
    char *p;

    for(p= s + (strlen(s)-1); *p && isspace((int)*p); p--);
    *(p+1)= (char)0;
}

/* separate multiple values on single line to separate lines */
void flatten(char *s)
{
    char *p, *f, *t, *h;
    char inbuf[LINE_LEN];
    char outbuf[LINE_LEN];
    char head[LABEL_LEN];

    outbuf[0]= (char)0;
    /* process each line in buffer*/
    for(p=s; *p && sscanf(p, "%[^\n]", inbuf) > 0;  ) {
        /* get head token */
        head[0]= (char)0;
        if( !(h= strchr(inbuf,(int)'[')) ) continue;
        if( sscanf(h, "%[^.]", head) < 1 ) continue;
        if( !(t= strchr(h,(int)'.')) ) continue;
        t++;

        /* if no head token */
        {
            char *c;
            int cnt;

            cnt= 0;
            for(c=t; *c && !isspace(*c); c++) {
                if( *c == '.' ) cnt++;
            }
            /* no head token */
            if( !cnt ) {
                t= h;
                head[0]= (char)0;
            }
        }

        /* while another token on line */
        while( (f= strchr(t,(int)' ')) ) {

            /* find start of next token */
            if( !(f= strchr(f,(int)'[')) ) break;
            *(f-1)=0;

            if( head[0] ) {
                strcat(outbuf, head);
                strcat(outbuf, ".");
            }
            strcat(outbuf, t);
            strcat(outbuf, "\n");
            t=f;
        }
        if( head[0] ) {
            strcat(outbuf, head);
            strcat(outbuf, ".");
        }
        strcat(outbuf, t);
        strcat(outbuf, "\n");

        if( !(p=strchr(p,(int)'\n'))) break;
        p++;
    }
    if(outbuf[0]) strcpy(s, outbuf);
}







void print_context(aFrame **context, Config *cnfg)
{
    aFrame    *cf;
    int        xx;

    printf("Context frames:\n");
    for(cf= getFrame(context, "*", "first", &xx); cf;
        cf= getFrame(context, "*", "next", &xx)){

        print_frame(cf, cnfg);
    }
    printf("\n"); fflush(stdout);
}

void merge_context(aFrame **context, aFrame **extracts, Config *cnfg)
{
    aFrame    *ef,
        *cf;
    int        xx;

    /* print extracted frames */
    if( cnfg->VERBOSE > 2 ) {
        for(ef= getFrame(extracts, "*", "first", &xx); ef;
            ef= getFrame(extracts, "*", "next", &xx)){
        }
    }

    /* for each extracted frame */
    for(ef= getFrame(extracts, "*", "first", &xx); ef;
        ef= getFrame(extracts, "*", "next", &xx)){


        /* if empty frame */
        if( frame_empty(ef, cnfg) ) continue;

        /* if no appropriate frame in context, create one */
        if( !(cf= findContext(ef, cnfg)) ) {
            if( !(cf= createFrame(ef->name, cnfg)) ) return;
                cf= pushFrame(ef->name,context, completed, &fc_idx, (aFrame *)0, 0, cnfg);
            cp_frame(cf, ef, cnfg);
            focus= cf;
            continue;
        }

        /*  cp values to current frame */
        cp_values(cf, ef, cnfg);

    }

    /* copy extracted frames to history list */
/* FIXME: Currently commented out.  Someday to be used for anaphora resolution 
    copy_stack(extracts, history, &hist_idx);
 */

    /* reset extracts for subsequent use */
    reset_stack(extracts);

}


/* find frame in context which is best match to extracted frame */
aFrame *findContext(aFrame *extract, Config *cnfg)
{
    aFrame *frame, *best_frame;
    int xx, count, best_count;

    /* if same as current focus frame */
    if( focus && !strcmp(focus->name, extract->name) ) return(focus);

    if( prompted.frame && !scmp(prompted.frame->name, extract->name) )
        return(prompted.frame);
    best_frame= (aFrame *)0;
    best_count= 0;
    for(frame= getFrame(context, extract->name, "first", &xx); frame;
        frame= getFrame(context, extract->name, "next", &xx)){

        if( (count= overlap(frame, extract, cnfg)) >= best_count ) {
            best_frame= frame;
            best_count= count;
        }
    }
    return(best_frame);
}

/* compute context slot overlap between frame */
int overlap(aFrame *f1, aFrame *f2, Config *cnfg)
{
    int         i, count;
    aFrameDef   *fd;
    Slot        *s;
    char *c;

    if( !f1 || !f2 ) return(0);
    if( scmp(f1->name, f2->name) ) return(0);

    /* Find frame definition */
    if( !(fd= getFrameDef(f1->name, cnfg)) ) return(0);

    count= 0;
    for( i=0, s= fd->slot; i < fd->n_slot; i++, s++ ){
        /* ignore if not content */
        if( !(c= strchr(s->key, (int) '[')) ) continue;
        if( !isupper( (int) *(++c)) ) continue;
        if( !scmp("[Active]", s->key) ) continue;

        if( !((f1->value)+i) || !((f2->value)+i) ) continue;
        if( !*((f1->value)+i) || !*((f2->value)+i) ) continue;

        if( !scmp(*((f1->value)+i), *((f2->value)+i) ) ) count++;
    }
    return(count);

}


/* read task definition */
void read_task(char *dir, char *task_file, Config *cnfg)
{
    int     slot_num,
            rule_num,
            fn,
            pn,
            i, j,
            cnt;
    char    name[LINE_LEN];
    char    root[LINE_LEN];
    char    path[LINE_LEN];
    char    line[LINE_LEN];
    char    prompt[LINE_LEN];
    char    *r;
    char    *c;
    char    field[LINE_LEN];
    Slot    *form_buf;
    Rule    *rule_buf;
    FILE    *fp;
    RulesTreeNode *node = NULL;


    /* open task file */
    sprintf(line, "%s/%s", dir, task_file);
    if( !(fp= fopen(line, "r")) ) {
        fprintf(stderr, "ERROR: can't open %s\n", line);
        exit(-1);
    }

    /* count frames, slots_per_frame, and rules_per_frame */
    cnfg->task_slots= 0;
    cnfg->task_rules= 0;
    slot_num= 0;
    rule_num= 0;
    for(cnfg->task_frames= 0; ; cnfg->task_frames++ ) {

        /* scan for start of frame */
        while( (r= fgets(line, LINE_LEN, fp)) ) {
                sscanf(line, "%s%*[^\r\n]\n", name);
            if( !scmp("Frame", name) ) break;
        }
        if( !r ) break;
    
        /* count slot and rules declarations */
        slot_num = 0;
        rule_num = 0;
        while( (r= fgets(line, LINE_LEN, fp)) ) {
            if( sscanf(line, "%s%*[^\n]\n", name) < 1 ) continue;
            if( name[0] == ';' ) break;
            /* Rules: section, count the number of rules */
            else if ( !scmp("Rules:", name)) {
                /* Exceptions rules follow all slot declarations 
                 * and if present will finish off the frame */
                int end = 0;
                while ( (r = fgets(line, LINE_LEN, fp)) ) {
                    if( sscanf(line, "%s%*[^\n]\n", name) < 1 ) { continue; }
                    else if( name[0] == ';' ) { end = 1; break; }
                    else if( name[0] == '[' ) { rule_num++; }
                    else if( name[0] == '!' ) { rule_num++; }
                    else if( name[0] == '(' ) { rule_num++; }
                }
                break;
            }       
            else if( name[0] == '<' ) slot_num++;
            else if( name[0] == '[' ) slot_num++;
            else if( name[0] == '.' ) slot_num++;
        }
        if( !r ) {
            fprintf(stderr, "Bad format in forms file\n");
            exit(-1);
        }
        if ( slot_num > cnfg->task_slots ) cnfg->task_slots = slot_num; 
        if ( rule_num > cnfg->task_rules ) cnfg->task_rules = rule_num;

    }

    /* malloc space for frame templates */
    if( !(ontology=(aFrameDef *) calloc(cnfg->task_frames, sizeof(aFrameDef)))) {
        fprintf(stderr, "ERROR: malloc Frame templates failed\n");
        exit(-1);
    }

    /* malloc space for temporary slots buffer */
    if( !(form_buf= (Slot *) calloc(cnfg->task_slots, sizeof(Slot))) ) {
        fprintf(stderr, "ERROR: malloc form buffer failed\n");
        exit(-1);
    }

    
    /* malloc space for temporary rules buffer */
    if ( !(rule_buf = (Rule*) calloc(cnfg->task_rules, sizeof(Rule)))) {
        fprintf(stderr, "ERROR: malloc form buffer failed\n");
        exit(-1);
    }


    /* Now that we know the number of frames, slots, and rules, go back and parse
       them out */
    rewind(fp);

    /* for each frame in file */
    for(fn = 0; ; fn++ ) {

        /* scan for start of frame */
        while( (r= fgets(line, LINE_LEN, fp)) ) {
                sscanf(line, "%[^:]:%s", prompt, name);
            if( !scmp("Frame", prompt) ) break;
        }
        if( !r ) break;
        if( scmp("Frame", prompt) ) break;

        /* copy frame name */
        cp_str( &((ontology+fn)->name), name);

        /* initialize description field */
        ontology[fn].description = NULL;

        /* read slot names,  prompts, and rules */
        slot_num= -1;
        rule_num = -1;
        prompt[0]=0;
        while( (r= fgets(line, LINE_LEN, fp)) ) {
            for(c= line; isspace( (int) *c); c++);
            if( !*c ) continue;
            if( *c == ';' ) break;
            if( !scmp("Description:", c)) {
                if (sscanf(c, "Description:%[^\r\n]", field)) {
                    cp_str(&(ontology[fn].description), field);
                }
            }
   
            if ( !scmp("Rules:", c) ) {
                /* Exceptions/Rules Section template 
                 *  The Rules section will look like the following;
                    Rules:
                    [Slot1] == [Slot2]
                    prompt("actions here")
                    [Slot1] == [Slot2]
                    prompt("actions here")
                    ;
                 */
                int rule_pn = 0;
                while ( (r=fgets(line, LINE_LEN, fp)) ) {
                    for (c=line; isspace( (int) *c); c++);

                    if (*c == (char)NULL) { continue; }   // ignore blank lines
                    if (*c == '#') { continue; }    // ignore commented out lines   
                    if (*c == ';') { break; }       // indicates end of a task-frame
                    else if (*c == 'A') {  
                        /* action template - now add this prompt to the rule struct */
                        // isolate string
                        if( !(c= strchr(c, (int) '"')) ) continue;
                        c++;
                        prompt[0]= (char)0;
                        while( 1 ) {
                            // scan until end of line or quote
                            while( isspace( (int) *c) ) c++;
                            if( (cnt= sscanf(c, "%[^\"\r\n]", name)) < 1) break;
                            strcat(prompt,name);
                            if( (c= strchr(c, (int) '"')) ) break;
                            if( !(r= fgets(line, LINE_LEN, fp)) ) break;
                            c = line;
                        }
            
                        if( rule_pn == cnfg->MaxPrompt ) {
                            printf("WARNING: too many rule prompts\n");
                        }
                        else {
                            /* copy prompt string into rule */
                            (rule_buf+rule_num)->prompt[rule_pn] = (char *) NULL;
                            cp_str( &((rule_buf+rule_num)->prompt[rule_pn++]), prompt);
                            (rule_buf+rule_num)->n_prompt = rule_pn;
                        }
    
                    }
                    else { 
                        /* new rule */
                        rule_num++;
                        rule_pn = 0;
                        (rule_buf+rule_num)->n_prompt = 0;

                        // Parse Line and store rule
                        node = NULL;
                        node = parse_rule(c);
                        (rule_buf+rule_num)->rule_tree = node;
                    }
                }
                break;
            }

            /* new root net */
            if( (*c == '[') || (*c == '<') ) {
                prompt[0]= (char)0;
                pn=0;
                if( sscanf(c, "%[^+* \r\n]%s", name, prompt) < 1) continue;
                    slot_num++;
    
                /* copy key string into slot */
                (form_buf+slot_num)->key= (char *)0;
                cp_str( &((form_buf+slot_num)->key), name);
                
                // copy required char
                (form_buf+slot_num)->required = prompt[0];

                // initialize other slot values
                (form_buf+slot_num)->n_prompt = 0;

                for(i=0; i < cnfg->MaxPrompt; i++) {
                    (form_buf+slot_num)->prompt[i]= (char *)0;
                }
                (form_buf+slot_num)->sql= (char *)0;
                (form_buf+slot_num)->conf_prompt= (char *)0;
                strcpy(root, name);
            }

            /* new child net */
            else if( *c == '.' ) {
                prompt[0]= (char)0;
                pn= 0;
                c++;
                if( sscanf(c, "%[^+*\r\n]%s", name, prompt) < 1) continue;
                sprintf(path, "%s%s", root, name);
                    slot_num++;
        
                /* copy key string into slot */
                (form_buf+slot_num)->key= (char *)0;
                cp_str( &((form_buf+slot_num)->key), path);

                // copy required char
                (form_buf+slot_num)->required = prompt[0];

                // initialize other slot values
                (form_buf+slot_num)->n_prompt = 0;
                for(i=0; i < cnfg->MaxPrompt; i++)
                    (form_buf+slot_num)->prompt[i]= (char *)0;
                (form_buf+slot_num)->sql= (char *)0;
                (form_buf+slot_num)->conf_prompt= (char *)0;
            }
    
            /* prompt template */
            else if( *c == 'A' ) {
                if( !(c= strchr(c, (int) '"')) ) continue;
                c++;
                prompt[0]= (char)0;
                while( 1 ) {
                    while( isspace( (int) *c) ) c++;
                    if( (cnt= sscanf(c, "%[^\"\r\n]", name)) < 1) break;
                    strcat(prompt,name);
                    if( (c= strchr(c, (int) '"')) ) break;
                    if( !(r= fgets(line, LINE_LEN, fp)) ) break;
                    c= line;
                }
    
                if( pn == cnfg->MaxPrompt ) {
                    printf("WARNING: too many prompts\n");
                }
                else {
                    /* copy prompt string into slot */
                    (form_buf+slot_num)->prompt[pn]= (char *)0;
                    cp_str( &((form_buf+slot_num)->prompt[pn++]), prompt);
                    (form_buf+slot_num)->n_prompt = pn;
                }
            }

#ifdef old
            /* action sequence */
            else if( *c == 'A' ) {
                if( !(c= strchr(c, (int) '"')) ) continue;
                c++;
                prompt[0]= (char)0;
                while( 1 ) {
                    while( isspace( (int) *c) ) c++;
                    if( (cnt= sscanf(c, "%[^\"\r\n]", name)) < 1) break;
                    strcat(prompt,name);
                    if( (c= strchr(c, (int) '"')) ) break;
                    if( !(r= fgets(line, LINE_LEN, fp)) ) break;
                    c= line;
                }

                /* copy sql string into slot */
                (form_buf+slot_num)->action= (char *)0;
                cp_str( &((form_buf+slot_num)->action), prompt);
            }
#endif


            /* sql template */
            else if( *c == 'S' ) {
                if( !(c= strchr(c, (int) '"')) ) continue;
                c++;
                prompt[0]= (char)0;
                while( 1 ) {
                    while( isspace( (int) *c) ) c++;
                    if( (cnt= sscanf(c, "%[^\"\r\n]", name)) < 1) break;
                    strcat(prompt,name);
                    if( (c= strchr(c, (int) '"')) ) break;
                    if( !(r= fgets(line, LINE_LEN, fp)) ) break;
                    c= line;
                }
    
                /* copy sql string into slot */
                (form_buf+slot_num)->sql= (char *)0;
                cp_str( &((form_buf+slot_num)->sql), prompt);
            }
    
            /* confirm prompt template */
            else if( *c == 'C' ) {
                if( !(c= strchr(c, (int) '"')) ) continue;
                c++;
                prompt[0]= (char)0;
                while( 1 ) {
                    while( isspace( (int) *c) ) c++;
                    if( (cnt= sscanf(c, "%[^\"\r\n]", name)) < 1) break;
                    strcat(prompt,name);
                    if( (c= strchr(c, (int) '"')) ) break;
                    if( !(r= fgets(line, LINE_LEN, fp)) ) break;
                    c= line;
                }

                /* copy prompt string into slot */
                (form_buf+slot_num)->conf_prompt= (char *)0;
                cp_str( &((form_buf+slot_num)->conf_prompt), prompt);
            }

        }
        if( !r ) {
            fprintf(stderr, "Bad format in forms file\n");
            exit(-1);
        }

        slot_num++;
        rule_num++;

        /* zero out rule_visits */
        for (i = 0; i < MAX_RULE; i++) { (ontology+fn)->rule_visits[i] = 0; }

        /* copy to frame definition structure */
        (ontology+fn)->n_slot= slot_num;
        (ontology+fn)->n_rule = rule_num;

        /* malloc space for slot array */
        if( !( (ontology+fn)->slot= (Slot *)malloc(slot_num * sizeof(Slot)))){
        fprintf(stderr, "ERROR: malloc form def failed\n");
          exit(-1);
        }

        /* malloc space for rules array */
        if( !( (ontology+fn)->rule= (Rule *)malloc(rule_num * sizeof(Rule)))){
        fprintf(stderr, "ERROR: malloc form def failed\n");
          exit(-1);
        }

        /* copy temporary structure */
        for(j = 0; j < slot_num; j++) { (ontology+fn)->slot[j] = form_buf[j]; }
        for(j = 0; j < rule_num; j++) { (ontology+fn)->rule[j] = rule_buf[j]; }

    }
    fclose(fp);
}

/* prints out a task file to FILE handle */
void write_task(aFrameDef* task, FILE* fp, Config* cnfg) {

    int i;
    int j;
    int k;
    aFrameDef* frame;
    Slot*      slot;
    Rule*      rule;
    char*      prompt;

    for (i = 0; i < cnfg->task_frames; i++) {
        
        frame = &task[i];
        fprintf(fp, "Frame: %s\n", frame->name);
        fprintf(fp, "Description: %s\n", frame->description);

        // Iterate over slots (elements)
        for (j = 0; j < frame->n_slot; j++) {
            slot = &(frame->slot[j]); 
            fprintf(fp, "%s", slot->key);
            if (slot->required == '*' || slot->required == '+') {
              fprintf(fp, "%c", slot->required);
            }
            fprintf(fp, " \n");


            // Iterate over prompts (actions)
            for (k = 0; k < slot->n_prompt; k++) { 
                prompt = slot->prompt[k];
                if (prompt == NULL) { break; }
                fprintf(fp, "    Action: \"%s\"\n", prompt);
            }

        } // end for (slots in frame)

        if (frame->n_rule > 0) { 
            fprintf(fp, "Rules:\n");
        }
        // Iterate over rules
        for (j = 0; j < frame->n_rule; j++) {
            rule = &(frame->rule[j]);
            print_rules_tree(fp, rule->rule_tree);
            fprintf(fp, "\n");
            for (k = 0; k < rule->n_prompt; k++) {
                prompt = rule->prompt[k];
                if (prompt == NULL) { break; }
                fprintf(fp, "    Action: \"%s\"\n", prompt);
            }
        } // end for(rules in frame)
        fprintf(fp, ";\n\n");

    } // end for(frames in task)

}


char** get_frame_prompts(aFrameDef* frame, int* pnum_prompts_tot, Config* cnfg) {
    int i;
    int j;
    int pn = 0;
    Slot* slot;
    Rule* rule;
    char ** prompts;
    char stripped[LINE_LEN];

    *pnum_prompts_tot = 0;
    // Iterate over slots (elements)
    for (i = 0; i < frame->n_slot; i++) {
        slot = &(frame->slot[i]); 
        *pnum_prompts_tot += slot->n_prompt;
    }

    // Iterate over rules
    for (i = 0; i < frame->n_rule; i++) {
        rule = &(frame->rule[i]);
        *pnum_prompts_tot += rule->n_prompt;
    }

    prompts = (char**) calloc(*pnum_prompts_tot, sizeof(char*));

    for (i = 0; i < *pnum_prompts_tot; i++) {
        prompts[i] = (char*) calloc(LINE_LEN, sizeof(char));
    }

    // Iterate over slots (elements)
    // Copy prompts into buffer
    for (i = 0; i < frame->n_slot; i++) {
        slot = &(frame->slot[i]); 
        for (j = 0; j < slot->n_prompt; j++) {
            strip_dm_actions(slot->prompt[j], stripped, cnfg);
            strcpy(prompts[pn], stripped);
            pn++;
        }
    }

    // Iterate over rules (elements)
    // Copy prompts into buffer
    for (i = 0; i < frame->n_rule; i++) {
        rule = &(frame->rule[i]); 
        for (j = 0; j < rule->n_prompt; j++) {
            strip_dm_actions(rule->prompt[j], stripped, cnfg);
            strcpy(prompts[pn], stripped);
            pn++;
        }
    }

    return  prompts;
}






void    nl_request(char *speech_act, char *prompt, char *response)
{

    if( !scmp("prompt_user", speech_act) ) {
        sprintf(response,"%s", prompt);
    }
    else if( !scmp("continue", speech_act) ) {
        sprintf(response,"%s", "speak(continue)");
    }

    
}

/* find best parse in buffer, given current context */
char *get_best_parse(char *parse_str, Config* cnfg)
{
    char    *s,
            *p,
            *best;
    int     num_parses;
    int     score, best_score;
    int     i;

    best= (char *)0;
    best_score= 0;
    // iterate through all the parses
    for(s= parse_str, num_parses= 0; *s; num_parses++) {
        score= 0;
        /* find start of next parse */
        if( !(s= strstr(s, "PARSE")) ) break;
        if( !(s= strchr(s, (int) '\n')) ) break;
        if( !*(++s)  ) break;
        p= s;
    
        /* find end of parse */
        if( !(s= strstr(s, "END_PARSE")) ) break;
        *s++= (char)0;
    
        /* see if responsive to prompt */
        if( prompted.frame && prompted.key ) {
    
            // Parse our frame and key from the phoenix parse
            char parsed_frame[100];
            char parsed_key[100];
            sscanf(p, "%[^:]:%[^\n]", parsed_frame, parsed_key);
            printf("lb: Parsed frame: %s\n", parsed_frame);
            
            if (!strcmp(prompted.frame->name, parsed_frame)) {
                if (strstr(parsed_key, prompted.key)) {
                    // parsed input matches the frame and the key (element)
                    // (best possible match)
                    score += 100000;
                } else {
                    // parsed input matches just the frame
                    // (next best match)
                    score += 10000;
                }
            }
    
        }
    
        /* see if same frame in context, pick one closest to top of stack */
        if( !score ) {
            // There was no match to the current frame.  Find other candidate frames
            for( i=fc_idx; i >= 0; i--) {
                // Parse our frame and key from the phoenix parse
                char parsed_frame[100];
                char parsed_key[100];
                sscanf(p, "%[^:]:%[^\n]", parsed_frame, parsed_key);
                printf("lb: Parsed frame: %s\n", parsed_frame);
        
                if ( !strcmp(parsed_frame, context[i]->name) ) {
                    // frame is in not in context pick one closest to top of the stack
                    score += i * 5;
                    break;
                } else {
                    // frame is not in the stack, now look through all frames on the stack, to see if they push
                    // any frames onto the stack.  The preferred parse in the absence of any other information will
                    // choose the frame deepest in the stack and highest in a frame's element ranking
                    int        slot_num;
                    aFrameDef* fd;
                    aFrame*    frame;
                    int        xx;
    
                    // look at frame on the stack
                    fd = getFrameDef(context[i]->name, cnfg);
                    frame = getFrame(context, context[i]->name, "first", &xx); 

                    for (slot_num = 0; slot_num < fd->n_slot; slot_num++) {
                        // look at elements in the frame and see if any push a frame matching the parsed frame
                        // into focus
                        char* key = (fd->slot+slot_num)->key;
                        char frame_name[LABEL_LEN];
                        char* end;
                        if (key[0] == '<') {
                            strcpy(frame_name, key+1);
                            if ( (end = strchr(frame_name, '>') ) ) { *end = (char) 0; }

                            if ( !strcmp(frame_name, parsed_frame) && !frame->value[slot_num] ) {
                                // Formula weights stack depth of frame more heavily followed by order of frame elements
                                score += 1000 * (fc_idx - i) + 100 * (fd->n_slot-slot_num);
                                break;
                            }
                        }
    
                    } // end for (slots in frame)
                }
            } // end for (i in fcidx)
        } // end if (!score)
    
        if( !best || (score > best_score)) {
            best= p;
            best_score= score;
        }
    
        if( !(s= strchr(s, (int) '\n')) ) break;
    } // end for s in num_parses
    return(best);
}




