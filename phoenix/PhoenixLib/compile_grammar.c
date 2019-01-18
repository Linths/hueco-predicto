/* take input grammar and generate nets

   input: file containing grammar rules
   output: <file>.net file containing compiled grammar
	   dictionary
*/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>	  
#include <ctype.h>
#include "phoenix.h"



void compile_grammar( Config *cnfg );
void concept_leaf( Config *cnfg);
int set_config(Config *cnfg);
Gnode *new_nfa();
SucLink *new_nfa_link();
void alloc_sp ();
void net_gen ();
void rewrite();
void add_nt(int token, SucLink *arc, Config *cnfg );
int find_sym();
int get_symbol();
int get_right ();
int blank_line();
int pconf();
void pusage();
void write_net(FILE *fp_out);
void reset_compile();
int find_gnet();
int	find_word(char *s, Gram *gram);
int mk_dic(char *gram_file, char *dic_file, Config *cnfg);
int read_dic(char *dir, char *dict_file, Gram *gram, char **sb_start,
                char *sym_buf_end, Config *cnfg);


Gnode	*nfa,
	*nfa_ptr,
	*nfa_end;

SucLink *nfa_succ,
	*nfa_suc_ptr,
	*nfa_suc_end;

int	NetNumber;
char	NetName[LABEL_LEN];
char	NetSym[LABEL_LEN];

non_term *non_t;
int num_nt;

char	*sym_buf,	/* buff for ascii input strings */
	*sym_ptr,
	*sym_buf_end;
char	*tok_buf,	/* buff for ascii input strings */
	*tok_ptr,
	*tok_buf_end;
char **sym;		/* symbol table */
int num_sym;
int last_net;
int num_nets;
char	state;
int	nt_tok;

char	*wrds[MAX_WRDS];	/* pointers to strings for word numbers */
char	update_dict;

char rule_buf[LINE_LEN];
char *rule_ptr;

Gram gram;

int main( argc, argv)
    int argc;
    char *argv[];
{
    /* structure for configuration parameters */
    Config  cnfg;

    /* set command line parms */
    if( set_config(&cnfg) ) {
	fprintf(stderr, "Configuration failed\n");
	return(-1);
    }

    if( !*cnfg.grammar_file ) {
	printf("Usage: Must specify output file\n");
	exit(-1);
    }

    if( !*cnfg.grammar_rules ) {
	printf("Usage: Must specify rules file\n");
	exit(-1);
    }

    /* malloc space for data structures */
    alloc_sp (&cnfg);

    /* create new dictionary */
    mk_dic(cnfg.grammar_rules, cnfg.dict_file, &cnfg);

    /* read dic */
    read_dic(cnfg.task_dir, cnfg.dict_file, &gram, &sym_ptr,sym_buf_end, &cnfg);

    /* compile grammar networks */
    compile_grammar(&cnfg);

    /* mark concept leafs for extraction */
    concept_leaf(&cnfg);

    return(0);
}


void compile_grammar( Config *cnfg )
{
    int 	net_num,
		j;
    char	fname[LABEL_LEN],
    		rule_buf[LINE_LEN],
		*s;
    FILE	*fp_in, *fp_out;

    nfa_ptr= nfa;
    nfa_suc_ptr= nfa_succ;
    num_nt = 0;
    state= 'n';

    /* leave null symbol for net 0 */
    sym[0]= sym_ptr;
    strcpy(sym_ptr, "*");
    sym_ptr += strlen(sym_ptr)+1;
    num_sym= 1;


    /* open input grammar rules file */
    sprintf(fname, "%s/%s", cnfg->task_dir, cnfg->grammar_rules);
    if( !(fp_in = fopen(fname,"r")) ) {
	printf("ERROR: Can't open grammar file %s\n", fname);
	exit(-1);
    }

    /* read net names into symbol table */
    for(net_num=1; fgets(rule_buf, LINE_LEN, fp_in); ) {

	/* if not start of new net */
	if( rule_buf[0] != '[' ) continue;

	/* delimit end of net name */
	if( !(s= strchr(rule_buf, (int)']')) ) continue;
	s++;
	*s= (char)0;

	/* check for duplicate net names */
	for(j=1; j < num_sym; j++ ) {
	    if( !strcmp(sym[j], rule_buf) ) {
		fprintf(stderr, "Duplicate net name %s\n", rule_buf);
		exit(-1);
	    }
	}

	/* add to symbol buffer */
	sym[num_sym++]= sym_ptr;
	if( (sym_ptr + strlen(rule_buf) +1) >= sym_buf_end ) {
	    printf("Buffer overflow, SymBufSize= %d\n", cnfg->SymBufSize);
	    exit(1);
	}
	strcpy( sym_ptr, rule_buf );
	sym_ptr += strlen(rule_buf) +1;
	net_num++;
    }
    last_net= num_sym-1;
    num_nets= net_num-1;

    /* open output .net file */
    sprintf(fname, "%s/%s", cnfg->task_dir, cnfg->grammar_file);
    if( !(fp_out = fopen(fname,"w")) ) {
	printf("can't open %s\n", fname);
	exit(1);
    }
    /* write number of nets at start of file */
    fprintf(fp_out, "Number of Nets= %d\n", num_nets);

    fclose(fp_in);

    /* read grammar rules and compile networks */
    net_gen(cnfg->grammar_rules, fp_out, cnfg);

    fclose(fp_out);

    return;
}

void net_gen (char *filename, FILE *fp_out, Config *cnfg)
{
    SucLink	*arc;
    Gnode	*start_node, *end_node;
    int		s_idx;
    int		nt;
    char	*s;
    char	fname[LABEL_LEN];
    char	name[LABEL_LEN];
    FILE	*fp_in;
  
    /* open input grammar rules file */
    sprintf(fname, "%s/%s", cnfg->task_dir, filename);
    if( !(fp_in = fopen(fname,"r")) ) {
	printf("ERROR: Can't open grammar file %s\n", fname);
	exit(-1);
    }


    /* generate non-deterministic FSA by re-writing non-terminals */

    /* read each line from gra file */
    while( fgets(rule_buf, LINE_LEN, fp_in) ) {

	if( blank_line(rule_buf) ) continue;

	/* include file */
	if( !strncmp(rule_buf, "#incl", 5) ) {
	    sscanf(rule_buf, "#include %s", name);
	    sprintf(fname, "%s/%s", cnfg->task_dir, name);
	    net_gen(fname, fp_out, cnfg);
	    continue;
	}

	/* comment */
	else if( *rule_buf == '#' ) continue;

	/* net definition end */
	else if( *rule_buf == ';' ) {
	    if( state != 'n' ) {
		/* write out compiled net */
		write_net(fp_out);
		/* reset structures */
		reset_compile();
		state= 'n';
	    }
	    continue;
	}

	/* net name, begin net definition */
	else if( *rule_buf == '[' ) {

	    if( state != 'n' ) {
		printf("ERROR: Unexpected net definition %s\n", rule_buf);
		continue;
	    }

	    /* set net number */
	    sscanf(rule_buf, "%s", NetName);
	    NetNumber= find_gnet( NetName );

	    /* replace square brackets with angle brackets */
	    s= strchr(rule_buf, '[');
	    if(s) *s= '<';
	    s= strchr(rule_buf, ']');
	    if(s) *s= '>';
	    sscanf(rule_buf, "%s", NetSym);
	    s_idx= find_sym( NetSym, cnfg );

	    /* create start and end nodes */
	    start_node= new_nfa();
	    start_node->n_suc= 1;
	    start_node->final= 0;
	    end_node= new_nfa();
	    end_node->n_suc= 0;
	    end_node->final= 1;
    
	    /* arc between labelled with net name */
	    arc= new_nfa_link();
	    arc->succ.tok= s_idx;
	    arc->succ.state= end_node;
	    arc->succ.call_net= 0;
	    arc->link= 0;
	    arc->nt= 1;
	    start_node->succ= (struct gsucc *) arc;

	    add_nt( s_idx, arc, cnfg );
	    rule_ptr= rule_buf;
	    nt_tok= get_symbol(cnfg);
	    state= 'p';
	    continue;
	}

	/* rewrite rule */
	else if( isspace( (int) *rule_buf) ) {
            if( !strchr(rule_buf, (int) '(' ) ||
                !strchr(rule_buf, (int) ')' ) ) {
		printf("ERROR: Bad format for rule: %s\n", rule_buf);
		continue;
	    }
    	    /* rewrite each previous occurence of symbol with rhs */
    	    for( nt= 0; nt < num_nt; nt++) {
      		if( non_t[nt].tok == nt_tok ) {
        	    rewrite( non_t[nt].arc, cnfg );
        	    /* mark as rewritten */
        	    non_t[nt].rw++;
      		}
    	    }
	}

	/* macro definition */
	else if( isupper( (int) *rule_buf) ) {
	    rule_ptr= rule_buf;
	    if( (nt_tok= get_symbol(cnfg)) < 0) continue;
	    state= 'e';
	}

	/* error */
	else {
	    printf("ERROR: bad format: %s\n", rule_buf);
	}

    }

    fclose(fp_in);
}

void rewrite( SucLink *arc, Config *cnfg)
{
    int tok, tok1;
    SucLink	*alt,
		*new_arc,
		*link,
		*start;

    Gnode	*state,
		*from_state;
    char opt, opt1,
	 self, self1,
	 type, type1,
	 prev_self;

    /* set rule_ptr to start of right hand side */
    if( !(rule_ptr= strchr(rule_buf, (int) '(' )) ) return;
    rule_ptr++;
    from_state= 0;
    start= 0;
    prev_self=1;


    if( (tok= get_right(&opt, &self, &type, cnfg)) < 0 ) return;

    /* while more tokens remaining */
    while( (tok1= get_right(&opt1, &self1, &type1, cnfg)) >= 0 ) {

	/* create arc for token */
	alt= new_nfa_link();
	alt->succ.tok= tok;
	alt->succ.call_net= 0;
	alt->nt= 0;
	alt->link= 0;

        if( type == 1) {
	    /* if non_term token */
	    alt->nt= 1;
	    add_nt( tok, alt, cnfg );
        }
        else if( type == 2 ) {
	    /* if pre_term token */
	    alt->succ.call_net= tok;
        }

	if( self && opt ) {
	    /* if start or prev state contained self trans add null arc */
	    if ( !from_state || prev_self ) {
		/* create next state */
		state= new_nfa();

		/* add null arc to state */
	        new_arc= new_nfa_link();
	        new_arc->succ.tok= 0;
	        new_arc->succ.call_net= 0;
                new_arc->succ.state= state;
	        new_arc->nt= 0;
		if( from_state ) {
	    	    new_arc->link= (SucLink *) from_state->succ;
	    	    from_state->succ= (struct gsucc *) new_arc;
		}
	        /* if first arc from start node */
	        if( !start ) start= new_arc;

		/* add self transition */
		alt->succ.state= state;
	        alt->link= (SucLink *) state->succ;
	        state->succ= (struct gsucc *) alt;

	    }
	    else {
		/* add self transition */
		alt->succ.state= from_state;
	        alt->link= (SucLink *) from_state->succ;
	        from_state->succ= (struct gsucc *) alt;
	    }
	}
	else {
	    /* if first arc from start node */
	    if( !start ) start= alt;

	    /* create next state */
	    state= new_nfa();
	    /* point arc at state */
	    alt->succ.state= state;

	    /* if not start of rule, link in to state */
	    if( from_state ) {
	        alt->link= (SucLink *) from_state->succ;
	        from_state->succ= (struct gsucc *) alt;
	    }

	    if( opt ) {
	        /* add null arc to next state */
	        new_arc= new_nfa_link();
	        new_arc->succ.tok= 0;
	        new_arc->succ.call_net= 0;
                new_arc->succ.state= state;
	        new_arc->nt= 0;
	        new_arc->link= alt->link;
	        alt->link= new_arc;
	    }
            if( self ) {
	        /* add self-transition */
	        new_arc= new_nfa_link();
		*new_arc= *alt;
        	if( type == 1) {
	    	    /* if non_term token */
	    	    add_nt( tok, new_arc, cnfg );
        	}
	        new_arc->link= (SucLink *) state->succ;
	        state->succ= (struct gsucc *) new_arc;
	    }
	}
	from_state= state;
	prev_self= self;
	tok= tok1;
	opt= opt1;
	self= self1;
	type= type1;
	
    }

    /* create ending arc */
    alt= new_nfa_link();
    if( !start ) start= alt;
    alt->succ.tok= tok;
    alt->succ.call_net= 0;
    alt->succ.state= arc->succ.state;
    alt->nt= 0;
    alt->link= 0;

    if( type == 1) {
	/* if non_term token */
	alt->nt= 1;
	add_nt( tok, alt, cnfg );
    }
    else if( type == 2 ) {
	/* if pre_term token */
	alt->succ.call_net= tok;
    }

    if( self && opt ) {
	/* if prev state contained self transition add null arc */
	if ( !from_state || prev_self ) {
	    /* create next state */
	    state= new_nfa();

	    /* add null arc to state */
	    new_arc= new_nfa_link();
	    new_arc->succ.tok= 0;
	    new_arc->succ.call_net= 0;
            new_arc->succ.state= state;
	    new_arc->nt= 0;
	    if( from_state ) {
	    	new_arc->link= (SucLink *) from_state->succ;
	    	from_state->succ= (struct gsucc *) new_arc;
	    }

	    /* add self transition */
	    alt->succ.state= state;
	    alt->link= (SucLink *) state->succ;
	    state->succ= (struct gsucc *) alt;
	}
	else {
	    /* add self transition */
	    alt->succ.state= from_state;
	    alt->link= (SucLink *) from_state->succ;
	    from_state->succ= (struct gsucc *) alt;
	    state= from_state;
	}

	/* create null arc to final state */
	new_arc= new_nfa_link();
	new_arc->succ.tok= 0;
    	new_arc->succ.state= arc->succ.state;
	new_arc->succ.call_net= 0;
	new_arc->nt= 0;
	new_arc->link= (SucLink *) state->succ;
	state->succ= (struct gsucc *) new_arc;
    }
    else {
	/* if not start of rule, link in to state */
	if( from_state ) {
	    alt->link= (SucLink *) from_state->succ;
	    from_state->succ= (struct gsucc *) alt;
	}

        if( self) {
	    /* create next state */
	    state= new_nfa();
	    alt->succ.state= state;

	    /* add self transition */
	    new_arc= new_nfa_link();
	    *new_arc= *alt;
    	    if( type == 1) add_nt( tok, new_arc, cnfg );
	    new_arc->link= (SucLink *) state->succ;
	    state->succ= (struct gsucc *) new_arc;

	    /* create null arc to final state */
	    new_arc= new_nfa_link();
	    new_arc->succ.tok= 0;
    	    new_arc->succ.state= arc->succ.state;
	    new_arc->succ.call_net= 0;
	    new_arc->nt= 0;
	    new_arc->link= (SucLink *) state->succ;
	    state->succ= (struct gsucc *) new_arc;
	}
	else {
	    alt->succ.state= arc->succ.state;
	    if( opt ) {
		/* if optional, add null arc */
		new_arc= new_nfa_link();
		new_arc->succ.tok= 0;
		new_arc->succ.call_net= 0;
		new_arc->succ.state= alt->succ.state;
		new_arc->nt= 0;
		new_arc->link= alt->link;
		alt->link= new_arc;
	    }
	}
    }

    /* link in start arc */
    if( !arc->link ) {
	arc->link= start;
    }
    else {
	for( link= arc->link; link->link; link= link->link) ;
	link->link= start;
    }
    
}


void alloc_sp (Config *cnfg)
{
  /* allocate space for symbol buffer */
  if( !(sym_buf= (char *) malloc(cnfg->SymBufSize)) ) {
    printf("not enough memory to allocate symbol buffer\n");
    exit(1);
  }
  sym_buf_end= sym_buf + cnfg->SymBufSize;
  sym_ptr= sym_buf;

  /* allocate space for non-term symbol buffer */
  if( !(tok_buf= (char *) malloc(cnfg->TokBufSize)) ) {
    printf("not enough memory to allocate symbol buffer\n");
    exit(1);
  }
  tok_buf_end= tok_buf + cnfg->TokBufSize;
  tok_ptr= tok_buf;

  /* allocate space for non-term array */
  if( !(non_t= (non_term *) calloc(cnfg->MaxNonTerm, sizeof(non_term))) ) {
    printf("not enough memory to allocate non-term array\n");
    exit(1);
  }

  /* allocate space for symbol table */
  if( !(sym= (char **) calloc(cnfg->MaxSymbol, sizeof(char *))) ) {
    printf("not enough memory to allocate non-term array\n");
    exit(1);
  }

  /* allocate space for nfa */
  if( !(nfa= (Gnode *) calloc(cnfg->MaxNfa, sizeof(Gnode))) ) {
    printf("not enough memory to allocate nfa\n");
    exit(1);
  }
  nfa_end= nfa + cnfg->MaxNfa;
  if( !(nfa_succ= (SucLink *) malloc(cnfg->MaxSucLink * sizeof(SucLink))) ) {
    printf("not enough memory to allocate nfa_succ\n");
    exit(1);
  }
  nfa_suc_end= nfa_succ + cnfg->MaxSucLink;
}


int get_right(char *opt, char *self, char *type, Config *cnfg)
{
    char symbol[LABEL_LEN];
    char *sym_ptr = symbol;
    char *sym_end = symbol+LABEL_LEN;
    int w_idx;

    /* skip white space */
    if( !*rule_ptr ) return(-1);
    while( *rule_ptr &&  isspace( (int) *rule_ptr ) ) rule_ptr++;
    if( *rule_ptr == ')' ) return(-1);

    /* check optional and self-transition flags */
    *opt= 0;
    *self= 0;
    for( ; 1; rule_ptr++ ) {
        if( *rule_ptr == '*' ) *opt= 1;
        else if( *rule_ptr == '+' ) *self= 1;
	else break;
    }

    /* copy symbol */
    while(!isspace((int)*rule_ptr) && (sym_ptr< sym_end) && (*rule_ptr != ')'))
	*sym_ptr++ = *rule_ptr++;
    if( sym_ptr >= sym_end ) {
	printf("ERROR: symbol too long on right side.\n");
	return(-1);
    }
    *sym_ptr= '\0';

    if( *symbol == '[' ) {
	/* net call */
	int	symi;
 
	*type= 2;
	symi= find_sym(symbol, cnfg);
	if( symi == (num_sym-1) ) {
	    printf("WARNING: compiling net %s could not find called net %s\n",
			 NetName, symbol);
	}
	return( symi );
    }
    else if( isupper((int) *symbol) ) {
	/* non-terminal */
	*type= 1;
	return( find_sym( symbol, cnfg ) );
    }
    else {
	/* word */
	*type= 0;
	w_idx= find_word(symbol, &gram);
	return( w_idx );
    }
}


int find_sym( char *s, Config *cnfg )
{
    int i;

    /* if null arc */
    if( !strcmp( s, sym[0] ) )  return(0);

    for(i=num_sym-1; i >= 0; i-- )
	if( !strcmp( s, sym[i] ) ) break;
    if( i >=  0 ) return(i);

    /* install new symbol */
    if( num_sym >= cnfg->MaxSymbol ) {
	printf("Table overflow, MaxSymbol= %d\n", cnfg->MaxSymbol);
	exit(1);
    }
    if( (tok_ptr + strlen(s)+1 ) >= tok_buf_end ) {
	printf("Buffer overflow, TokBufSize= %d\n", cnfg->TokBufSize);
	exit(1);
    }

    strcpy(tok_ptr, s);
    sym[num_sym]= tok_ptr;
    tok_ptr += strlen(s)+1;
    return( num_sym++);
}

Gnode *new_nfa(Config *cnfg)
{
    if( nfa_ptr >= nfa_end ) {
	printf("Table overflow, MaxNonTerm= %d\n", cnfg->MaxNonTerm);
	exit(1);
    }

    nfa_ptr->n_suc = 0;
    nfa_ptr->final = 0;
    nfa_ptr->succ = NULL;

    return( nfa_ptr++);
}

SucLink *new_nfa_link(Config *cnfg)
{
    if( nfa_suc_ptr >= nfa_suc_end ) {
	printf("Table overflow, MaxSucLink= %d\n", cnfg->MaxSucLink);
	exit(1);
    }

    nfa_suc_ptr->succ.tok = 0;
    nfa_suc_ptr->succ.state = NULL;
    nfa_suc_ptr->succ.call_net = 0;
    nfa_suc_ptr->link = NULL;
    nfa_suc_ptr->nt = 0;

    return( nfa_suc_ptr++);
}



void add_nt(int token, SucLink *arc, Config *cnfg )
{

    if( num_nt >= cnfg->MaxNonTerm ) {
	printf("Table overflow, MaxNonTerm= %d\n", cnfg->MaxNonTerm);
	exit(1);
    }
    non_t[num_nt].tok= token;
    non_t[num_nt].rw= 0;
    non_t[num_nt++].arc= arc;
}


void write_net(FILE *fp_out)
{
    Gnode	*state;
    SucLink	*arc;
    int		count,
    		tot_arcs;
    int	i;

#ifdef DEBUG
  {
    Gnode *state;
    SucLink *arc;

    printf("Symbol Table\n");
    for(i=0; i < num_sym; i++ ) printf("%s\n", sym[i]);
    printf("\n\n");

    printf("NFA Network\n");
    for(state= nfa, i=0; state < nfa_ptr; state++, i++) {
      printf("node %d  %d %d\n", i, state->n_suc, state->final);
      for( arc= (SucLink *) state->succ; arc; arc= arc->link ) {
        printf("\t\t%d    %d    %d\n", arc->succ.tok,
               arc->succ.call_net, arc->succ.state - nfa);
      }
    }
  }
#endif

    /* check to see all nt rewritten */
    for(i=0; i < num_nt; i++) {
    	if( !non_t[i].rw ) {
      	    printf("WARNING: In <%s> NonTerm %s not rewritten\n",
                 NetName, sym[ non_t[i].tok ]);
    	}
    }

    /* write net name, net number, number of states and concept_flag */
    fprintf(fp_out, "%s %d %d %d\n", 
		NetName, NetNumber, (int)(nfa_ptr - nfa), 0);

    /* generate final net by removing non-terminal arcs */
    tot_arcs= 0;
    for(state= nfa, i=0; state < nfa_ptr; state++, i++) {
      /* count succs */
      for(arc= (SucLink *)state->succ,count=0; arc; arc= arc->link) {
        if( !arc->nt ) count++;
      }
      fprintf(fp_out, "%d  %d %d\n", i, count, state->final);
      for( arc= (SucLink *) state->succ; arc; arc= arc->link ) {
        if( arc->nt ) continue;
        fprintf(fp_out, "\t\t%d    %d    %d\n", arc->succ.tok,
                arc->succ.call_net, (int)(arc->succ.state - nfa));
      }
      tot_arcs += count;
    }
    printf("%s  %d states  %d arcs\n", NetName, (int)(nfa_ptr - nfa), tot_arcs);

}

void reset_compile()
{
    num_sym= last_net +1;
    tok_ptr= tok_buf;
    nfa_ptr= nfa;
    nfa_suc_ptr= nfa_succ;
    num_nt = 0;
}


int find_gnet( s )
    char *s;
{
    int i;

    for(i=1; i <= last_net; i++ )
	if( !strcmp( s, sym[i] ) ) break;
    if( i <=  last_net ) return(i);

    printf("WARNING: net %s not found\n", s);
    fflush(stdout);
    return(-1);
}




int get_symbol(Config *cnfg)
{
    static int last_tok = 0;
    char symbol[LABEL_LEN];
    char *sym_ptr = symbol;
    char *sym_end = symbol+LABEL_LEN;
    char *ptr;

    while( isspace((int) *rule_ptr) ) rule_ptr++;
    if( !*rule_ptr || (*rule_ptr == '\n')) return(-1);

    ptr= rule_ptr;
    while( !isspace( (int) *rule_ptr ) && (sym_ptr < sym_end) )
	*sym_ptr++ = *rule_ptr++;
    if( sym_ptr >= sym_end ) {
	printf("ERROR: symbol too long: %s.\n", ptr);
	return(-1);
    }
    *sym_ptr= '\0';

    last_tok= find_sym(symbol, cnfg);
    return( last_tok );
}

int blank_line(char *s)
{
    while( *s && isspace( (int) *s) ) s++;
    if( !*s ) return(1);
    else return(0);
}

