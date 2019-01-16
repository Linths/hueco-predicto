#ifndef __RULES_STACK_H__
#define __RULES_STACK_H__

#include "rules.h"

#define STACK_ELEM_TYPE RulesTreeNode*

typedef struct rules_stack_node_def {
    STACK_ELEM_TYPE         element;
    struct rules_stack_node_def*   next;
} RulesStackNode;

typedef struct stack_def {
    RulesStackNode* top;
} RulesStack;

void push(RulesStack* pStack, STACK_ELEM_TYPE new_elem);

STACK_ELEM_TYPE pop(RulesStack* pStack);

RulesStack* create_stack(void);
void delete_stack(RulesStack* pStack);

int is_stack_empty(RulesStack* pStack);

#endif
