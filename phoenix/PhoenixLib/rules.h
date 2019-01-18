#ifndef __RULES_H__
#define __RULES_H__
#include <stdio.h>
/*******************************************************************************
 * rules.h -functions and structures used for doing exception rules on frames
 ******************************************************************************/

#define NODE_FUNCTION_SIG (struct aFrame*, struct Config*, RulesTreeNodeChild args[])

/* Forward declare frame and config types  (defined in dm.h) */
struct aFrame;
struct Config;


/* structures and date types for a node in the Rule Tree */
typedef union RulesTreeNodeChild {
    struct RulesTreeNode* node;    
    char*                 str;
} RulesTreeNodeChild;

typedef struct RulesTreeNode {

    RulesTreeNodeChild args[2];
    int (*function) NODE_FUNCTION_SIG;
                
} RulesTreeNode;


// Node creation functions
RulesTreeNode* create_and_node(RulesTreeNode* child0, RulesTreeNode* child1);
RulesTreeNode* create_or_node(RulesTreeNode* child0, RulesTreeNode* child1);
RulesTreeNode* create_key_eq_key_node(char* key0, char* key1);
RulesTreeNode* create_key_neq_key_node(char* key0, char* key1);
RulesTreeNode* create_key_eq_keyval_node(char* key, char* keyval);
RulesTreeNode* create_key_neq_keyval_node(char* key, char* keyval);
RulesTreeNode* create_key_is_empty_node(char* key);
RulesTreeNode* create_key_is_filled_node(char* key);

// Node deletion function
void rules_node_delete(RulesTreeNode* node);
void rules_tree_delete(RulesTreeNode* node);

void print_rules_tree(FILE* fp, RulesTreeNode* node);
void print_rules_node(FILE* fp, RulesTreeNode* node);



/* evaluates the function tree associated with node */
int rules_node_eval(struct aFrame* frame, struct Config* cnfg, RulesTreeNode* node);


#endif



