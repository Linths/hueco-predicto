#ifdef WIN
#include "stdafx.h"
#endif

#ifndef OSX
#include <malloc.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "phoenix.h"


int check_tmp(int tmp_num);
Gram *read_grammar(Config *cnfg);
void read_nets(char *net_file, Gram *gram, char **sb_start, char *sym_buf_end, Config *cnfg);
void read_frames(char *dir, char *frames_file, Gram *gram, char **sb_start, char *sym_buf_end, Config *cnfg);
int find_net(char *name, Gram *gram);
int read_dic(char *dir, char *dict_file, Gram *gram, char **sb_start,
		char *sym_buf_end, Config *cnfg);

extern TmpDef tmps[MAX_TMP];
extern int	num_tmps;

/* read grammar and associated files */
Gram *read_grammar(Config *cnfg)
{
	Gram	*gram;
    char	name[PATH_LEN];
    char	*sym_ptr, *sym_buf_end;


    /* malloc structure to hold grammar configuration */
    if( !(gram= (Gram *) calloc(1, sizeof(Gram))) ) {
		fprintf(stderr, "ERROR: malloc for grammar failed\n");
		return((Gram *)0);
    }

    /* malloc space for symbol buffer */
    if( !(gram->sym_buf=(char *)malloc(cnfg->SymBufSize * sizeof(char)))){
		fprintf(stderr, "ERROR: can't allocate space for script buffer\n");
		return((Gram *)0);
    }
    sym_ptr= gram->sym_buf;
    sym_buf_end= gram->sym_buf + cnfg->SymBufSize;

    /* read dictionary word strings */
    gram->num_words= read_dic(cnfg->task_dir, cnfg->dict_file, gram, &sym_ptr, sym_buf_end,cnfg);

    /* read grammar networks */
    sprintf(name, "%s/%s", cnfg->task_dir, cnfg->grammar_file);
    read_nets(name, gram, &sym_ptr, sym_buf_end, cnfg);

    /* read frames and create set of active  nets */
    read_frames(cnfg->task_dir, cnfg->frames_file, gram, &sym_ptr, sym_buf_end, cnfg);
   
    return(gram);
}


/* read grammar networks */
void read_nets(char *net_file, Gram *gram, char **sb_start, char *sym_buf_end, Config *cnfg)
{
    FILE	*fp;
    Gnode	*net,
		*gnode;
    Gsucc	*succ;
    char	name[PATH_LEN],
			*sym_ptr;
    int		num_nodes,
		net_num,
		num_nets,
		offset,
		i,
		j;
    int		zz,
		val;

    sym_ptr= *sb_start;

    if( !(fp = fopen(net_file, "r") )) {
	fprintf(stderr, "Cannot open grammar file %s\n", net_file);
	return;
	}

    /* read number of nets */
    if( fscanf(fp, "Number of Nets= %d", &num_nets) < 1 ) {
	fprintf(stderr,"ERROR: bad format in grammar file %s\n", net_file);
	return;
    }
    /* alow for net numbers start at 1 */
    num_nets++;
    gram->num_nets= num_nets;
    cnfg->num_nets= num_nets;

    /* malloc space for net names pointers */
    if( !(gram->labels= (char **) calloc(num_nets, sizeof(char *))) ) {
		fprintf(stderr, "ERROR: malloc for labels failed\n");
		return;
    }

    /* malloc space for pointers to nets */
    if( !(gram->Nets= (Gnode **) malloc(num_nets * sizeof(Gnode *))) ) {
		fprintf(stderr, "ERROR: malloc for Nets failed\n");
		return;
    }

    /* malloc space for node counts to nets */
    if( !(gram->node_counts= (int *) calloc(num_nets, sizeof(Gnode *))) ) {
		fprintf(stderr, "ERROR: malloc for node counts failed\n");
		return;
    }

    /* malloc space for concept leaf flags */
    if( !(gram->leaf= (char *)malloc(num_nets * sizeof(char))) ) {
		printf("malloc failed\n");
		return;
    }

    /* read net name, net number, number of nodes and concept_leaf flag */
    while( fscanf(fp, "%s %d %d %d", name, &net_num, &num_nodes, &val) == 4 ) {
	gram->leaf[net_num]= val;
	/* save node count */
	gram->node_counts[net_num]= num_nodes;

        /* copy net name to labels array */
        strcpy(sym_ptr, name);
        gram->labels[net_num]= sym_ptr;
        sym_ptr += strlen(sym_ptr) +1;
        if( sym_ptr >= sym_buf_end ) {
	    fprintf(stderr,"ERROR: overflow SymBufSize %d\n", cnfg->SymBufSize);
	    return;
        }

        /* malloc space for nodes */
        if( !(net= (Gnode *) malloc( num_nodes* sizeof(Gnode) ) ) ) {
	    fprintf(stderr, "ERROR: malloc net failed: net %s  num_nodes %d\n",
				name, num_nodes);
	    return;
        }

	/* save pointer to start node in Nets array */
	gram->Nets[net_num]= net;

        /* read nodes */
        for( i=0, gnode= net; i < num_nodes; i++, gnode++ ) {
		/* read node */
		if( fscanf(fp, "%d %hd %hd", &zz,
			&gnode->n_suc, &gnode->final) != 3) {
			fprintf(stderr,"ERROR: reading grammar node %d\n", zz);
			return;
		}
		if( zz != i ) {
		    fprintf(stderr, "WARNING: net %s node %d out of seq\n",
				 name, zz);
		}

		if( !gnode->n_suc ) {
			gnode->succ= 0;
			continue;
		}

		/* malloc space for succs */
		if( !(succ= (Gsucc *) malloc( gnode->n_suc * sizeof(Gsucc)))) {
		    fprintf(stderr, "ERROR: malloc for succ nodes failed\n");
		    return;
		}
		gnode->succ= succ;

		/* read succs */
		for(j=0; j < gnode->n_suc; j++, succ++) {
			if( fscanf(fp, "%d %d %d", &succ->tok,
				&succ->call_net, &offset) != 3) {
				fprintf(stderr, "ERROR: reading node succs\n");
				return;
			}
			succ->state= net + offset;
		}
        }
    }
    fclose(fp);
    *sb_start= sym_ptr;
}




/* read frame definitions */
void read_frames(char *dir, char *frames_file, Gram *gram, char **sb_start,
		char *sym_buf_end, Config *cnfg)
{
    int		tmp_num, fn, j,
    		tmp_per_frame,
		slot_num,
		t_idx,
    		num_frames;
    char	name[PATH_LEN],
		line[LINE_LEN],
		*c,
		*r,
		*sym_ptr;
    unsigned short	*tmp_set;
    FILE	*fp;

    sym_ptr= *sb_start;

    /* open frames file */
    sprintf(line, "%s/%s", dir, frames_file);
    if( !(fp= fopen(line, "r")) ) {
		fprintf(stderr, "ERROR: can't open %s\n", line);
		return;
    }

    /* count frames and tmps_per_frame */
    tmp_per_frame= 0;
    tmp_num= 0;
    for(num_frames= 0; ; num_frames++ ) {

	/* scan for start of frame */
	while( (r= fgets(line, LINE_LEN, fp)) ) {
            sscanf(line, "%s%*[^\n]\n", name);
	    if( !strncmp(name, "Frame:", 6) ) break;
	}
	if( !r ) break;

	/* scan for net declarations */
	while( (r= fgets(line, LINE_LEN, fp)) ) {
            sscanf(line, "%s%*[^\n]\n", name);
	    if( !strncmp(name, "Elements:", 8) ) break;
	}
	if( !r ) {
		fprintf(stderr, "Bad format in frames file\n");
		return;
	}

		/* count net declarations */
		tmp_num= 0;
		while( (r= fgets(line, LINE_LEN, fp)) ) {
            		sscanf(line, "%s%*[^\n]\n", name);
			if( name[0] == ';' ) break;
			if( name[0] != '[' ) continue;	 /* if comment */
			tmp_num++;
		}
		if( !r ) {
			fprintf(stderr, "Bad format in frames file\n");
			return;
		}
		if( tmp_num > tmp_per_frame ) tmp_per_frame= tmp_num;

		/* scan for end of frame marker */
		if( name[0] != ';' ) {
			while( (r= fgets(line, LINE_LEN, fp)) ) {
                		sscanf(line, "%s%*[^\n]\n", name);
				if( name[0] == ';') break;
			}
			if( !r ) {
				fprintf(stderr, "Bad format in frames file\n");
				return;
			}
		}

    }

    gram->num_frames= num_frames;

    /* malloc space for frame definitions */
    if(!(gram->frame_def= (FrameDef *) malloc(num_frames * sizeof(FrameDef)))) {
		fprintf(stderr, "ERROR: malloc Frame templates failed\n");
		return;
    }
    /* malloc space for frame names */
    if( !(gram->frame_name= (char **) malloc(num_frames * sizeof(char **))) ) {
		fprintf(stderr, "ERROR: malloc Frame names failed\n");
		return;
    }

    /* malloc space for temporary template buffer */
    if( !(tmp_set= (unsigned short *) malloc(tmp_per_frame *
		                            sizeof(unsigned short *))) ) {
		fprintf(stderr, "ERROR: malloc form buffer failed\n");
		return;
    }

    rewind(fp);

    /* read frames */
    tmp_num= 0;
    for(fn= 0; ; fn++ ) {

		/* scan for start of frame */
		while( (r= fgets(line, LINE_LEN, fp)) ) {
            		sscanf(line, "%s%s%*[^\n]\n", name, sym_ptr);
			if( !strncmp(name, "Frame:", 6) ) break;
		}
		if( !r ) break;
		if( strncmp(name, "Frame:", 6) ) break;

		/* copy frame name */
		gram->frame_name[fn]= sym_ptr;
		sym_ptr += strlen(sym_ptr) +1;
		if( sym_ptr >= sym_buf_end ) {
			fprintf(stderr, "ERROR: overflow SymBufSize\n");
			return;
		}

		/* scan for net declarations */
		while( (r= fgets(line, LINE_LEN, fp)) ) {
            		sscanf(line, "%s%*[^\n]\n", name);
			if( !strncmp(name, "Elements:", 8) ) break;
		}
		if( !r ) {
			fprintf(stderr, "Bad format in frames file\n");
			return;
		}

		/* read templates for frame */
		slot_num= 0;
		while( fgets(line, LINE_LEN, fp) ) {
			if( line[0] == '#' ) continue;
			if( line[0] == ';' ) break;
			if( !(c= strchr(line, (int)'[')) ) continue;

			t_idx= 0;
			for(c=line; (c= strchr(c, (int)'[')); c++) {
			    sscanf(c, "%[^]]", name);
			    strcat(name, "]");

			    /* look up net number */
			    if( (j= find_net(name, gram)) > -1 ) {
				/* insert element in template */
				if( t_idx == (MAX_TMP-1) ) {
				 fprintf(stderr,"WARNING: template too long\n");
				}
				else {
				    tmps[tmp_num].el[t_idx++]= j;
				}
			    }
			    else {
				fprintf(stderr,
				"WARNING: can't find net %s for frame\n", name);
			    }
		    }

		    tmps[tmp_num].len= t_idx;

		    /* check for duplicate */
		    if( (j= check_tmp(tmp_num) ) >= 0 ) {
		        tmp_set[slot_num++]= (unsigned short)j;
		    }
		    else {
		        tmp_set[slot_num++]= (unsigned short)tmp_num;
		        tmp_num++;
		    }
		}

		/* copy tmp buffer to frame definition */
		gram->frame_def[fn].n_slot= slot_num;

		/* malloc space for tmp id array */
		if( !(gram->frame_def[fn].slot= (unsigned short *)
		       malloc(slot_num * sizeof(unsigned short *))) ) {
			fprintf(stderr, "ERROR: malloc frame def failed\n");
			return;
    		}

		/* copy tmp numbers */
		for(j= 0; j < slot_num; j++) {
			gram->frame_def[fn].slot[j]= tmp_set[j];
		}

    }

    *sb_start= sym_ptr;
    num_tmps= tmp_num;

    if( cnfg->VERBOSE > 3 ) {
	int i, k, z;

	printf("Templates:\n");
	for(i=0; i < num_tmps; i++) {
	    for(k=0; k < tmps[i].len; k++) {
	        z= tmps[i].el[k];
	        printf("\t%s ", gram->labels[z]);
	    }
	    printf("\n");
	}
    }

#ifdef debug
{
int i, k, z;
for(i=0; i < fn; i++) {
    printf("frame %d\n", i);
    for(j= 0; j< gram->frame_def[i].n_slot; j++) {
	for(k=0; k < tmps[gram->frame_def[i].slot[j]].len; k++) {
	    z= tmps[gram->frame_def[i].slot[j]].el[k];
	    printf("%s ", gram->labels[z]);
	}
	printf("\n");
    }
}
}
#endif

}

int check_tmp(int tmpn) {
    int i, k;

    for(i=0; i < tmpn; i++) {
	if( tmps[i].len != tmps[tmpn].len) continue;
	for(k=0; k < tmps[i].len; k++) {
	    if( tmps[i].el[k] != tmps[tmpn].el[k] ) break; 
	}
	if( k == tmps[i].len ) return(i);
    }
    return(-1);
}

#ifdef old
void read_net_names(char *dir, char *nets_file, Gram *gram, char **sb_start, char *sym_buf_end,
					Config *cnfg)
{
    int		i;
    char	name[PATH_LEN],
			line[LINE_LEN],
			*sym_ptr;
    FILE	*fp;

    sym_ptr= *sb_start;

    sprintf(line, "%s/%s", dir, nets_file);
    if( !(fp= fopen(line, "r")) ) {
		fprintf(stderr, "ERROR: can't open %s\n", line);
		return;
    }

    /* count number of nets */
    for(i= 0; fgets(line, LINE_LEN, fp); i++ );
    gram->num_nets= i+1; /* allow for net numbers start at 1 */

    /* malloc space for net names pointers */
    if( !(gram->labels= (char **) calloc(gram->num_nets, sizeof(char *))) ) {
		fprintf(stderr, "ERROR: malloc for labels failed\n");
		return;
    }


    rewind(fp);
    for(i= 1; fgets(line, LINE_LEN, fp); ) {
	if( sscanf(line, "%s", name) < 1 ) continue; /* empty line */
        /* copy net name to labels array */
		sprintf(sym_ptr, "[%s]", name);
        gram->labels[i++]= sym_ptr;
        sym_ptr += strlen(sym_ptr) +1;
        if( sym_ptr >= sym_buf_end ) {
	    fprintf(stderr,"ERROR: overflow SymBufSize %d\n", cnfg->SymBufSize);
	    return;
        }
    }

    *sb_start= sym_ptr;
}
#endif

/* input net name, return net number */
int find_net(char *name, Gram *gram)
{
    int i;

    if( !*name ) return(-1);
    for( i=1; i < gram->num_nets; i++ ) {
		if( !gram->labels[i] ) continue;
		if(!strcmp(gram->labels[i], name) ) break;
    }

    if( i < gram->num_nets ) return(i);
    return(-1);

}



/* input frame name, return frame number */
int find_frame(char *name, Gram *gram)
{
    int i;

    if( !*name ) return(-1);
    for( i=0; i < gram->num_frames; i++ ) {
		if(!strcmp(gram->frame_name[i], name) ) break;
    }

    if( i < gram->num_frames ) return(i);
    return(-1);

}

void free_grammar( Gram *gram) {
    int		net_n,
			node_n,
			fn;
    Gnode	*gnode;

    if( gram->sym_buf ) free(gram->sym_buf);
    if( gram->wrds ) free(gram->wrds);
    if( gram->labels ) free(gram->labels);

    /* for each net */
    for( net_n=0; net_n < gram->num_nets; net_n++) {
		gnode= gram->Nets[net_n];

		/* for each node */
		for( node_n=0; node_n < gram->node_counts[net_n]; node_n++, gnode++) {  
			if( gnode->succ ) free(gnode->succ);
		}

		if( gram->Nets[net_n] ) free(gram->Nets[net_n]);
    }
    free( gram->Nets );
    free( gram->node_counts );
    free( gram->leaf );
	
    /* free frames */
    for(fn=0; fn < gram->num_frames; fn++)
		if( gram->frame_def[fn].slot ) free( gram->frame_def[fn].slot );
    if( gram->frame_def ) free( gram->frame_def );
    if( gram->frame_name ) free( gram->frame_name );
 
    free( gram );
    gram= (Gram *)0;
}
