#ifndef __DM_H__
#define __DM_H__

#define MAX_STR_LENGTH 128
#define	MAX_SLOT	40
#define MAX_RULE    10
#define MAX_RULE_VISITS 5
#define	MAX_NET		500
#define MAX_LEG	4
#define MAX_FORM 10
#define MAX_FIELDS	100  /* maximum fields in single form */
#define MAX_FRAME_STACK	100  /* maximum active frames in a session */
#define MAX_ACTION	10	

#define get_slot(tree, slot)   sscanf(tree, "%[^.\n]", slot)
#include "rules.h"

typedef struct slot_def {
	char	*key;
    int     n_prompt;
	char	*prompt[MAX_ACTION];  // Longer term this should be a ptr to a ptr
	char	*action;
	char	*sql;
	char	*conf_prompt;
	char	required;
} Slot;

typedef struct rule_def {
    RulesTreeNode   *rule_tree;
    int             n_prompt;
    char            *prompt[MAX_ACTION]; // This too should be changed to be a ptr to a ptr
} Rule;

typedef struct aframe_def {
	char	*name;
    char    *description;
	int	n_slot;
	Slot	*slot;
    int     n_rule;
    Rule    *rule;
    int     rule_visits[MAX_RULE];
} aFrameDef;

typedef struct aframe {
	char	*name;			/* frame name */
	char	**value;		/* pointer to element fillers */
	char	*prompt_count;		/* prompt count for each element */
	struct aframe	*rtn_f;		/* pointer to calling frame */
	int	rtn_e;			/* calling element */
	char	*confirmed;
	char	*consistent;
} aFrame;


typedef struct prmpt {
	aFrame	*frame;	/* frame prompted for */
	char	*key;	/* key prompted for */
	char	*type;	/* type of prompt, "value", "yes", "confirm" */
} Prompt;

#endif
