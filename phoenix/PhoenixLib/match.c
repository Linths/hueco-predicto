#ifdef WIN
#include "stdafx.h"
#endif

#ifndef OSX
#include <malloc.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "phoenix.h"

static	EdgeLink	**chart;	/* chart of matched nets */
static	EdgeLink	*edge_link_buf,	/* buffer of nodes to link edges in lists */
					*edge_link_end,
					*edge_link_ptr;
static	Edge 		*edge_buf,			/* buffer of chart edges */
					*edge_buf_end, 
					*edge_ptr;			/* ptr to next free edge in buf */
static	Edge		**pe_buf,			/* buffer for trees pointed to by edges */
					**pe_buf_end,
					**pe_buf_ptr;


/* function declarations*/
int expand_path(Path *path, Gram *gram, Config *cnfg );
int check_chart(int net, int word_pos, Edge **edge);
int check_final(Path *path, Config *cnfg );
void breakpt_reset(int num_nets);

/* produce all parse trees for given net starting at word position
   add trees found to chart
   Creates an initial path node and recursively calls expand_path() 
   return 0 if ok, -1 if error
*/
int match_net(int net, int word_pos, Gram *gram, Config *cnfg)
{
    EdgeLink	*prev, *next;
    Path	path;
    int		expand_path_ret;

    /* create initial path node, with start state of net */
    path.state= gram->Nets[net];
    path.net= net;
    path.sw= word_pos;
    path.ew= 0;
    path.score= 0;
    path.nchld= 0;
    path.word_pos= word_pos;

    if( cnfg->VERBOSE > 5 ) {
		printf("match_net: %s %d\n", gram->labels[net], word_pos);
		fflush(stdout);
    }

    /* add to chart with edge= 0 in case no parse */
    if( edge_link_ptr == edge_link_end ) {
		printf("ERROR: edge link buffer overflow, ChartBufSize= %d\n", cnfg->ChartBufSize);
		return(-1);
    }
    edge_link_ptr->sw= word_pos;
    edge_link_ptr->edge= 0;

    /* find position in chart list for net */
    prev= 0;
    for( next= chart[net]; next; next= next->link) {
		if(word_pos <= next->sw) break;
		prev= next;
    }
    if( next && (word_pos == next->sw) ) {
		/* net already matched */
    }
    else if( !prev ) {
		/* insert at head of list */
		edge_link_ptr->link= chart[net];
		chart[net]= edge_link_ptr;
        edge_link_ptr++;
    }
    else {
		/* insert in list */
		edge_link_ptr->link= prev->link;
		prev->link= edge_link_ptr;
        edge_link_ptr++;
    }

    /* explore all paths leaving state */
    if( (expand_path_ret=expand_path(&path, gram, cnfg)) < 0)
	return(expand_path_ret);
    return(0);
}



int expand_path(Path *path, Gram *gram, Config *cnfg )
{
    int		len,
		result,
		ret,
		i;
    Gsucc	*suc;
    Path	new_path;
    Edge	*ptr;


    /* get pointer to arcs */
    len= path->state->n_suc;
    suc= path->state->succ;
    /* for each successor of state */
    for(i=0; i < len; i++) {
	/* if push arc */
	if( suc[i].call_net ) {
		/* if end of input */
		if( path->word_pos >= cnfg->script_len ) continue;

		/* check chart for net starting at word_pos */
		result= check_chart(suc[i].call_net, path->word_pos, &ptr);

		/* if (net,sw) not tried, see if match */
		if( !result ) {
			if( (ret=match_net(suc[i].call_net, path->word_pos, gram, cnfg)) <0)
				return(ret);
			result= check_chart(suc[i].call_net, path->word_pos, &ptr);
		}

		/* if no match */
		if( result < 0 ) continue;

		/* purse all parses for called net starting at word */
		/* ptr= 0 if no parses */
		for( ; ptr && (ptr->sw == path->word_pos); ptr= ptr->link) {
			/* transit arc and consume words */
			new_path= *path;
			new_path.state= suc[i].state;
			new_path.word_pos= ptr->ew+1;
			new_path.chld[(int)new_path.nchld++]= ptr;
			/* add to chart if final state */
			if( check_final( &new_path, cnfg ) < 0 ) return(-1);
			/* continue to expand path */
			if( (ret=expand_path( &new_path , gram, cnfg)) < 0) return(ret);
		}
	}
	/* if null arc */
	else if( !suc[i].tok && suc[i].state ) {
		/* transit without consuming word */
		new_path= *path;
		new_path.state= suc[i].state;
		/* add to chart if final state */
		if( check_final( &new_path, cnfg ) < 0) return(-1);
		/* continue to expand path */
		if( (ret=expand_path( &new_path , gram, cnfg)) < 0) return(ret);
	}
	/* else word arc */
	else if( path->word_pos < cnfg->script_len ) {
		if( suc[i].tok != cnfg->script[path->word_pos]) {
			/* no match */
			continue;
		}

		/* transit word arc and consume word */
		new_path= *path;
		new_path.state= suc[i].state;
		new_path.word_pos++;
		/* add to chart if final state */
		if( check_final( &new_path, cnfg ) < 0) return(-1);
		/* continue to expand path */
		if( (ret=expand_path( &new_path, gram, cnfg )) < 0) return(ret);
	}
    }
    return(0);
}


/* add matched token to chart */
static int add_to_chart(Path *path, Config *cnfg)
{
    Edge *prev, *next, *e;
    EdgeLink  *eln;
    int	i;

    if( edge_ptr == edge_buf_end ) {
		printf("ERROR: edge buffer overflow, EdgeBufSize= %d\n", cnfg->EdgeBufSize);
		return(-1);
    }

    /* find pointer entry in chart for net, start word */
    for(eln= chart[path->net]; eln; eln= eln->link) {
		if( eln->sw == path->sw ) break;
    }
    if( !eln ) {
		printf("ERROR: no entry in chart for net %d sw %d\n",
			path->net, path->sw);
		return(-1);
    }

    /* find position in list to insert */
    prev= 0;
    if( !eln->edge ) {
    }
    else {
		for(next= eln->edge; next; prev= next, next= next->link) {
			if(path->sw > next->sw) continue;
			if( (path->sw == next->sw) &&
				(path->ew > next->ew) ) continue;
			break;
		}

		/* if repeat of existing edge, don't add */
		for( e=next; e; e= e->link) {
			if( path->sw != e->sw) break;
			if( path->ew != e->ew) break;
			if( path->nchld != e->nchld ) break;
			for( i=1; i < path->nchld; i++) {
				if( path->chld[i] != e->chld[i] ) break;
			}
			if( i >= path->nchld ) return(0);
		}
    }

    /* copy path to edge */
    edge_ptr->net= path->net;
    edge_ptr->sw= path->sw;
    edge_ptr->ew= path->ew;
    edge_ptr->score= path->score;
    edge_ptr->nchld= path->nchld;

    if( (pe_buf_ptr + path->nchld +1) > pe_buf_end ) {
		printf("ERROR: overflow, PeBufSize= %d\n", cnfg->PeBufSize);
		return(-1);
    }
    edge_ptr->nchld= path->nchld;
    edge_ptr->chld= pe_buf_ptr;
    for(i=0; i < path->nchld; i++)   *(pe_buf_ptr++)= path->chld[i];

    /* insert edge in list for net, sw */
    if( !prev ) {
		/* insert at head */
		edge_ptr->link= eln->edge;
		eln->edge= edge_ptr;
    }
    else {
		edge_ptr->link= next;
		prev->link= edge_ptr;
    }

    edge_ptr++;
    return(0);
}

/* return 0 if net not tried, 1 if matched, -1 if not matched */
/* set ptr to edge matched, 0 if not */
int check_chart(int net, int word_pos, Edge **edge)
{
    EdgeLink *next;

    for( next= chart[net]; next; next= next->link) {
		if(word_pos <= next->sw) break;
    }
    *edge= 0;
    if( !next ) return(0);
    if( word_pos != next->sw ) return(0);
    if(!next->edge) return(-1); 
    *edge= next->edge;
    return(1);
}

/* edges for net are sorted by sw as major, ew as minor */
Edge *find_edge(int net, int sw, int ew)
{
    EdgeLink	*eln;
    Edge	*next,
		*e;

    /* find nets staring at sw */
    for( eln= chart[net]; eln; eln= eln->link) {
		if(sw <= eln->sw) break;
    }
    if(sw != eln->sw) return(0);

    /* in list for sw, find ew */
    for(next= eln->edge; next; next= next->link) {
		if(ew < next->ew) return(0);
		if(ew == next->ew) break;
    }
    if( !next ) return(0);

    /* find edge with fewest tokens in case ambiguous */
    for(e= next; e && (e->ew == next->ew); e= e->link) {
		if( e->nchld >= next->nchld ) continue;
	else  next= e;
    }
    return(next);
}

/* see if path is in final state or can trace through nulls to one */
int check_final(Path *path, Config *cnfg )
{
    if( !path->state->final ) return(0);
    /* update end word of phrase */
    if ( path->word_pos <= 0 ) {
    	printf ( "\nWARNING: null path encountered in net %d.\n", path->net );
		return (-1);
    }
    path->ew= path->word_pos-1;
    return( add_to_chart( path, cnfg )  );
}




int score_phrase(unsigned short sw, unsigned short ew)
{
	int	count;

	count= ew - sw +1;
#ifdef old
    for(count= 0, i= sw; i <= ew; i++) {
		if( !fun_wrds[ cnfg->script[i] ] ) count++;
    }
#endif
    return(count);
}


void clear_chart(int num_nets) {
  int i;

  /* clear chart */
  for(i=0; i < num_nets; i++) chart[i]= 0;
}


int init_match(Config *cnfg)
{

    /* malloc space for edge buffer */
    if( !(edge_buf= (Edge *) malloc( cnfg->EdgeBufSize * sizeof(Edge)) ) ) {
		fprintf(stderr, "ERROR: can't allocate space for Edge buffer\n");
		exit(-1);
    }
    edge_buf_end= edge_buf + cnfg->EdgeBufSize;
    
    /* malloc space for edge link buffer */
    if(!(edge_link_buf=(EdgeLink *) malloc( cnfg->ChartBufSize * sizeof(EdgeLink)))){
		fprintf(stderr, "ERROR: can't allocate space for Edge Link buffer\n");
		exit(-1);
    }
    edge_link_end= edge_link_buf + cnfg->ChartBufSize;
    
    /* malloc space for tree buffer */
    if(!(pe_buf=(Edge **) malloc( cnfg->PeBufSize * sizeof(Edge *)))){
		fprintf(stderr,"ERROR: can't allocate space for edge pointer buffer\n");
		exit(-1);
    }
    pe_buf_end= pe_buf + cnfg->PeBufSize;

	    /* malloc space for chart */
    if(!(chart=(EdgeLink **) malloc( cnfg->num_nets * sizeof(EdgeLink *)))){
		fprintf(stderr, "ERROR: can't allocate space for chart\n");
		exit(-1);
    }

	return(0);
}

void breakpt_reset(int num_nets)
{

  /* reset edge buffer */
  edge_ptr= edge_buf;
  edge_link_ptr= edge_link_buf;
  pe_buf_ptr= pe_buf;

  clear_chart(num_nets);
}
