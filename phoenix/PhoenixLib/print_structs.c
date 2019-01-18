#ifdef WIN
#include "stdafx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "phoenix.h"


static int	pwp;
static int	first_concept;


char *print_edge(Edge *edge, Gram *gram, char *s, Config *cnfg)
{
    Edge	**cp;
    int		nxt, i;

    if( s ) {
		sprintf(s, "%s ( ", gram->labels[edge->net]);
		s += strlen(s);
    }
    else printf("%s ( ", gram->labels[edge->net]);

    pwp= edge->sw;
    /* print children */
    for( cp= edge->chld, i=0; i < edge->nchld; i++, cp++) {
	for(nxt= (*cp)->sw; pwp < nxt; pwp++) {
	    if( cnfg->script[pwp] == -1 ) continue;
	    if( cnfg->script[pwp] == cnfg->start_token) continue;
	    if( cnfg->script[pwp] == cnfg->end_token) continue;
	    if( s ) {
		sprintf(s, "%s ", gram->wrds[ cnfg->script[pwp] ] );
		s += strlen(s);
	    }
	    else printf("%s ", gram->wrds[ cnfg->script[pwp] ] );
	}
	s= print_edge( *cp, gram, s, cnfg );
	pwp= (*cp)->ew+1;
    }
    for( ; pwp <= edge->ew; pwp++) {
	    if( cnfg->script[pwp] == -1 ) continue;
	    if( cnfg->script[pwp] == cnfg->start_token) continue;
	    if( cnfg->script[pwp] == cnfg->end_token) continue;
	    if( s ) {
		sprintf(s, "%s ", gram->wrds[ cnfg->script[pwp] ] );
		s += strlen(s);
	    }
	    else printf("%s ", gram->wrds[ cnfg->script[pwp] ] );
    }
    if( s ) {sprintf(s, ") "); s += strlen(s); }
    else printf(") ");

    return(s);
}




char *print_extracts(Edge *edge, Gram *gram, char *s, int level, char *fn, Config *cnfg)
{
    Edge	**cp;
    int		nxt, i;
    char	concept,
		name[LABEL_LEN];

    /* get rid of [ */
    strcpy(name, gram->labels[edge->net]+1);
    if( isupper((int)name[0]) ) concept= (char) 1;
    else concept= (char) 0;
    /* if flag */
    if( name[0] == '_' ) {
		/* get rid of ] */
		name[strlen(name)-1]= 0;
		if( s ) {
			sprintf(s, "%s ", name+1 );
			s += strlen(s);
		}
		else printf("%s ", name+1 );
    }

    /* if concept */
    else if( concept ) {
		/* start slot on new line */
		if( !level && !first_concept ) {
            if( s ) { sprintf(s, "\n%s:", fn); s += strlen(s); }
			else printf("\n");
		}

        if( s ) {
			sprintf(s, "%s.", gram->labels[edge->net]);
			s += strlen(s);
        }
        else printf("%s.", gram->labels[edge->net]);

		first_concept= 0;
    }

    pwp= edge->sw;
    /* if concept leaf, print associated string */
    if( gram->leaf[edge->net] ) {
		/* if flag value */
		if( edge->nchld && (*(gram->labels[edge->chld[0]->net]+1) == '_') ) {
    	    strcpy(name, gram->labels[edge->chld[0]->net]+1);
			/* get rid of ] */
			name[strlen(name)-1]= 0;
			/*	strip_underscore(name); */
			if( s ) {
				/* get rid of _ */
				sprintf(s, "%s ", name +1 );
				s += strlen(s);
			}
			else printf("%s ", name+1 );
			pwp= edge->ew+1;
		}
		/* else print string */
		else {

            for( ; pwp <= edge->ew; pwp++) {
				if( cnfg->script[pwp] == -1 ) continue;
				if( cnfg->script[pwp] == cnfg->start_token) continue;
				if( cnfg->script[pwp] == cnfg->end_token) continue;
				if( s ) {
					sprintf(s, "%s ", gram->wrds[ cnfg->script[pwp] ] );
					s += strlen(s);
				}
				else printf("%s ", gram->wrds[ cnfg->script[pwp] ] );
            }
		}
    }
    else {
        /* print children */
        for( cp= edge->chld, i=0; i < edge->nchld; i++, cp++) {
			for(nxt= (*cp)->sw; pwp < nxt; pwp++) ;
			s= print_extracts( *cp, gram, s, (concept) ? level+1 : level, fn, cnfg);
			pwp= (*cp)->ew+1;
        }
    }

    return(s);
}

void init_print_extracts()
{
	first_concept= 1;
}

#ifdef old
void print_seq(SeqNode *ss, char **lbl)
{
    SeqNode	*sq, *np;

	for( sq= ss; sq; sq= sq->link) {
	    for( np= sq; np; np= np->back_link) {
		printf("%s ", lbl[np->edge->net]);
	    }
	    printf("\n");
    	}
}
#endif

void strip_underscore(char *str)
{
    char *f, *t;

    for(f= t= str; *f; f++) {
	if( *f != '_' ) { *t++ = *f; continue;}
	if( *(f+1) == '_' ) { *t++ = *f; f++; continue;}
	else  *t++ = ' ';
    }
    *t= (char)0;
}
