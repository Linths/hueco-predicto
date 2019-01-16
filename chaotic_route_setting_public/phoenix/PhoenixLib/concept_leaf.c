/* flags leaf nodes in nets for extracts function */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "phoenix.h"

char is_leaf(int net, Config *cnfg);
void write_out(int net_id, FILE *fp_out, Config *cnfg);
Gram *read_grammar();


char	*leaf;
extern char	*sym_buf,	 	/* buff for ascii input strings */
	*sym_ptr, *sym_buf_end;

void concept_leaf( Config *cnfg)
{
    int		netid;
    FILE	*fp_out;

    sym_ptr= sym_buf;

    cnfg->gram= read_grammar(cnfg);
    cnfg->num_nets= cnfg->gram->num_nets;

    if( !(leaf= (char *)malloc(cnfg->num_nets * sizeof(char))) ) {
	printf("malloc failed\n");
	exit(-1);
    }
    for(netid= 0; netid < cnfg->num_nets; netid++) leaf[netid]= (char)-1;

    /* set concept leaf flag for all nets, positive if no concept subnets */
    for(netid= 1; netid < cnfg->num_nets; netid++) {
	if( leaf[netid] != -1 ) continue;
	if( !cnfg->gram->labels[netid]) {
	    fprintf(stderr, "Net %d not found\n", netid);
	    continue;
	}
	if( !isupper((int) *(cnfg->gram->labels[netid]+1)) ) continue;
	leaf[netid]= is_leaf( netid, cnfg );
    }

    /* mark non-concept nets as not concept leaves */
    for(netid= 1; netid < cnfg->num_nets; netid++) {
	    if(!cnfg->gram->labels[netid] ||
			!isupper((int)*(cnfg->gram->labels[netid]+1)))
		leaf[netid]= 0;
    }

    /* open output .net file */
    if( !(fp_out= fopen(cnfg->grammar_file,"w")) ) {
	printf("can't open %s\n", cnfg->grammar_file);
	exit(1);
    }
    /* write number of nets at start of file */
    fprintf(fp_out, "Number of Nets= %d\n", cnfg->num_nets-1);

    for(netid= 1; netid < cnfg->num_nets; netid++) {
	write_out(netid, fp_out, cnfg);
    }
    fclose(fp_out);
    return;
}

char is_leaf(int net, Config *cnfg)
{
    int		nc, sc;
    Gnode	*gn;
    Gsucc	*gs;

    /* for each node in net */
    for( nc=0, gn=cnfg->gram->Nets[net]; nc < cnfg->gram->node_counts[net]; nc++, gn++) {
	/* for each sucsessor (arc) of node */
	for(sc=0, gs= gn->succ; sc < gn->n_suc; sc++, gs++) {
	    /* id not call arc */
	    if( !gs->call_net ) continue;
	    if( gs->call_net >= cnfg->num_nets ) {
		fprintf(stderr, "net %d not found\n", gs->call_net);
		exit(-1);
	    }

	    if( !cnfg->gram->labels[gs->call_net]) {
		fprintf(stderr, "net %d not found\n", gs->call_net);
		exit(-1);
	    }

	    /* if concept net */
	    if( isupper((int)*(cnfg->gram->labels[gs->call_net]+1)) ) {
		return((char)0);
	    }
	    else {
		leaf[gs->call_net]= is_leaf(gs->call_net, cnfg);
		if(!leaf[gs->call_net]) return((char)0);
	    }
	}
    }
    return((char)1);

}


void write_out(int net_id, FILE *fp_out, Config *cnfg)
{
    Gnode	*gn;
    Gsucc	*gs;
    int		i, j;

    /* write net name, net number, number of states and concept_leaf flag */
    fprintf(fp_out, "%s %d %d %d\n",
	cnfg->gram->labels[net_id], net_id, cnfg->gram->node_counts[net_id], leaf[net_id]);

    /* write out nodes */
    for(i=0, gn= cnfg->gram->Nets[net_id];  i < cnfg->gram->node_counts[net_id]; i++, gn++) {
	/* write node */
	fprintf(fp_out, "%d  %d %d\n", i, gn->n_suc, gn->final);
	/* write arcs */
	for(j=0, gs= gn->succ; j < gn->n_suc; j++, gs++) {
            fprintf(fp_out, "\t\t%d    %d    %d\n", gs->tok,
                gs->call_net, (int) (gs->state - cnfg->gram->Nets[net_id]));
      }
    }

}
