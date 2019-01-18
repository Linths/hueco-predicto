#include "rules.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define DEBUG 0

char    *getVal(struct aFrame *frame, char *key, struct Config *cnfg);
char    **getKeyPtr(struct aFrame *frame, char *key, struct Config *cnfg);
void    cp_str(char **to, char *from);




// Function Definitions for node functions
int rules_node_and(struct aFrame*, struct Config*, RulesTreeNodeChild args[]);
int rules_node_or(struct aFrame*, struct Config*, RulesTreeNodeChild args[]);
int rules_node_key_eq_key(struct aFrame*, struct Config*, RulesTreeNodeChild args[]);
int rules_node_key_neq_key(struct aFrame*, struct Config*, RulesTreeNodeChild args[]);
int rules_node_key_eq_keyval(struct aFrame*, struct Config*, RulesTreeNodeChild args[]);
int rules_node_key_neq_keyval(struct aFrame*, struct Config*, RulesTreeNodeChild args[]);
int rules_node_key_is_empty(struct aFrame*, struct Config*, RulesTreeNodeChild args[]);
int rules_node_key_is_filled(struct aFrame*, struct Config*, RulesTreeNodeChild args[]);

// Helper function for creating nodes
RulesTreeNode* rules_node_create(RulesTreeNodeChild args[],
                        int (*function)NODE_FUNCTION_SIG);

// Helper function for initializing nodes
int rules_node_init(RulesTreeNode* node, RulesTreeNodeChild args[],
                        int (*function)NODE_FUNCTION_SIG);


RulesTreeNode* create_key_eq_key_node(char* key0, char* key1) {
    RulesTreeNodeChild args[2];
    RulesTreeNode *node;

    args[0].str = NULL;
    args[1].str = NULL;
    cp_str(&args[0].str, key0);
    cp_str(&args[1].str, key1);

    node = rules_node_create(args, &rules_node_key_eq_key);
    return node;
}

RulesTreeNode* create_key_neq_key_node(char* key0, char* key1) {
    RulesTreeNodeChild args[2];
    RulesTreeNode *node;

    args[0].str = NULL;
    args[1].str = NULL;
    cp_str(&args[0].str, key0);
    cp_str(&args[1].str, key1);

#if DEBUG
    // FIXME: Change this to work with a verbosity bitmap
    printf("DEBUG create_key_neq_key_node(): key0 %s\n", key0);
    printf("DEBUG create_key_neq_key_node(): key1 %s\n", key1);
    printf("DEBUG create_key_neq_key_node(): %s\n", args[0].str);
    printf("DEBUG create_key_neq_key_node(): %s\n", args[1].str);
#endif
    node = rules_node_create(args, &rules_node_key_neq_key);
    return node;
}

RulesTreeNode* create_key_eq_keyval_node(char* key, char* keyval) {
    RulesTreeNodeChild args[2];
    RulesTreeNode *node;
    //int key_len;
    int keyval_len;

    args[0].str = NULL;
    args[1].str = NULL;
    cp_str(&args[0].str, key);
    cp_str(&args[1].str, keyval+1);   // Offset by 1 to move past the '"'
    keyval_len = strlen(keyval) - 2;  // Strip off quotation marks
    args[1].str[keyval_len] = '\0';   // 

#if DEBUG
    printf("DEBUG create_key_neq_keyval_node(): key %s\n", key);
    printf("DEBUG create_key_neq_keyval_node(): keyval %s\n", keyval);
    printf("DEBUG create_key_neq_keyval_node(): %s\n", args[0].str);
    printf("DEBUG create_key_neq_keyval_node(): %s\n", args[1].str);
#endif
    node = rules_node_create(args, &rules_node_key_eq_keyval);
    return node;
}

RulesTreeNode* create_key_neq_keyval_node(char* key, char* keyval) {
    RulesTreeNodeChild args[2];
    RulesTreeNode *node;
    //int key_len;
    int keyval_len;

    args[0].str = NULL;
    args[1].str = NULL;
    cp_str(&args[0].str, key);
    cp_str(&args[1].str, keyval+1);   // Offset by 1 to move past the '"'
    keyval_len = strlen(keyval) - 2;  // Strip off quotation marks
    args[1].str[keyval_len] = '\0';   // 

    node = rules_node_create(args, &rules_node_key_neq_keyval);
    return node;
}

RulesTreeNode* create_key_is_empty_node(char* key) {
    RulesTreeNodeChild args[2];
    RulesTreeNode *node;

    args[0].str = NULL;
    args[1].str = NULL;
    cp_str(&args[0].str, key);

    node = rules_node_create(args, &rules_node_key_is_empty);
    return node;
}

RulesTreeNode* create_key_is_filled_node(char* key) {
    RulesTreeNodeChild args[2];
    RulesTreeNode *node;

    args[0].str = NULL;
    args[1].str = NULL;
    cp_str(&args[0].str, key);

    node = rules_node_create(args, &rules_node_key_is_filled);
    return node;
}


RulesTreeNode* create_and_node(RulesTreeNode* arg0, RulesTreeNode* arg1) {
    RulesTreeNodeChild args[2];
    RulesTreeNode *node;

    args[0].node = arg0;
    args[1].node = arg1;
    node = rules_node_create(args, &rules_node_and);
    return node;
}

RulesTreeNode* create_or_node(RulesTreeNode* arg0, RulesTreeNode* arg1) {
    RulesTreeNodeChild args[2];
    RulesTreeNode *node;

    args[0].node = arg0;
    args[1].node = arg1;
    node = rules_node_create(args, &rules_node_or);
    return node;
}

int rules_node_init(RulesTreeNode* node, RulesTreeNodeChild args[],
                        int (*function)NODE_FUNCTION_SIG) {
    node->args[0] = args[0];
    node->args[1] = args[1];
    node->function = function;
    return 1;
}

void print_rules_tree(FILE* fp, RulesTreeNode* node) {
    if (node->function == &rules_node_and || node->function == &rules_node_or) {
        fprintf(fp, "(");
        print_rules_tree(fp, node->args[0].node);
        print_rules_node(fp, node);
        print_rules_tree(fp, node->args[1].node);
        fprintf(fp, ")");
    } else {
        print_rules_node(fp, node);
    }
}

void print_rules_node(FILE* fp, RulesTreeNode* node) {
    fprintf(fp, " ");
    if (node == NULL) {
        fprintf(fp, "NULL\n");
    } else if (node->function == &rules_node_key_is_filled) {
        fprintf(fp, "%s", node->args[0].str);
    } else if (node->function == &rules_node_key_is_empty) {
        fprintf(fp,"!%s", node->args[0].str);
    } else if (node->function == &rules_node_key_eq_key) {
        fprintf(fp,"%s == %s", node->args[0].str, node->args[1].str);
    } else if (node->function == &rules_node_key_neq_key) {
        fprintf(fp,"%s != %s", node->args[0].str, node->args[1].str);
    } else if (node->function == &rules_node_key_eq_keyval) {
        fprintf(fp,"%s == %s", node->args[0].str, node->args[1].str);
    } else if (node->function == &rules_node_key_neq_keyval) {
        fprintf(fp,"%s != %s", node->args[0].str, node->args[1].str);
    } else if (node->function == &rules_node_and) {
        fprintf(fp,"AND");
    } else if (node->function == &rules_node_or) {
        fprintf(fp,"OR");
    }
    fprintf(fp," ");
}

RulesTreeNode* rules_node_create(RulesTreeNodeChild args[], 
                        int (*function)NODE_FUNCTION_SIG) {
    RulesTreeNode *node = NULL;

    node = malloc(sizeof(RulesTreeNode));
    rules_node_init(node, args, function);
    return node;
}


void rules_tree_delete(RulesTreeNode* node) {
    if (node->args[0].node) {
        rules_tree_delete(node->args[0].node);
    } else if (node->args[1].node) {
        rules_tree_delete(node->args[0].node);
    }
    // may need to free other attributes of the node here like the strings
    free(node->args[0].str);
    free(node->args[1].str);
    free(node);

}


int rules_node_eval(struct aFrame* frame, struct Config* cnfg, RulesTreeNode* node) {
    return (node->function(frame, cnfg, node->args));

}




int rules_node_and(struct aFrame* frame, struct Config* cnfg, 
                   RulesTreeNodeChild args[]) {

    return rules_node_eval(frame, cnfg, args[0].node) 
           && rules_node_eval(frame, cnfg, args[1].node);
}


int rules_node_or(struct aFrame* frame, struct Config* cnfg, 
                  RulesTreeNodeChild args[]) {
    return rules_node_eval(frame, cnfg, args[0].node) 
           || rules_node_eval(frame, cnfg, args[1].node);
}


int rules_node_key_eq_key(struct aFrame* frame, struct Config* cnfg,
                          RulesTreeNodeChild args[]) {
    /* This is the function used for exception rules of the form:
     * [Key1] == [Key2] */

    char* keyval0 = getVal(frame, args[0].str, cnfg);
    char* keyval1 = getVal(frame, args[1].str, cnfg);
    if (!keyval0 || !keyval1) { return 0; }

    return (0 == strcmp(keyval0, keyval1));
}


int rules_node_key_neq_key(struct aFrame* frame, struct Config* cnfg,
                           RulesTreeNodeChild args[]) {
    char *keyval0;
    char *keyval1;

    /* This is the function used for exception rules of the form:
     * [Key1] != [Key2] */
    keyval0 = getVal(frame, args[0].str, cnfg);
    keyval1 = getVal(frame, args[1].str, cnfg);
    if (!keyval0 || !keyval1) { return 0; }

    return (0 != strcmp(keyval0, keyval1));
}


int rules_node_key_eq_keyval(struct aFrame* frame, struct Config* cnfg,
                             RulesTreeNodeChild args[]) {
    /* This is the function used for exception rules of the form:
     * [Key1] == "Value" */
    char *keyval0;
    char *keyval1;

    keyval0 = getVal(frame, (char*) args[0].str, cnfg);
    keyval1 = args[1].str;
    if (!keyval0 || !keyval1) { return 0; }
    
    return ( 0 == strcmp(keyval0, keyval1));
}
    


int rules_node_key_neq_keyval(struct aFrame* frame, struct Config* cnfg,
                              RulesTreeNodeChild args[]) {
    /* This is the function used for exception rules of the form:
     * [Key1] != "Value" */
    char *keyval0;
    char *keyval1;

    keyval0 = getVal(frame, (char*) args[0].str, cnfg);
    keyval1 = args[1].str;
    if (!keyval0 || !keyval1) { return 0; }
    return ( 0 != strcmp(keyval0, keyval1));
}

int rules_node_key_is_empty(struct aFrame* frame, struct Config* cnfg,
                            RulesTreeNodeChild args[]) {
   /* This is the function used for exception rules of the form:
    * ![Key] */
    char *key;
    char *val;
     key = (char*) args[0].str;
     val = getVal(frame, key, cnfg);
    return (val == NULL);
    
}

int rules_node_key_is_filled(struct aFrame* frame, struct Config* cnfg,
                             RulesTreeNodeChild args[]) {
   /* This is the function used for exception rules of the form:
    * [Key] */
    return (!rules_node_key_is_empty(frame, cnfg, args));
}
