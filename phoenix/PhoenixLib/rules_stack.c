#include "rules_stack.h"
#include <stdio.h>
#include <stdlib.h>


void push(RulesStack* pStack, STACK_ELEM_TYPE new_elem) {
    RulesStackNode* new_node;

    new_node = malloc(sizeof(RulesStackNode));
    if (pStack != NULL) {
        new_node->element = new_elem;
        new_node->next = pStack->top;
        pStack->top = new_node;
        return;
    }
}

STACK_ELEM_TYPE pop(RulesStack* pStack) {
    RulesStackNode* curr_node = NULL;
    STACK_ELEM_TYPE pop_node = NULL;

    if (pStack != NULL) {
        curr_node = pStack->top;
        if (curr_node != NULL) {
            pStack->top = curr_node->next; 

            pop_node = curr_node->element;
            free(curr_node);
            return pop_node;
        }
    }
    return (STACK_ELEM_TYPE) NULL;
}

void delete_stack(RulesStack* pStack) {
    while (pStack->top != NULL) {
        pop(pStack);
    }
}

int is_stack_empty(RulesStack* pStack) {
   return (pStack && pStack->top == NULL);
}
