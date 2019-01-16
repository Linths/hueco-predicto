/************** file paths  *************************/
#define CONFIG_FILE	"phoenix_config"

/************** Define lengths **********************/
#define MAX_WRDS	0xFFFF	/* max words in lexicon */
#define LABEL_LEN	50	/* max length of labels */
#define LINE_LEN	2000	/* max length line buffer */
#define	PATH_LEN	200	/* max length of path names */
#define TMPLEN		5	/* max number of elementrs in a template */
#define MAX_TMP		500	/* max number of individual templates */
#define MAX_PARSELEN	1000	/* max length of single parse */

/************** Default Buffer Sizes ******************/
#define EDGEBUFSIZE	1000	/* max number of paths in beam */
#define	CHARTBUFSIZE	40000	/* max number of paths in beam */
#define	PEBUFSIZE	2000	/* number of Val slots for trees  */
#define INPUTBUFSIZE	1000	/* max words in line of input */
#define STRINGBUFSIZE	50000	/* max words in line of input */
#define SLOTSEQLEN	200	/* max number of slots in a sequence */
#define FRAMEBUFSIZE	500	/* buffer for frame nodes */
#define SYMBUFSIZE	200000	/* buffer size to hold char strings */
#define PARSEBUFSIZE	200	/* buffer for parses */
#define OUTBUFSIZE	10000	/* buffer for output parses */
#define SEQBUFSIZE	500	/* buffer for sequence nodes */
#define FIDBUFSIZE	1000	/* buffer for frame i */


/************** grammar structures ******************/
/* grammar state structure */
typedef struct gnode
{
	short	n_suc;		/* number of succs */
	short	final;		/* true if final state */
	struct gsucc	*succ;	/* arcs out */
} Gnode;

/* grammar arc structure */
typedef struct gsucc
{
	int	tok;		/* word number, call_netber or 0 */
	int	call_net;	/* 0 or number of net called */
	Gnode	*state;		/* ptr to successor state */
} Gsucc;

typedef struct suc_link
{
	Gsucc	succ;
	struct suc_link *link;
	int nt;
} SucLink;

struct state_set {
	int state;
	char used;
	struct state_set *next;
};
typedef struct state_set set;

typedef struct {
	int tok;
	SucLink *arc;
	char	rw;	/* has been rewritten flag */
} non_term;


/*********************** Parser structures **************************/
#define	PATH_TREE_DEPTH	50	/* max call-nets in single rewrite rule */
#define HIST_LEN	5
#define PARSEBUF_SIZE	2000

typedef unsigned short	Id;

typedef struct framedef {
	Id	n_slot;		/* number of slots in form */
	Id	*slot;		/* net numbers for slots */
} FrameDef;

typedef struct gram
{
    FrameDef	*frame_def;		/* nets used in each form */
    char	**frame_name;		/* frame names */
    int		num_frames;		/* number of frames read in */
    char	**labels;		/* names of nets */
    Gnode	**Nets;			/* pointers to heads of nets */
    int		num_nets;		/* number of nets read in */
    char	**wrds;			/* pointers to strings for words */
    int		num_words;		/* number of words in lexicon */
    int		*node_counts;		/* number of nodes in each net */
    char	*leaf;			/* concept leaf flags */
    char	*sym_buf;		/* strings for words and names */
} Gram;


/* cells of chart */
typedef struct edge {
    Id			net;
    Id			sw;
    Id			ew;
    Id			score;
    char		nchld;
    struct edge		**chld;
    struct edge		*link;
} Edge;


/* temporary structure used in matching nets to create edges */
typedef struct buf {
	Gnode	*state;		/* current state */
	Id	net;		/* number of top-level net */
	Id	sw;		/* number of top-level net */
	Id	ew;		/* number of top-level net */
	Id	score;		/* number of top-level net */
	char	nchld;
	Edge	*chld[PATH_TREE_DEPTH];	/* pointers to sub-trees (children) */
	int	word_pos;	/* extended flag */
}Path;



/* structure for linking edges into chart */
typedef struct edge_link {
	int	sw;
	struct edge_link *link;
	Edge *edge;
}EdgeLink;


typedef struct frame_id {
	unsigned short id;
	unsigned short count;
} Fid;

typedef struct tempdef {
	int	len;
	int	el[TMPLEN];
} TmpDef;

typedef struct template {
	int	id;
	int	n;
	int	len;
	Edge	*tmp[TMPLEN];
	struct	template *link;
} Tmp;

typedef struct seq_node {
	Tmp *tmp;
	unsigned short	n_frames;	/* frame count for path */
	unsigned short	n_act;	/* number of active frames for path */
	Fid	*frame_id;
	struct seq_node *back_link;
	struct seq_node *link;
	unsigned short *pri;
} SeqNode;

typedef struct seq_cell {
	Id score;
	Id n_slots;
	Id n_frames;
	SeqNode *link;
} SeqCell;

typedef struct frame_node {
	int n_frames;
	int frame;
	SeqNode *slot;
	struct frame_node *bp;
	struct frame_node *link;
} FrameNode;

/********************** Configuration ***************************/
typedef struct conf_struct {
	/*  flags  */
	int	VERBOSE;
	char	EXTRACT;
	char	ALL_PARSES;
	char	IGNORE_OOV;
	char	PROFILE;
	/* file pointers */
	FILE	*fp_debug;	/* file for debug output */

	/* file names */
	char	task_dir[PATH_LEN],	/* dir containing grammar, dict, etc */
		dict_file[LABEL_LEN],		/* dictionary */
		grammar_file[LABEL_LEN],	/* compiled grammar file */
		grammar_rules[LABEL_LEN],	/* grammar rules file */
		frames_file[LABEL_LEN],		/* frames file for parser */
		task_file[LABEL_LEN],		/* task file for DM */
		nets_file[LABEL_LEN],		/* list of nets */
		script_file[LABEL_LEN];		/* script file for DM */

	/* variables */
	int	start_token,			/* start of sentence token */
		end_token;
	char	*start_sym,
		*end_sym;

	/* pointers to structures and variables */
	Gram	*gram;				/* grammar */
	int	num_nets,
		num_tmps,			/* number of templates */
		num_frames;
	int	*active_slots,	/* set of slot level nets used in forms */
		*cur_nets, 	/* set of nets to be used in this parse */
		num_active;	/* number of active slots */
	int	*script,	/* array of word nums for input line */
		script_len;		/* number of words in script */
	int	*matched_script;	/* words included in parse */
	int	num_parses;			/* number of parses produced */
	int	n_slots;			/* number of slots in parse */
	char	*print_line;		/* buffer for output */
	int	task_frames;		/* number of frames in task file */
	int	task_slots;		/* max slots in any task frame */
    int task_rules;     /* max number of rules in any task frame */

	/* parameter Limits */
	int			MaxPrompt;


	/* Buffer Sizes */
	int	EdgeBufSize,		/* max number of paths in beam */
		ChartBufSize,		/* max number of paths in beam */
		PeBufSize,		/* number of Val slots for trees  */
		InputBufSize,		/* max words in line of input */
		SlotSeqLen,		/* max number of slots in a sequence */
		SymBufSize,		/* buffer size to hold char strings */
		ParseBufSize,		/* buffer for parses */
		OutBufSize,		/* buffer for output parses */
		SeqBufSize,			/* buffer for sequence nodes */
		TokBufSize,
		MaxNonTerm,
		MaxSymbol,
		MaxNfa,
		MaxSucLink,
		FidBufSize;			/* buffer for frame ids */

} Config;

