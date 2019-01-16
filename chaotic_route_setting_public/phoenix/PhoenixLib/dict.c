/* routine to read dictionary */
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

int isword( int c );
void sort_dic(char **wrds, int num_w);

int read_dic(char *dir, char *dict_file, Gram *gram, char **sb_start,
		char *sym_buf_end, Config *cnfg)
{
    FILE	*fp;
    int		num_words;
    char	filename[PATH_LEN];
    char	*sym_ptr;

    sprintf(filename, "%s/%s", dir, dict_file);

    if( !(fp= fopen(filename, "r")) ) {
		printf("read_dict: can't open %s\n", filename);
		return(0);
    }

    sym_ptr= *sb_start;

    /* malloc space for word pointers */
    if( !(gram->wrds=(char **)calloc(MAX_WRDS, sizeof(char *)))){
		printf("can't allocate space for script buffer\n");
		return(-1);
    }

    /* word 0 is *  */
    strcpy(sym_ptr, "*"); 
    sym_ptr += strlen(sym_ptr)+1;
    gram->wrds[0]= sym_ptr;

    for(num_words=1; fscanf(fp, "%s", sym_ptr) == 1; num_words++) {
	gram->wrds[num_words]= sym_ptr;
	sym_ptr += strlen(sym_ptr) +1;
	if( sym_ptr >= sym_buf_end ) {
	    fprintf(stderr,"ERROR: overflow SymBufSize %d\n", cnfg->SymBufSize);
	    return(-1);
	}
    }

    fclose(fp);
    *sb_start= sym_ptr;
    gram->num_words= num_words;
    return(num_words);
}




int find_word(char *s, Gram *gram)
{
    int	hi, low, id;
    int res;

    for(low= 1, hi= gram->num_words; ; ) {
	id= (hi+low)/2;
	res= strcmp( gram->wrds[id], s);
	if( !res )  return(id);
	if( id == low ) return(-1);
	if( res < 0 ) {
	    low= id;
	}
	else hi= id;
    }
    return(-1);
}

int mk_dic(char *gram_file, char *dic_file, Config *cnfg)
{

    FILE	*fp_gram,
		*fp_dic;
    char	*s, *w;
    char	line[1000];		/* input line buffer */
    char	word[100];		/* output text buffer for parses */
    char	*wrds[MAX_WRDS];
    char	*buf;
    char	*bp;
    int		num_w;
    int		i;


    if( !(fp_gram= fopen(gram_file, "r")) ) {
	printf("can't open grammar file %s\n", gram_file);
	exit(-1);
    }

    if( !(fp_dic= fopen(dic_file, "w")) ) {
	printf("can't open dictionary file %s\n", dic_file);
	exit(-1);
    }

    if( !(buf= malloc(cnfg->SymBufSize * sizeof(char)))){
		printf("can't allocate space for dict\n");
		return(-1);
    }
    bp= buf;

    num_w= 0;
    /* for each utterance */
    for( ; fgets(line, 1000-1, fp_gram);  ) {
	/* if printing comment */
	if (line[0] == ';' ) continue;
	/* if non-printing comment */
	if (line[0] == '#' )  continue;
	if (line[0] == '[' )  continue;
	if (line[0] == ';' )  continue;

	/* find start of rule */
	if( !(s= strchr(line, (int)'(')) ) continue;

	while( *s && (*s != ')' ) ) {
	    for( ; *s && (*s != '\n'); s++ ) {
	        if( *s == ')' ) break;
	        if( isword( (int)*s ) ) break;
	    }

	    /* end of rule */
	    if( *s == ')' ) break;

	    /* next line continues rule */
	    if( *s == '\n' ) {
		if( !fgets(line, 1000-1, fp_gram) ) break;
		s= line;
		continue;
	    }

	    /* start of new word */
	    if( !isword( (int)*s ) ) break;
	    for( w= word; isword( (int) *s ); w++, s++ ) *w= *s;
	    *w= (char)0;

	    /* see if existing word */
	    for( i=0; i < num_w; i++) if( !strcmp(word, wrds[i]) ) break;
	    /* add new word */
	    if( i == num_w ) {
	        if( (((int)(bp-buf)) + (int)strlen(word)) > cnfg->SymBufSize ) {
		    printf("ERROR: SYMBUFSIZE overflow=%d\n", cnfg->SymBufSize);
		    exit(-1);
	        }
	        if( num_w == (MAX_WRDS-1) ) {
		    printf("ERROR: dic too large\n");
		    exit(-1);
	        }
		wrds[num_w++]= bp;
		strcpy(bp, word);
		bp += strlen(bp) +1;
	    }
	}

    }

    /* sort dic */
    sort_dic(wrds, num_w);

    /* write to file */
    for(i=0; i< num_w; i++ ) fprintf(fp_dic, "%s\n", wrds[i]);

    fclose(fp_gram);
    fclose(fp_dic);

    return(0);
}

void sort_dic(char **wrds, int num_w)
{
    int	last,
	new_last,
	i,
	done;
    char	*tmp;

    last= num_w-1;
    for( done= 0; !done; ) {
	done= 1;
	new_last= last;
	for( i=0; i < last; i++ ) {
	    if( strcmp(wrds[i], wrds[i+1] ) > 0 ) {
		tmp= wrds[i];
		wrds[i]= wrds[i+1];
		wrds[i+1]= tmp;
		done= 0;
		new_last= i+1;
	    }
	}
	last= new_last;
    }
}

int isword( int c )
{
    if( islower( c ) ) return(1);
    if( (char)c == '\'' ) return(1);
    return(0);
}
