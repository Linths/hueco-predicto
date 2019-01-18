/*-----------------------------------------------------------------------*
		Parser
   parse.c
   parse(text, gram, cnfg)
      Take word string as argument
      Generate lattice of parses in structure "parses"

   The general parsing algorithm is:

   for each word in input
      for each frame element
         match_net() - match element starting at word
                       adds edges to the chart as they are matched

         add_to_tmp() - add matched elements to active templates

         add_to_path() - adds completed templates to seq_end[] array

   The above process creates the chart and the seq_end[] array which
   contains backpointer chains for template sequences that end with
   each word of the input.

   Then find best scoring template sequence in seq_end[] and prune
   all sequences that have a worse score.

    expand_seq() is then called for each sequence remaining in seq_end[]
    to generate a flat sequence from the backpointers.

    map_to_frames() is then called for each sequence to add frame labels


*-----------------------------------------------------------------------*/

#ifdef WIN
#include "stdafx.h"
#include <stdlib.h>
#endif

#ifdef OSX
#include <stdlib.h>	   
#else
#include <malloc.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "phoenix.h"


#define NO_SCORE 0xFFFF
#define MAX_HYP	500

/* functions defined here */
int add_to_path(Tmp *tp, Gram *gram, Config *cnfg);
int add_to_tmp(int net, int start_word, Gram *gram, Config *cnfg);
void	expand_seq(SeqNode *seq, int ns, int *num_seqs, Config *cnfg);
void	extend_map(SeqNode **seq, int i, Gram *gram, Config *cnfg);
SeqNode	*get_new_node(Config *cnfg);
int	init_parse(Config *cnfg);
void	map_to_frames(SeqNode **seq_start, int num_seqs, Gram *gram,
		Config *cnfg);
int	tmp_in_frame(int tid, int frame, Gram *gram);
Tmp	*new_tmp();
void	parse(char *line, Gram *gram, Config *cnfg);
int print_parse(int parse_num, char *out_str, char *buf_end, char extract,
		Gram *gram, Config *cnfg);
void	read_line(char *line, Gram *gram, Config *cnfg );
void	reset_parse(Config *cnfg);

/*** functions called ***/
int	check_chart(int net, int word_pos, Edge **edge);
void	breakpt_reset(int num_nets);
int	find_word(char *s, Gram *gram);
int	init_match(Config *cnfg);
void	init_print_extracts();
int	match_net(int net, int word_pos, Gram *gram, Config *cnfg);
char	*print_edge(Edge *edge, Gram *gram, char *s, Config *cnfg);
char	*print_extracts(Edge *edge, Gram *gram, char *s, int level,
		char *fn, Config *cnfg);
Gram	*read_grammar(Config *cnfg);

/*** buffers used in parse ***/
static	Fid	*fid_buf,		/* buffer for frame path tree */
		*fid_buf_ptr,
		*fid_buf_end;
static	SeqNode	**pbuf,			/* lattice of final parses */
		**pbuf_end;
static	SeqCell	*seq_end;
static	SeqNode	*seq_buf,		/* buffer for sequence path tree */
		*seq_buf_ptr,
		*seq_buf_end;

static int	fr_seq[100];
static SeqNode	*sl_seq[100];
static SeqNode	*parses[MAX_HYP];
static int	frames_in_parse;

char	*outbuf,
	*outbuf_end,
	*action_seq,
	*response;
TmpDef tmps[MAX_TMP];
int num_tmps;
static Tmp *act_tmps[MAX_TMP];

/* parse function */
void parse(char *line, Gram *gram, Config *cnfg)
{

    int		word_pos,
		fe_num,
		n_frames,
		result,
    		i, j,
		fn, sn,
		num_seqs,
		best_wp;
    SeqNode	*best_seq,
		*sq;
    int		tid;

 
    /* if cur_nets not set, create from frames in grammar */
    if( !cnfg->num_active || !cnfg->cur_nets ) {

    	/* for each frame */
    	for(fn= 0; fn < gram->num_frames; fn++) {
	    /* for each template in frame */
	    for(sn= 0; sn < gram->frame_def[fn].n_slot; sn++) {
		tid= (int)gram->frame_def[fn].slot[sn];
		/* for each element in template */
		for(i=0; i < tmps[tid].len; i++) { 
    	    	    /* see if already in list */
	    	    for(j=0; j < cnfg->num_active; j++) {
			if( tmps[tid].el[i] == cnfg->active_slots[j] ) break;
	    	    }
    	    	    if( j < cnfg->num_active ) continue;
    	    	    cnfg->active_slots[cnfg->num_active++]= tmps[tid].el[i];
	        }
	    }
    	}
        cnfg->cur_nets= cnfg->active_slots;
    }

    /* convert word strings to numbers and put in script array */
    read_line( line, gram, cnfg );

    /* for each word position in input try to match each element */
    for( word_pos= 1; word_pos < cnfg->script_len; word_pos++) {
	for(fe_num= 0; fe_num < cnfg->num_active; fe_num++) {

	    /*	match element fe_num starting at word_pos
		put matches and attempted matches as edges in chart
		return 0 if ok, -1 if error
	    */
	    result=match_net(cnfg->cur_nets[fe_num], word_pos, gram, cnfg);
	    if( result < 0 ) return;

	    /*	add matched frame element to active templates
		completed templates are added to seq_end
	    */
	    result= add_to_tmp(cnfg->cur_nets[fe_num], word_pos, gram, cnfg);
	    if( result < 0) return;

	}
    }

    /* find best scoring seq end */
    best_wp= 0;
    for( word_pos= 0; word_pos < cnfg->script_len; word_pos++) {

	/* compare score */
	if( seq_end[word_pos].score < seq_end[best_wp].score ) continue;
	if( seq_end[word_pos].score > seq_end[best_wp].score ) {
		best_wp= word_pos;
		continue;
	}

	/* compare number of slots */
	if( seq_end[word_pos].n_slots > seq_end[best_wp].n_slots ) continue;
	if( seq_end[word_pos].n_slots < seq_end[best_wp].n_slots ) {
		best_wp= word_pos;
		continue;
	}

	/* compare number of frames */
	if( seq_end[word_pos].n_frames > seq_end[best_wp].n_frames ) continue;
	if( seq_end[word_pos].n_frames < seq_end[best_wp].n_frames ) {
		best_wp= word_pos;
		continue;
	}
    }
    if( !best_wp ) {
	num_seqs= 0;
	return;
    }

    /* print number of paths ending at each word position */
    if( cnfg->VERBOSE > 5 ) {
	int count;
	SeqNode *sn;
	int max, max_idx;

	max= max_idx=0;
        printf("number of paths ending at word position\n");
        for( word_pos= 0; word_pos < cnfg->script_len; word_pos++) {
		count= 0;
		for( sn= seq_end[word_pos].link; sn; sn= sn->link) count++;
		printf(" %d", count);
		if (count > max) {
			max= count;
			max_idx= word_pos;
		}
        }
        printf("\n");
        printf("best_wp= %d\n", best_wp);
    }

    cnfg->n_slots= seq_end[best_wp].n_slots;

    /* find least fragmented seq */
    best_seq= 0;
    num_seqs= 0;
    n_frames= seq_end[best_wp].n_frames;

    /* for each sequence */
    for( sq= seq_end[best_wp].link; sq; sq= sq->link) {
	Fid *fid_ptr;
	int	count;

	/* delete more fragmented frame ids */
	count= sq->n_act;
	for(fid_ptr= sq->frame_id, j=0; j < sq->n_act; fid_ptr++, j++) {
	    if( fid_ptr->count == n_frames ) continue;
	    fid_ptr->count= 0;
	    count--;
	}
	if( count < 1 ) {
	    /* delete sequence */
	    sq->n_act= 0;
	    continue;
	}

	if( !best_seq ) best_seq= sq;
    }

    /*	No parse */
    if( !best_seq ) return;

    /* generate template sequences from back pointers in seq_end array
       put sequences in pbuf
    */
    num_seqs= 0;
    /* generate only one sequence */
    if( (cnfg->ALL_PARSES) == 'n' ) {
	expand_seq(best_seq, cnfg->n_slots, &num_seqs, cnfg);
    }
    /* generate all sequences */
    else {
	/* for each sequence */
	for( sq= seq_end[best_wp].link; sq; sq= sq->link) {
	   if( !sq->n_act ) continue;
    	   expand_seq(sq, cnfg->n_slots, &num_seqs, cnfg);
    	}
    }

    /* add frame ids to template sequences */
    map_to_frames(pbuf, num_seqs, gram, cnfg);

    return;
}


/* generate template sequence from back pointers in seq_end array
   append sequence to entries in pbuf
*/
void expand_seq(SeqNode *seq, int ns, int *num_seqs, Config *cnfg)
{
    SeqNode *sn, **pn;
    int	col;

    /* add new seq to set of parses */
    pn= pbuf  + ((*num_seqs)*ns) +ns-1;
    if( pn >= pbuf_end ) {
	fprintf(stderr, "ERROR: overflow ParseBufSize %d\n",cnfg->ParseBufSize);
	return;
    }
    for( col= ns-1, sn= seq; col >= 0; col--, sn= sn->back_link) {
	    *pn-- = sn;
    }
    (*num_seqs)++;
}

/* add frame labels to template seq, creating alts when ambiguous
   malloc space for seqs and put pointers in parses[] array
*/
void map_to_frames(SeqNode **seq_start, int num_seqs, Gram *gram, Config *cnfg)
{
    int		i, k, p;
    SeqNode 	**seq;
    SeqNode	*sn;

    /* print template sequences */
    if( cnfg->VERBOSE > 3 ) {
	printf("num_seqs= %d\n", num_seqs); fflush(stdout);
	for(p=0; p <num_seqs; p++) {
	    printf("seq %d\n", p); fflush(stdout);
	    seq= pbuf + (cnfg->n_slots * p);
    	    for(i=0;i< cnfg->n_slots; i++, seq++) {
	        sn= *seq;
	        for(k=0; k < tmps[sn->tmp->id].len; k++) {
	            printf("\t%s ", gram->labels[tmps[sn->tmp->id].el[k]]);
	        }
	    }
	    printf("\n");
	}
	printf("\n");
    }

    cnfg->num_parses= 0;
    frames_in_parse= 1000;
    /* for each template sequence */
    for(p=0; p <num_seqs; p++) {
	seq= pbuf + (cnfg->n_slots * p);

	/* initial frame label array */
    	for(i=0;i< cnfg->n_slots; i++) {
	    fr_seq[i]= -1; 
	    sl_seq[i]= (SeqNode *)0;
	}
    	i= 0;
	/* generate all frame labelings for seq */
    	extend_map(seq, i, gram, cnfg);
    }

}

/* generate all frame labelings for template seq i creating alts when ambiguous
   malloc space for seqs and put pointers in parses[] array
*/
void extend_map(SeqNode **seq, int i, Gram *gram, Config *cnfg)
{
    int j, nfr;
    SeqNode *s;

    if( cnfg->num_parses >= MAX_HYP ) return;

    /* if at end of sequence */
    if( i >= cnfg->n_slots ) {
	/* count number of frames used in parse of seq */
        for(i=0, nfr=0; i< cnfg->n_slots; i++) {
	    if( (i == 0) || (fr_seq[i] != fr_seq[i-1]) ) nfr++;
	}

	/* prune more fragmented parses */
	if( nfr < frames_in_parse ) {
	    frames_in_parse= nfr;
	    /* delete existing parses */
            for(i=0; i< cnfg->num_parses; i++) {
		free(parses[i]);
	    }
	    cnfg->num_parses= 0;
	}
	/* if more fragmented, don't save */
	else if( nfr > frames_in_parse ) return;

	/* malloc space and copy parse */
        if( !(parses[cnfg->num_parses]=
		(SeqNode *)malloc(cnfg->n_slots*sizeof(SeqNode)))){
	    fprintf(stderr, "malloc parse failed\n");
	    return;
        }
        for(i=0, s=parses[cnfg->num_parses]; i< cnfg->n_slots; i++, s++) {
	    *s= *sl_seq[i];
	    s->n_act= fr_seq[i];
        }
        cnfg->num_parses++;
        return;
    }

    /* if not at end of seq, recursively assign all possible labels
       to each template */
    sl_seq[i]= *seq;
    for( j=0; j < gram->num_frames; j++ ) {
	if( !tmp_in_frame((*seq)->tmp->id, j, gram) ) continue;
	fr_seq[i]= j;
	extend_map(seq+1, i+1, gram, cnfg);
    }
   
}


/* return 1 if template is included in frame, 0 otherwise */
int tmp_in_frame(int id, int frame, Gram *gram)
{
    int	j;

    /* see if template applies to frame */
    for(j=0; j < gram->frame_def[frame].n_slot; j++) {
	if( id == gram->frame_def[frame].slot[j] ) break;
    }
    /* if template doesn't apply */
    if( j == gram->frame_def[frame].n_slot) return(0);
    return(1);
}


/* return pointer to new SeqNode */
SeqNode *get_new_node(Config *cnfg)
{
    if( seq_buf_ptr == seq_buf_end ) {
	fprintf(stderr, "ERROR: overflow SeqBufSize  %d\n", cnfg->SeqBufSize);
	return((SeqNode *)0);
    }
    return(seq_buf_ptr++);
}


/* add frame element to active templates
   add completed templates to seq_end[] array
*/
int add_to_tmp(int el, int start_word, Gram *gram, Config *cnfg)
{
    int ew,
	i, j, t;
    Edge *edge;
    Tmp *tp, *ntp;

    /* find pointer for edges for el, sw */
    if( check_chart(el, start_word, &edge) <= 0 ) return(0);
    if( !edge ) return(0);

    /* add all versions (different end points) for el */
    ew= -1;
    for( ; edge; edge= edge->link) {
    	/* don't add alternate parses for same start end point */
	if(ew == edge->ew) continue;

	/* add to each active tmp where it matches next open element */
	for(t= 0; t < num_tmps; t++) {
	    /* check if el a part of template */
	    for(i=0; i < tmps[t].len; i++) if( el == tmps[t].el[i] ) break; 
	    if( i == tmps[t].len ) continue;

	    /* add new active template */
	    if( i == 0 ) {
	    	if( el != tmps[t].el[0] ) continue; 

		ntp= new_tmp();
		ntp->id= t;
		ntp->len= tmps[t].len;
		ntp->n= 1;
		ntp->tmp[0]= edge;

		/* if not complete, link in at head */
		if( ntp->n < ntp->len ) {
		    ntp->link= act_tmps[t];
		    act_tmps[t]= ntp;
		}
		/* if complete, add to sequence lattice */
		else {
#ifdef debug
		    printf("template found\n"); fflush(stdout);
		    for(i=0; i < ntp->len; i++)
			print_edge(ntp->tmp[i], gram, (char *)0, cnfg);
		    printf("\n");
#endif
		    /* add template to seq_end[] array */
		    if( add_to_path(ntp, gram, cnfg) < 0) return(-1);
		}
	    }
	    /* see if extends active template */
	    else {
		for(tp= act_tmps[t]; tp; tp= tp->link) {
		    if( tp->n < i ) continue;

		    /* greate new instance */
		    ntp= new_tmp();
		    ntp->id= t;
		    ntp->len= tmps[t].len;
		    ntp->n= i+1;
		    for(j=0; j < i; j++) ntp->tmp[j]= tp->tmp[j];
		    ntp->tmp[i]= edge;

		    /* if not complete, link in */
		    if( ntp->n < ntp->len ) {
		        ntp->link= tp->link;
		        tp->link=  ntp;
			break;
		    }
		    /* if complete, add to sequence lattice */
		    else {
#ifdef debug
			printf("template found\n"); fflush(stdout);
			for(i=0; i < ntp->len; i++)
			    print_edge(ntp->tmp[i], gram, (char *)0, cnfg);
			printf("\n");
#endif
			/* add template to seq_end[] array */
			if( add_to_path(ntp, gram, cnfg) < 0) return(-1);
			break;
		    }
		}
	    }
	}

	ew= edge->ew;
    }
    return(0);
}

/* add template to seq_end[] array of template sequences */
int add_to_path(Tmp *tp, Gram *gram, Config *cnfg)
{
    int phrase_score;
    unsigned short new_score,
		ns,
		sw, ew;
    SeqNode *new_slt, *link;
    SeqNode *sn;
    Fid new_frame[100],
    	*id;
    int  new_count;
    int nf, prev_nf,
	num_extended,
	wp,
	i, j;
    char	prune;
    int		first;
    Tmp	*newtmp;

    /* start word of template */
    sw= tp->tmp[0]->sw;
    ew= tp->tmp[tp->len-1]->ew;

    /* find closest end_word position to abut to */
    for(wp= sw-1; wp && (!seq_end[wp].link); wp--);

    /* compute score for extending path with template */
    phrase_score=0;
    for(i=0; i < tp->len; i++) {
	phrase_score += tp->tmp[i]->ew - tp->tmp[i]->sw +1;
    }
    new_score =  (unsigned short) phrase_score;
    if (wp) {
        new_score +=  seq_end[wp].score;
        ns= seq_end[wp].n_slots +1;
    }
    else ns= 1;
    
    prune= 0;

    /* if no sequences already end at word_pos, just add it */
    if( !seq_end[ew].link ) prune= 1;

    /* compare score of words accounted for to existing paths */
    if( new_score < seq_end[ew].score ) return(0);
    if( new_score > seq_end[ew].score ) {
	/* prune old phrases ending at word_pos */
	prune= 1;
    }

    if( !prune ) {
	/* same score, check number of slots */
	if( ns > seq_end[ew].n_slots ) return(0);
	if( ns < seq_end[ew].n_slots ) {
		/* prune old phrases ending at word_pos */
		prune= 1;
	}
    }

    /* set number of frames for frame fragmentation */
    if( seq_end[ew].link ) nf= seq_end[ew].n_frames;
    else if( wp ) nf= seq_end[wp].n_frames +1;
    else nf= 1;

    if( wp ) prev_nf= seq_end[wp].n_frames;
    else prev_nf= 0;

    /* if new path better than current, prune current */
    if(prune) {
	if( seq_end[ew].link )
	    for(sn= seq_end[ew].link; sn; sn= sn->link) free(sn->tmp);
	seq_end[ew].link= (SeqNode *) 0;
        seq_end[ew].score= new_score;
        seq_end[ew].n_slots= ns;
    	if( wp ) nf= seq_end[wp].n_frames +1;
    	else nf= 1;
    }


    /* if partial paths being extended */
    if (wp ) {
      int	min_f;

	/* for each path being extended */
	first= 1;
	for(link= seq_end[wp].link; link; link=link->link) {
	    new_count= 0;
	    min_f= 1000;
	    /* try to extend each active frame state of the path */
	    for( id= link->frame_id, j=0; j < link->n_act; j++, id++) {
	        if( id->id == gram->num_frames ) continue;
	        if( id->count < min_f) min_f= id->count;
	        if( id->count > (nf+1) ) continue;
	        if( !tmp_in_frame(tp->id, id->id, gram) ) continue;

	        new_frame[new_count].count= id->count;
	        new_frame[new_count++].id= id->id;

	        if( id->count < nf ) nf= id->count;
	    }

	    /* add frames (containing slot) that were not active */
	    if( (min_f +1) <= (nf+1) ) {
	        num_extended= new_count;
	        for(i=0; i < gram->num_frames; i++ ) {
		    if( !tmp_in_frame(tp->id, i, gram) ) continue;
		    /* don't add if extended */
		    for(j=0; j < num_extended; j++) 
			    {if( new_frame[j].id == i ) break; }
		    if( j < num_extended ) continue;
    
		    new_frame[new_count].count= min_f +1;
		    new_frame[new_count++].id= i;
	            if( (min_f+1) < nf ) nf= min_f+1;
	        }
	    }

	    /* if no new frames, don't add */
	    if( !new_count ) continue;

	    /* fill in new node */
	    if( !(new_slt= get_new_node(cnfg)) ) return(-1);
	    if( first ) {
		first= 0;
		newtmp= tp;
	    }
	    else {
		newtmp= new_tmp();
		newtmp->id= tp->id;
		newtmp->n= tp->n;
		newtmp->len= tp->len;
		for( i=0; i < tp->len; i++) newtmp->tmp[i] = tp->tmp[i];
		newtmp->link= tp->link;
	    }
	    new_slt->tmp= newtmp;
	    new_slt->frame_id= fid_buf_ptr;
	    new_slt->n_act= new_count;
	    if( fid_buf_ptr + new_count >= fid_buf_end ) {
	       fprintf(stderr,"ERROR: overflow FidBufSize  %d\n",
			cnfg->FidBufSize);
	       return(-1);
	    }
	    for(i=0; i<new_count; i++) {
		*fid_buf_ptr++ = new_frame[i];
	    }

	    new_slt->back_link= link;
	    new_slt->n_frames= 1;

	    /* link in node */
    	    new_slt->link= seq_end[ew].link;
    	    seq_end[ew].link= new_slt;
        }
    }

    /* first slot in seq */
    else {
	/* add frames containing slot */
	new_count= 0;
	for(i=0; i < gram->num_frames; i++ ) {
		if( !tmp_in_frame(tp->id, i, gram) ) continue;
		new_frame[new_count].count= 1;
		new_frame[new_count++].id= i;
	}

	/* fill in and add node */
    	if( !(new_slt= get_new_node(cnfg)) ) {
		return(-1);
	}
    	new_slt->tmp= tp;
	new_slt->frame_id= fid_buf_ptr;
	new_slt->n_act= new_count;
	if( fid_buf_ptr + new_count >= fid_buf_end ) {
	    fprintf(stderr, "ERROR: overflow FidBufSize  %d\n",
			cnfg->FidBufSize);
	    return(-1);
	}
	for(i=0; i<new_count; i++) *fid_buf_ptr++ = new_frame[i];

    	new_slt->back_link= 0;
	new_slt->n_frames= 1;

	/* link in node */
    	new_slt->link= seq_end[ew].link;
    	seq_end[ew].link= new_slt;

    }

    seq_end[ew].score= new_score;
    seq_end[ew].n_slots= ns;
    seq_end[ew].n_frames= nf;
    return(0);
}


void reset_parse(Config *cnfg)
{
    int	i;
    Tmp *tp;
    SeqNode *sn;
	Tmp *t1;

 
    breakpt_reset(cnfg->num_nets);
	
    /* parse tree buffer */
    fid_buf_ptr= fid_buf;

    /* initialize slot sequence table */
    seq_buf_ptr= seq_buf;

    /* free partial templates */
    for(i= 0; i < num_tmps; i++) {
	for( tp= act_tmps[i]; tp; tp= t1)	{
		t1=tp->link;
		free(tp);
	}
	act_tmps[i]= 0;
    }

    /* free parses that were malloced */
    for(i=0; i< cnfg->num_parses; i++) {
	free(parses[i]);
	parses[i]= (SeqNode *)0;
    }

    /* free tmps in seq_end table
       initialize seq_end table */
    for(i=0; i < cnfg->script_len; i++) {
	for(sn= seq_end[i].link; sn; sn= sn->link) free(sn->tmp);
	seq_end[i].link= (SeqNode *)0;
	seq_end[i].score= 0;
	seq_end[i].n_slots= 0;
    }

    cnfg->script_len = 1;
    cnfg->num_parses= 0;

}


void read_line(char *line, Gram *gram, Config *cnfg)
{
    char	*s, *wd, word[LABEL_LEN];
    int		first_word = 1,
			w_idx;



    /* find word numbers for utt start and end tokens */
    cnfg->start_token= find_word(cnfg->start_sym, gram);
    cnfg->end_token= find_word(cnfg->end_sym, gram);

    /* convert word strings to numbers and put in script array */
    /* word position starts numbering from 1 */
    cnfg->script[0]= 0;
    cnfg->script_len= 1;

    /* first "word" is start of utt token */
    cnfg->script[cnfg->script_len++]= cnfg->start_token;

    /* skip blanks */
    s= line;
    while( *s == ' ' ) s++;

    /* for each word */
    while (sscanf(s, "%s", word) == 1) {
	if (*word == '+') {
	    /* noise event */
	}
	else {
	    first_word = 0;
	    wd = word;
	    if ((w_idx= find_word(word, gram)) == -1) {
		/* unknown word */
		if( !cnfg->IGNORE_OOV ) {
                    if( cnfg->script_len == cnfg->InputBufSize) {
                        fprintf(stderr, "ERROR: overflow InputBufSize %d\n",
                                cnfg->InputBufSize);
                        return;
		    }

		    cnfg->script[cnfg->script_len++]= w_idx;
		}
	    }
	    else {
		if( cnfg->script_len == cnfg->InputBufSize) {
		    fprintf(stderr, "ERROR: overflow InputBufSize %d\n",
				cnfg->InputBufSize);
		    return;
		}
		cnfg->script[cnfg->script_len++]= w_idx;
	    }
	}
	/* skip blanks */
	s= strchr(s, (int) ' ');
	if(!s) break;
	while( *s && (*s == ' ') ) s++;
    } 

    /* append end of utt token as last "word" */
    cnfg->script[cnfg->script_len++]= cnfg->end_token;
}


/* print parse parse_num to buffer out_str */
int print_parse(int parse_num, char *out_str, char *buf_end, char extract,
		Gram *gram, Config *cnfg)
{
    int	i, j,
	frame;
    SeqNode	*fptr;
    char	*out_ptr;
    Tmp	*tp;

    if( cnfg->num_parses < 1 ) {
		strcpy(out_str, "No parse");
		return(0);
    }
    if( parse_num >= cnfg->num_parses ) return(1);

    if( (buf_end - out_str) < MAX_PARSELEN) {
	fprintf(stderr, "ERROR: overflow OutBufSize %d\n", cnfg->OutBufSize);
		return(1);
    }

    out_ptr= out_str;
    frame= -1;

    fptr= parses[parse_num];
    for(j=0; j < cnfg->n_slots; j++, fptr++) {
	/* if starting a new frame */
	if( fptr->n_act != frame ) {
		frame= fptr->n_act;
		if( (frame < 0) || (frame > gram->num_frames) ) {
		    fprintf(stderr,"ERROR: frame  id out of range %d\n", frame);
		    return(1);
		}
	}

	/* print slot tree */
	sprintf(out_ptr, "%s:", gram->frame_name[frame]);
	out_ptr += strlen(out_ptr);
	init_print_extracts();
	tp= fptr->tmp;
	for(i=0; i < tp->len; i++) {
	    if( extract == 'y' ) {
	        out_ptr= print_extracts( tp->tmp[i], gram, out_ptr, 0, 
			gram->frame_name[frame], cnfg);
	    }
	    else {
		out_ptr= print_edge( tp->tmp[i], gram, out_ptr, cnfg);
	    }
	}
	if( out_ptr ) {
	    sprintf(out_ptr, "\n");
	    out_ptr += strlen(out_ptr);
	}
	else printf("\n");
    }

    fflush(stdout);
    return(0);
}



int init_parse(Config *cnfg)
{

    /* read grammar */
    cnfg->gram=read_grammar(cnfg);
    cnfg->cur_nets= (int *)0;
    cnfg->num_active= 0;

    /* malloc space for Frame id buffer */
    if( !(fid_buf=(Fid *)malloc(cnfg->FidBufSize * sizeof(Fid)))){
	fprintf(stderr, "ERROR: can't allocate space for Frame id buffer\n");
	return(-1);
    }
    fid_buf_end= fid_buf + cnfg->FidBufSize;
    
    /* malloc space for parses */
    if(!(pbuf=(SeqNode **)
	malloc(cnfg->ParseBufSize * sizeof(SeqNode **)))){
	    fprintf(stderr, "ERROR: can't allocate space for parse buffer\n");
	    return(-1);
    }
    pbuf_end= pbuf + cnfg->ParseBufSize;
    
    /* malloc space for parsed output */
    if(!(outbuf= malloc(cnfg->OutBufSize * sizeof(char)))){
	    fprintf(stderr, "ERROR: can't allocate space for parse output\n");
	    return(-1);
    }
    outbuf_end= outbuf + cnfg->OutBufSize;
    
    /* malloc space for parsed output */
    if(!(response= malloc(cnfg->OutBufSize * sizeof(char)))){
	    fprintf(stderr, "ERROR: can't allocate space for parse output\n");
	    return(-1);
    }
	/* initialize response buffer */
	*response = (char) 0;

    /* malloc space for action sequence */
    if(!(action_seq= malloc(cnfg->OutBufSize * sizeof(char)))){
	    fprintf(stderr, "ERROR: can't allocate space for parse output\n");
	    return(-1);
    }
	/* initialize action_seq buffer */
	*action_seq = (char) 0;

    /* malloc space for Seq buffer */
    if( !(seq_buf=(SeqNode *)malloc(cnfg->SeqBufSize * sizeof(SeqNode)))){
	fprintf(stderr, "ERROR: can't allocate space for Seq buffer\n");
	return(-1);
    }
    seq_buf_end= seq_buf + cnfg->SeqBufSize;
    
    /* malloc space for Seq end list */
    if( !(seq_end=(SeqCell *)calloc(cnfg->InputBufSize, sizeof(SeqCell)))){
	fprintf(stderr, "ERROR: can't allocate space for Seq end list\n");
	return(-1);
    }

    /* malloc space for script buffer */
    if( !(cnfg->script=(int *)malloc(cnfg->InputBufSize * sizeof(int)))){
	fprintf(stderr, "ERROR: can't allocate space for script buffer\n");
	return(-1);
    }

    /* malloc space for pointers to slots to search for */
    if( !(cnfg->active_slots= (int *) malloc(cnfg->num_nets * sizeof(int))) ) {
	fprintf(stderr, "ERROR: malloc active list failed\n");
	return(-1);
    }
    
#ifdef OLD
    /* if function words not to count in score */
    for(i=0; i < MAX_WRDS; i++) fun_wrds[i]= 0;
    if( function_wrd_file ) {
	if( (fp= fopen(function_wrd_file, "r")) ) {
	    while( fscanf(fp, "%s", name) == 1 ) {
		i= find_word(name, gram);
		if( i >= 0 ) fun_wrds[i]= 1;
	    }
	    fclose(fp);
	}
    }
#endif

    cnfg->script[0]= 0;
    cnfg->script_len = 1;

	/* malloc match buffers and chart */
	if( init_match(cnfg) ) return(-1);

    /* clear internal lists and buffers in parser */
    reset_parse(cnfg);

	return(0);
}

Tmp *new_tmp()
{
    Tmp *tp;

    if( !(tp=(Tmp *)malloc( sizeof(Tmp)))){
		fprintf(stderr, "ERROR: can't allocate space for Tmp\n");
		exit(-1);
    }
    return(tp);
}

void initTmp()
{
	int i;
	for(i=0; i<MAX_TMP; i++)
		act_tmps[i] = (Tmp *)0;
}
