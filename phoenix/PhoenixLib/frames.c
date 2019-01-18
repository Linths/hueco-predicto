/* routines to manipulate frames */

#ifdef WIN
#include "stdafx.h"
#include <stdlib.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "phoenix.h"
#include "dm.h"
#include "rules.h"

#define MAX_FRAME_STACK 100

//#define DEBUG 1

extern aFrame	*context[MAX_FRAME_STACK],
		*extracts[MAX_FRAME_STACK],
/* Currently commented out.  Someday to be used for anaphora resolution
		*history[MAX_FRAME_STACK],
*/
		*completed[MAX_FRAME_STACK],
		*focus;
extern int	fc_idx; /* index of top of context stack */
extern Prompt	prompted;
extern aFrameDef	*ontology;
extern char		prompt_text[LINE_LEN],
			*prompt;


/* function declarations */
void	activateFrame(aFrame *frame, Config *cnfg);
void	addFrame(aFrame **list, aFrame *frame);
int 	child_filled(aFrame *frame, aFrameDef *fd, int *node);
int	cmpVal(aFrame *frame, char *key, char *val, Config *cnfg);
void	cp_frame(aFrame *to, aFrame *from, Config *cnfg);
void	cp_values(aFrame *to, aFrame *from, Config *cnfg);
aFrame	*createFrame(char *name, Config *cnfg);
char	*confirm_values( aFrame *frame, Config *cnfg);
void	expand_prompt(aFrame *frame, char *prompt, char *buf, Config *cnfg);
char	*fill_child(aFrame *frame, aFrameDef *fd, int *node, Config *cnfg);
int frame_complete( aFrame *frame, char *prompt, Config *cnfg);
int	frame_empty(aFrame *frame, Config *cnfg);
void	free_list(aFrame **list);
aFrame *getFrame(aFrame **list, char *name, char *ctl, int *fidx);
aFrameDef	*getFrameDef( char *name, Config *cnfg );
char	**getKeyPtr(aFrame *frame, char *key, Config *cnfg);
char	*getVal(aFrame *frame, char *key, Config *cnfg);
char	*is_child( aFrame *frame, char *slot, char *child, Config *cnfg);
void	print_frame( aFrame *frame, Config *cnfg);
void	setKey(aFrame *frame, char *key, char *val, Config *cnfg);
void	set_prompted(aFrame *frame, char *key, char *type);
void	skip( aFrame *frame, aFrameDef *fd, int *node);
char	*prompt_for_key( aFrame *frame, char *key, char *type, Config *cnfg);
char	*prompt_for_node( aFrame *frame, aFrameDef *fd, int node, Config *cnfg);

/* functions in util */
void	cp_str(char **to, char *from);
int		scmp(char *s1, char *s2);

aFrame *createFrame(char *name, Config *cnfg)
{
    aFrame      *new_frame;
    aFrameDef   *fd;

    /* find frame definition */
    if( !(fd= getFrameDef(name, cnfg)) ) {
        printf("frame: %s not found\n", name);
        exit(-1);
    }

    /* malloc space for frame */
    if( !(new_frame=(aFrame *) calloc(1, sizeof(aFrame)) )){
        fprintf(stderr, "ERROR: can't allocate space for frame\n");
        exit(-1);
    }

    /* copy frame name */
    cp_str( &(new_frame->name), name);

    /* malloc space for values */
    if( !(new_frame->value=(char **) calloc(fd->n_slot, sizeof(char *)) )){
        fprintf(stderr, "ERROR: can't allocate space for slots\n");
        exit(-1);
    }

    /* malloc space for confirmed flags */
    if( !(new_frame->confirmed=(char *) calloc(fd->n_slot, sizeof(char)) )){
        fprintf(stderr, "ERROR: can't allocate space for slots\n");
        exit(-1);
    }

    /* malloc space for consistent flags */
    if( !(new_frame->consistent=(char *) calloc(fd->n_slot, sizeof(char)) )){
        fprintf(stderr, "ERROR: can't allocate space for slots\n");
        exit(-1);
    }

    /* malloc space for prompt_count */
    if( !(new_frame-> prompt_count=(char *) calloc(fd->n_slot, sizeof(char)) )){
        fprintf(stderr, "ERROR: can't allocate space for prompt counts\n");
        exit(-1);
    }

    return(new_frame);

}

void activateFrame(aFrame *frame, Config *cnfg) {
    if( !cmpVal(frame, "[Active]", "yes", cnfg) ) return;
    setKey(frame, "[Active]", "yes", cnfg);
}


aFrame *pushFrame(char *fname, aFrame **context, aFrame **completed, 
                  int* context_idx, aFrame *rtn_frame, int rtn_element, Config *cnfg) {
    int     i, j, k;
    aFrame  *frame;

    i= (*context_idx) +1;
    if( i >=  MAX_FRAME_STACK ) {
        fprintf(stderr, "WARNING: Too many frames\n");
        return( (aFrame *)0 );
    }

    // Check the completed frames stack for the requested frame
    for (j = 0; j < MAX_FRAME_STACK; j++) {
        // *note* completed frames stack is 0 padded
        if (!completed[j]) { break; }

        if (!strcmp(completed[j]->name, fname)) {
            // The requested frame is in the completed frames stackreturn a null frame
            return( (aFrame *)0 );
        }
    }

    // Check the context stack for the requested frame
    for (j = 0; j <= *context_idx; j++) {
        if (!strcmp(context[j]->name, fname)) {
            // the requested frame is in the context, now rearrange the stack
            // so that the requested frame sits on top
            aFrame* newTopFrame = context[j];
            for (k = j; k  < *context_idx; k++) {
                context[k] = context[k+1];
            }
            context[*context_idx] = newTopFrame;
            return(newTopFrame);
        }
    }

    if( !(frame= createFrame(fname, cnfg)) ) {
        printf("can't create frame %s\n", fname);
        return( (aFrame *)0 );
    }

    frame->rtn_f= rtn_frame;
    frame->rtn_e= rtn_element;

    context[i]= frame;
    *context_idx= i;
    return(frame);

}

void popFrame()
{
    aFrame *frame;
    aFrame *next;
    char *s;

    if( fc_idx < 0 ) return;
    frame= context[fc_idx];

    /* add frame to completed frames stack */
    addFrame(completed, frame);

    fc_idx--;
    if( fc_idx < 0 ) return;
    next= context[fc_idx];

    /* mark calling element as filled */
    if( !(s=(char *)malloc(5) )){
        fprintf(stderr, "ERROR: can't allocate space for value\n");
        exit(-1);
    }
    strcpy(s, "TRUE");
    /* free any existing value */
    if( next->value[frame->rtn_e] ) free(next->value[frame->rtn_e]);
    next->value[frame->rtn_e]= s;
}


/* return pointer to frame on top of stack */
aFrame *getActiveFrame( Config *cnfg )
{

    if( fc_idx >= 0 ) return( context[fc_idx] );
    return( (aFrame *) 0 );
}


void setKey(aFrame *frame, char *key, char *val, Config *cnfg) {
    char **slot_ptr;

    if( !(slot_ptr=getKeyPtr(frame, key, cnfg)) ) return;

    /* free any existing value */
    if( *slot_ptr ) free(*slot_ptr);
    *slot_ptr= 0;

    if( !val ) return;

    if( !(*slot_ptr=(char *)malloc(strlen(val)+1) )){
        fprintf(stderr, "ERROR: can't allocate space for slots\n");
        exit(-1);
    }
    strcpy(*slot_ptr, val);
}


/* return 0 if key value matches val, 1 otherwise */
int cmpVal(aFrame *frame, char *key, char *val, Config *cnfg)
{
    char **slot_ptr;

    if( !(slot_ptr=getKeyPtr(frame, key, cnfg)) ) return(1);
    /* null slot matches val=0 */
    if( !*slot_ptr ) {
        if( !val ) return(0);
        else       return(1);
    }
    if( !val ) return(1);

    return( strcmp( *slot_ptr, val) );

}

/* return pointer to value string */
char *getVal(aFrame *frame, char *key, Config *cnfg)
{
    char **slot_ptr;


    if( !(slot_ptr=getKeyPtr(frame, key, cnfg)) ) return( (char *)0 );
    /* null slot matches val=0 */
    return( *slot_ptr );

}

void addFrame(aFrame **list, aFrame *frame) {
    int     i;
    aFrame  **f;

    for(i=0, f= list; i < MAX_FRAME_STACK; i++, f++) {
        if( !*f ) break;
    }
    if( i >=  MAX_FRAME_STACK ) {
        printf("Too many frames\n");
        return;
    }
    *f= frame;
}


void freeFrame(aFrame *frame) {
    if( frame->name ) free(frame->name);
    if( frame->value ) free(frame->value);
    if( frame->confirmed ) free(frame->confirmed);
    if( frame->consistent ) free(frame->consistent);
    if( frame->prompt_count ) free(frame->prompt_count);
    free(frame);
}

void    clear_all_frames() {
    free_list(extracts);
    free_list(context);
/* Currently commented out.  Someday to be used for anaphora resolution
    free_list(history);
*/
    free_list(completed);
}

void free_list(aFrame **list) {
    int     i;
    aFrame  **f;

    for(i=0, f= list; i < MAX_FRAME_STACK; i++, f++) {
        if( !*f) break;
        freeFrame(*f);
        *f= (aFrame *)0;
    }
}

/* copy list of frames to end of history */
/* Currently commented out.  Someday to be used for anaphora resolution
void cp_to_hist(aFrame **from, aFrame **history, int *hist_idx) {
    int     i;
    aFrame  **f;

    for(i=0, f= from; i < MAX_FRAME_STACK; i++, f++) {
        if( !*f) break;
        if( *hist_idx == MAX_FRAME_STACK ) *hist_idx= 0;
        history[*hist_idx++]= *f;
    }
}
*/


void copy_stack(aFrame **src, aFrame **tgt, int* idx) {
    int i;

    for (i = 0; i < MAX_FRAME_STACK; i++) {
        if (!src[i]) { break; }
        if(idx && *idx == MAX_FRAME_STACK) { *idx = 0; }
        tgt[*idx++] = src[i];
        //tgt[i] = src[i];
    }

}

void reset_stack(aFrame **stack) {
    int i;

    for (i = 0; i < MAX_FRAME_STACK; i++) {
        if (stack[i] == NULL) { break; }
        stack[i] = (aFrame*) NULL;
    }
}



aFrame *get_current_frame(char *name, aFrame *focus)
{
    int i;

    /* if same as current focus frame */
    if( focus && !strcmp(focus->name, name) ) return(focus);

    /* if same as prompted frame */
    if( prompted.frame && !strcmp(prompted.frame->name, name) ) {
        return(prompted.frame);
    }

    /* find end of stack */
    for(i=0; i < MAX_FRAME_STACK; i++) if( !context[i]) break;
        if( i == 0 ) return( (aFrame *)0 );

        for(i--; i >= 0; i--) {
            if( !strcmp(name, context[i]->name) ) return( context[i] );
        }
    return( (aFrame *)0 );
}


/* return 1 if node complete, 0 otherwise */
int node_done( aFrame *frame, aFrameDef *fd, int *node)
{
    int     or,
            done;
    char    *key;
    Slot    *s;

    if( *node >= fd->n_slot ) return(0);

    /* if value for node */
    if( *(frame->value + *node) ) return(1);

    if( !(key= (fd->slot+*node)->key) ) return(1);

    /* if no children */
    if( (*node == (fd->n_slot-1)) ||
        scmp(key, (fd->slot + *node+1)->key) ) {
        return(0);
    }

    /* for each child of node */
    or= 0;
    for( (*node)++; *node < fd->n_slot; ) {
        s= fd->slot + *node;
        if( scmp(key, s->key) ) break;
        if( !s->required ) {skip(frame, fd, node); continue;}
        else if( s->required == '*' ) or= 1;

        done= node_done( frame, fd, node);
        if( (s->required == '+') && !done ) return(0);
        if( (s->required == '*') && done ) return(1);
        (*node)++;
    }
    if( !or ) return(1);
    else return(0);

}

/* prompt for missing info, 0 if none */
char *missing_info( aFrame *frame, aFrameDef *fd, int *node, Config *cnfg)
{
    char    *key;
    Slot    *s;
    char    *p;
    int     i;
    int     done;
    int     pre_node;
	int		frame_is_empty;
	
    if( *node >= fd->n_slot ) return( (char *)0);

    /* if node complete, nothing needed */
    i= *node;
    done= node_done( frame, fd, &i);
    if( done ) {
        skip(frame, fd, node); return( (char *)0);
    }

	// If frame already has a value filled, skip the start node
	frame_is_empty = frame_empty(frame,cnfg);
	key= (fd->slot+ *node)->key;
	if (!frame_is_empty && !scmp(key, "[_start]")) {	
		setKey(frame, key, "TRUE", cnfg);
		skip(frame, fd, node);
		return ( (char*) 0);
	}
	
    /* if node not required */
    s= fd->slot + *node;
    if( !s->required ) { skip(frame, fd, node); return( (char *)0);}

    /* if prompt for node */
    if( (p= (fd->slot + *node)->prompt[0]) ) {
        if( (p= prompt_for_node(frame, fd, *node, cnfg)) ) { return(p); }
        else { return( (char *)0 ); }
    }

    /* if no key value for node */
    if( !(key= (fd->slot+ *node)->key) ) { (*node)++; return( (char *)0 );}


	
	/* for each child of node */
    for((*node)++ ; *node < fd->n_slot;  ) {		
        s= fd->slot + *node;
        if( scmp(key, s->key) ) break; /* end of kids */
        /* skip node if not required */
        if( !s->required ) { skip(frame, fd, node); continue;}

        /* skip node if already filled */
        i= *node;
        done= node_done( frame, fd, &i);
        if( done ) {skip(frame, fd, node); continue;}

        /* check prompt for child */
        if( (p= (fd->slot + *node)->prompt[0]) ){
            if( (p= prompt_for_node(frame, fd, *node, cnfg)) ) return(p);
            else return( (char *)0 );
        }
        pre_node= *node;
        if( (p= missing_info( frame, fd, node, cnfg)) ) return(p);
        if( *node == pre_node) (*node)++;
    }
    return( (char *)0);

}



char **getKeyPtr(aFrame *frame, char *key, Config *cnfg)
{
    int         i;
    aFrameDef   *fd;
    Slot        *s;

    if( !frame || !key ) return( (char **)0);

    /* find frame definition */
    if( !(fd= getFrameDef(frame->name, cnfg)) ) return( (char **)0);

    for( i=0, s= fd->slot; i < fd->n_slot; i++, s++ ){
        if( !scmp(key, s->key) ) break;
    }

    if( i < fd->n_slot ) return ( (frame->value)+i );
    return( (char **)0 );

}

char *getNodeKey(aFrame *frame, int node, Config *cnfg)
{
    aFrameDef   *fd;

    if( !frame ) return( (char *)0);

    /* find frame definition */
    if( !(fd= getFrameDef(frame->name, cnfg)) ) return( (char *)0);

    /* get key associated with node */
    if( node < fd->n_slot ) return ( ((fd->slot)+node)->key );
    return( (char *)0 );

}


void set_prompted(aFrame *frame, char *key, char *type)
{
    prompted.frame= frame;
    prompted.key= key;
    prompted.type= type;
}


aFrame *getFrame(aFrame **list, char *name, char *ctl, int *fidx)
{
    int     i, found;
    aFrame  **f;

    if( scmp("next", ctl)) *fidx= -1;

    if( *fidx < 0 ) i= 0;
    else i= (*fidx) +1;

    found= -1;
    for( f= list+i ; i < MAX_FRAME_STACK; i++, f++) {
        if( !*f) break;
        if( !scmp("*", name) ) {
            found= i;
            if( scmp("last", ctl)) break;
        }

        if( scmp((*f)->name, name) ) continue;
        /* found next frame with name */
        found= i;
        if( scmp("last", ctl)) break;
    }
    if( found < 0 ) {
        return( (aFrame *)0 );
    }
    *fidx= found;
    return( *(list+found) );
}

aFrame *findFrame(char *name, char *key, char *val, Config *cnfg)
{
    int i;

    for( i=0 ; i < MAX_FRAME_STACK; i++) {
        if( !context[i]) break;
        if( scmp(context[i]->name, name) ) continue;
        /* found next frame with name */
        if( !cmpVal(context[i], key, val, cnfg) ) return(context[i]);
    }
    return( (aFrame *)0 );
}



void print_frame( aFrame *frame, Config *cnfg)
{
    int         i;
    aFrameDef   *fd;
    char        **val;
    Slot        *s;

    if( !frame ) return;
    if( !(fd= getFrameDef(frame->name, cnfg)) ) return;

    printf("Frame: %s\n", frame->name);
    /* for each slot in frame */
    s= fd->slot;
    for(i=0, val= frame->value; i < fd->n_slot; i++, val++, s++) {
        if( !*val ) continue;
        printf("    %s:%s\n", s->key, *val);
    }

}


/* return pointer to definition struct for frame */
aFrameDef *getFrameDef( char *name, Config *cnfg )
{
    aFrameDef   *fd;
    int         i;

    if( !name ) return( (aFrameDef *)0 );

    for(i=0, fd= ontology; i < cnfg->task_frames; i++, fd++) {
        if( !scmp(name, fd->name) ) break;
    }

    if( i < cnfg->task_frames ) return( fd );

    return( (aFrameDef *)0 );
}


void skip( aFrame *frame, aFrameDef *fd, int *node)
{
    char *key;
    Slot *s;

    key= (fd->slot + *node)->key;
    for( (*node)++; *node < fd->n_slot; (*node)++ ) {
        s= fd->slot + *node;
        if( scmp(key, s->key) ) break;
    }
}


/* check frame for completion
   if frame not complete, prompt for missing info 
   return 1 if done, 0 otherwise */
int frame_complete( aFrame *frame, char *prompt, Config *cnfg)
{
    int node_idx;
    int fdone;
    int i;
    aFrameDef *fd;
    char *prmpt;
    char name[LABEL_LEN];
    char *s;
    int exceptions = 0;
    int check_rules;
    char rule_prompt[LINE_LEN];

    if( !(fd= getFrameDef(frame->name, cnfg)) ) return( -1);

    prmpt= (char *)0;

    fdone= 0;

    exceptions = 0;

    // Check rules if
    // 1) The frame is not the Start frame
    // 2) The frame is not empty
    check_rules = scmp("Start", fd->name) && !frame_empty(frame, cnfg);

    // Iterate and check over the rules
    for (i = 0; check_rules && i < fd->n_rule; i++) {
        // Limit number of times rules can be visited
        int num_visits = fd->rule_visits[i];
        if (num_visits == MAX_RULE_VISITS || 
            num_visits == fd->rule[i].n_prompt) { continue; }

        exceptions = rules_node_eval((struct aFrame*)frame, (struct Config*) cnfg, fd->rule[i].rule_tree); 
#if DEBUG
        // Print how the rules are firing
        printf("rule %d (n_prompt = %d): ", i, fd->rule[i].n_prompt);
        print_rules_tree(fd->rule[i].rule_tree);
        printf("exceptions = %d\n", exceptions);
#endif

        if ( exceptions ) {
            // This rule is firing
            // Store the prompt for response

            // get current prompt number by modding number of visits by
            // total number of prompts for the rule
            int prompt_num = fd->rule_visits[i];
            strcpy(rule_prompt, fd->rule[i].prompt[prompt_num] ); 

            // Expand templates in prompt
            expand_prompt(frame, rule_prompt, prompt, cnfg);
            // Set prompted frame so that get_best_parse gets proper rules match
            set_prompted(frame, "__RULES_DUMMY_KEY__", "value");

            // increment number of times this rule has been exercised.
            fd->rule_visits[i]++; 
            break;
        }

    }


    // If there are no exceptions firing perform slot filling like normal
    if (!exceptions) {		
        for(node_idx= 0; node_idx < fd->n_slot; ) {
            i= node_idx;
            /* if node not filled */
            if( !(*(frame->value + node_idx)) ) {
                /* if push new frame */
                if( (fd->slot + node_idx)->key[0] == '<' ) {
                    aFrame* f;
                    /* get name of frame to push */
                    strcpy(name, ((fd->slot + node_idx)->key)+1);
                    if( (s= strchr(name, (int) '>')) ) *s= (char)0;

                    // Attempt to push the frame onto the stack
                    f = pushFrame(name, context, completed, &fc_idx, frame, node_idx, cnfg);
                    if (!f) {
                        // frame requested was not pushed onto the context stack 
                        // (probably already on completed frames stack)
                        // set the frame to DONE
                        setKey(frame,(fd->slot+node_idx)->key, "DONE", cnfg);
                    }
                    return( 0 );
                }
                else {
        
                    /* get prompt */
                    if( (prmpt= missing_info( frame, fd, &node_idx, cnfg)) ) { break; }
                    
                }
            }
            if( node_idx == i ) node_idx++;
        }
    
        /* if all required slots filled */
        if( node_idx >= fd->n_slot ) {
            fdone= 1;
        }
        else {
            /* if [_done] node, set to TRUE */
            if( !strcmp( (fd->slot + node_idx)->key, "[_done]") ) {
                    setKey(frame,"[_done]", "TRUE", cnfg);
                fdone= 1;
            }
        }
    }


    // Do prompt expansion as needed
    if( prmpt ) {
        expand_prompt(frame, prmpt, prompt, cnfg);
        set_prompted( frame, (fd->slot + node_idx)->key, "value");
        if( !strcmp( (fd->slot + node_idx)->key, "[_start]") ) {
            setKey(frame,"[_start]", "TRUE", cnfg);
        }
    }

    return(fdone);
}

char *frame_confirm( aFrame *frame, aFrame *focus, Config *cnfg)
{
    char  *prmpt;

    /* if complete but not confirmed, then confirm */
    if( !(prmpt= getVal(frame,"[confirmed]", cnfg)) ) {
    if( !(prmpt= prompt_for_key(frame,"[confirmed]", "yes", cnfg)) ) {
        return( (char*)0 );
    }
    expand_prompt(frame, prmpt, prompt_text, cnfg);
    focus= frame;
    return(prompt_text);
    }
    /* else if confirmed, return */
    else if( !scmp("yes", prmpt) ) {
    return( (char*)0 );
    }
    /* else if disconfirmed, confirm each required slot */
    else {
    if( (prmpt= confirm_values(frame, cnfg)) ) {
        expand_prompt(frame, prmpt, prompt_text, cnfg);
        return(prompt_text);
    }

    focus= frame;

    /* set confirmed slot */
    setKey(frame,"[confirmed]", "yes", cnfg);
    }
    return( (char*)0 );
}


void expand_prompt(aFrame *frame, char *prompt, char *buf, Config *cnfg)
{
    char *p, *f;
    char *s;
    char  name[LINE_LEN];


    /* copy template string until substitute char */
    p= buf;
    for( f= prompt; *f; f++) {
      switch(*f) {
        case '$':
                if( *(f+1) == '[' ) {
                    if( sscanf(f+1, "%s", name) < 1 ) break;
                    if( !(s= getVal(frame, name, cnfg)) ) break;
                    sprintf(p, "%s", s);
                    p += strlen(p);
                    f += strlen(name);
                    break;
                }
                else if( *(f+1) == '(' ) {
                    if( !(s= strchr(f+1, (int) ']')) ) break;
                    if( !(s= strchr(f+1, (int) '[')) ) break;
                    if( sscanf(s, "%s", name) < 1 ) break;
                    for(s= name+ strlen(name)-1; *s && (*s != ']');s--)
                        *s= (char)0;
                    if( !(s= getVal(frame, name, cnfg)) ) {
                        if( (s= strchr(f+1, (int) ')')) ) f= s;
                        break;
                    }
                    strcpy(name,s);
                    for( f= f+2; *f && (*f != '['); f++) *p++= *f;
                    if( !*f ) break;
                    strcpy(p, name);
                    p += strlen(p);
                    for( ; *f && (*f != ' ') && (*f != ')'); f++);
                    for( ; *f && (*f != ')'); f++) *p++= *f;
                    break;
                }
                break;

        default:
                *p++= *f;
      }
    }
    *p= 0;
}



char *is_child( aFrame *frame, char *slot, char *child, Config *cnfg)
{
    aFrameDef   *fd;
    char        *key;
    int          node;

    if( !(fd= getFrameDef(frame->name, cnfg)) ) return( (char *)0 );

    /* find key */
    for(node= 0; node < fd->n_slot; node++ ) {
        /* if no key for node */
        if( !(key= (fd->slot+ node)->key) )  return( (char *)0 );
    if( !scmp(slot, key ) ) break;
    }
    if( node == fd->n_slot ) return( (char *)0 );

    for( ; node < fd->n_slot; node++ ) {
        if( !(key= (fd->slot+ node)->key) )  return( (char *)0 );
    if( scmp(slot, key ) ) break;
    if( strstr(key, child) ) return(key);
    }

    return( (char *)0 );
}

/* return confirmation prompt for next unconfirmed value */
char *confirm_values( aFrame *frame, Config *cnfg)
{
    aFrameDef    *fd;
    Slot    *s;
    char    *p;
    int        i;
    char    **val;

    if( !(fd= getFrameDef(frame->name, cnfg)) ) return( (char *)0 );

    /* for each node in frame */
    s= fd->slot;
    for(i=0, val= frame->value; i < fd->n_slot; i++, val++, s++) {
    /* if no value for node */
    if( !*val ) continue;

    /* if already confirmed */
    if( (int) *(frame->confirmed + i) != 0 ) continue;

        /* if no key value for node */
        if( !s->key ) continue;

        /* if prompt for node */
        if( (p= (fd->slot + i)->conf_prompt) ) {
         set_prompted(frame, s->key, "confirm");
         return( p );
        }
    }
    return( (char *)0 );
}


/* get prompt for value for specified node */
char *prompt_for_node( aFrame *frame, aFrameDef *fd, int node, Config *cnfg)
{
    Slot    *s;
    char    *p,
            *np,
            *kv;
    int     i,
            cnt;

    /* if prompt for node */
    s= fd->slot + node;
    if( (p= s->prompt[0]) ) {

        /* if some children already filled, fill other children */
        /* if more than second time, fill children */
        i= node;
        cnt= (int) *(frame->prompt_count + node);
        if( child_filled(frame, fd, &i) || (cnt >= cnfg->MaxPrompt )) {
            if( (np= fill_child(frame, fd, &i, cnfg)) ) {
                kv= getNodeKey(frame, i, cnfg);
                set_prompted(frame, kv, "value");
                (*(frame->prompt_count + i))++;
                return(np);
            }
        }
    
        /* if less than max count go ahead and prompt */
        if( cnt < cnfg->MaxPrompt ) {
            kv= getNodeKey(frame, node, cnfg);
            set_prompted(frame, kv, "value");
            p= s->prompt[cnt];

            // Following if statement will 
            if (!s->prompt[cnt+1]) {
                setKey(frame, kv, "FAIL", cnfg);
            }

            if( (cnt < (cnfg->MaxPrompt-1)) && (s->prompt[cnt+1])) {
                (*(frame->prompt_count + node))++;
            }
            if(p) return(p);
        }
    }
    return( (char *)0 );
}

/* return prompt for specified field */
char *prompt_for_key( aFrame *frame, char *key, char *type, Config *cnfg)
{
    aFrameDef   *fd;
    Slot        *s;
    char        *p,
                *kv;
    int         i, node;

    /* get pointer to frame definition structure */
    if( !(fd= getFrameDef(frame->name, cnfg)) ) return( (char *)0 );

    /* find node in frame */
    for(i= 0, s= fd->slot; i < fd->n_slot; i++, s++) {
        /* if no key value for node */
        if( !(kv= s->key) ) continue;
        /* if not specified key */
        if( !scmp(key, kv) ) break;
    }
    if( i >= fd->n_slot) return( (char *)0 );
    node= i;

    /* if prompt for node */
    if( (p= s->prompt[0]) ) {
        if( (p= prompt_for_node(frame, fd, node, cnfg)) ) {
            prompted.type= type;
            return(p);
        }
    }
    return( (char *)0 );
}

void setConfirm(aFrame *frame, char *key, int val, Config *cnfg) {
    int         i;
    aFrameDef   *fd;
    Slot        *s;

    if( !frame || !key ) return;

    /* find frame definition */
    if( !(fd= getFrameDef(frame->name, cnfg)) ) return;

    for( i=0, s= fd->slot; i < fd->n_slot; i++, s++ ){
        if( !scmp(key, s->key) ) break;
    }
    if( i >= fd->n_slot ) return;

    *( (frame->confirmed)+i )= (char) val;
}

void cp_values(aFrame *to, aFrame *from, Config *cnfg)
{
    int          i;
    aFrameDef   *fd;
    char        **f, **t;

    if( !to || !from ) return;
    if( scmp(from->name, to->name) ) return;

    /* find frame definition */
    if( !(fd= getFrameDef(from->name, cnfg)) ) return;

    f= from->value;
    t= to->value;
    for( i=0; i < fd->n_slot; i++, f++, t++ ){
        if( *f ) cp_str(t, *f);
    }

}

/* cp values by key name, frames can be different type */
void cp_key_values(aFrame *to, aFrame *from, Config *cnfg)
{
    int          i;
    aFrameDef   *fd;
    Slot        *s;
    char        **f;

    if( !to || !from ) return;

    /* find from frame definition */
    if( !(fd= getFrameDef(from->name, cnfg)) ) return;

    f= from->value;
    for( i=0, s=  fd->slot; i < fd->n_slot; f++, i++, s++ ){
        if( !*f ) continue;
        setKey(to, s->key, *f, cnfg);
    }

}

int frame_empty(aFrame *frame, Config *cnfg)
{
    int          i;
    aFrameDef   *fd;
    char        **f;

    if( !frame ) return(1);

    /* find frame definition */
    if( !(fd= getFrameDef(frame->name, cnfg)) ) return(1);

    f= frame->value;
    for( i=0; i < fd->n_slot; i++, f++ ){
        if( *f ) return(0);
    }
    return(1);
}


/* copy a frame */
void cp_frame(aFrame *to, aFrame *from, Config *cnfg)
{

    if( !from || !to ) return;

    cp_str(&(to->name), from->name);
    cp_values(to, from, cnfg);

}

int key_confirmed(aFrame *frame, char *key, Config *cnfg)
{
    int          i;
    aFrameDef   *fd;
    Slot        *s;

    if( !frame || !key ) return(0);

    /* find frame definition */
    if( !(fd= getFrameDef(frame->name, cnfg)) ) return(0);

    for( i=0, s= fd->slot; i < fd->n_slot; i++, s++ ){
        if( !scmp(key, s->key) ) break;
    }

    if( i < fd->n_slot ) return ( (int) *((frame->confirmed)+i) );
    return(0);

}

int key_consistent(aFrame *frame, char *key, Config *cnfg)
{
    int          i;
    aFrameDef   *fd;
    Slot        *s;

    if( !frame || !key ) return(0);

    /* find frame definition */
    if( !(fd= getFrameDef(frame->name, cnfg)) ) return(0);

    for( i=0, s= fd->slot; i < fd->n_slot; i++, s++ ){
        if( !scmp(key, s->key) ) break;
    }

    if( i < fd->n_slot ) return ( (int) *((frame->consistent)+i) );
    return(0);

}


int child_filled(aFrame *frame, aFrameDef *fd, int *node)
{
    char    *key;
    Slot    *s;
    int     nd;
    int     i;
    int     done;

    /* if no key value for node */
    if( !(key= (fd->slot+ *node)->key) )  return( 0 );

    /* for each child of node */
    for(nd=(*node)+1 ; nd < fd->n_slot;  ) {

    s= fd->slot + nd;
    if( scmp(key, s->key) ) return(0); /* end of kids */

    /* if already filled */
    i= nd;
    done= node_done( frame, fd, &i);
    if( done ) return(1);
        nd++;
    }
    return(0);
}

char *fill_child(aFrame *frame, aFrameDef *fd, int *node, Config* cnfg)
{
    char    *key;
    char    *p;
    Slot    *s;
    int     i;
    int     done;
    int     pre_node;
    int     cnt;

    /* if no key value for node */
    if( !(key= (fd->slot+ *node)->key) )  return( (char *)0 );

    /* for each child of node */
    for((*node)++ ; *node < fd->n_slot;  ) {

        s= fd->slot + *node;
        if( scmp(key, s->key) ) break; /* end of kids */
        /* skip node if not required */
        if( !s->required ) { skip(frame, fd, node); continue;}

        /* skip node if already filled */
        i= *node;
        done= node_done( frame, fd, &i);
        if( done ) {skip(frame, fd, node); continue;}

        /* check prompt for child */
        if( (fd->slot + *node)->prompt[0] ) {
            cnt= (int) *(frame->prompt_count + *node);
            p= (fd->slot + *node)->prompt[cnt];
            if( (cnt < (cnfg->MaxPrompt-1)) && ((fd->slot + *node)->prompt[cnt+1])) {
                (*(frame->prompt_count + *node))++;
            }
            if(p) return( p );
        }
        pre_node= *node;
        if( (p= fill_child( frame, fd, node, cnfg)) ) return(p);
        if( *node == pre_node) (*node)++;
    }
    return((char *)0);
}

