#include "rules_grammar.h"
#include "rules_stack.h"
#include <stdlib.h>
#include <stdio.h>
#define DEBUG 0

/*****************************************************
   Exception Rules Grammar is defined as follows
 
   expr -> term { OR term }
   term -> term1 { AND term1 }
   term1 -> KEY | NOT KEY | 
            KEY EQ KEY | KEY NEQ KEY | 
            KEY EQ KEYVAL | KEY NEQ KEYVAL |
            LPAREN expr RPAREN

 The implementation of the parser is TOP-DOWN
 *****************************************************/


/***************** GLOBAL VARIABLES *********************/
int global_num_tokens;
int global_current_token;
RuleToken global_rule_tokens[MAX_NUM_TOKENS];
RuleToken* global_next;
RulesStack global_rule_stack; // stack used throughout parse

/* Forward Declarations */
int compile_regex();
int expr(void);
int term(void);
int term1(void);


/******************************************************************************
 * scan() - Advances the lexer to the next token
 *****************************************************************************/
void scan(void) {

    while (global_current_token < global_num_tokens) {
        global_next = &global_rule_tokens[global_current_token++];
        return;
    }
    global_next = NULL;

}

int expr(void) {
    int result;
    RulesTreeNode	*child1,
    			*child0,
    			*node;

#if DEBUG
    printf("entering expr\n");
#endif
    result = term();
    if (result == PARSE_ERROR) { return result; }

    while (global_next && global_next->value == TT_OR) {
        scan();
        result = term();
        if (result == PARSE_ERROR) { return result; }

        // pop off two children (arguments to AND)
        child1 = pop(&global_rule_stack);
        if (child1 == NULL) { 
#if DEBUG
            printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
            return PARSE_ERROR;
        }
        child0 = pop(&global_rule_stack);
        if (child0 == NULL) { 
#if DEBUG
            printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
            return PARSE_ERROR;
        }

        // create OR node
        node = create_or_node(child0, child1);

        // push OR node onto stack
        push(&global_rule_stack, node);

#if DEBUG
        printf("stack: OR\n");
#endif
    }
    return PARSE_OK;
}


int term(void) {
    int result;
    RulesTreeNode *child0,
		   *child1,
        	   *node;
#if DEBUG
    printf("entering term\n");
#endif
    result = term1();
    if (result == PARSE_ERROR) { return result; }

    // Iterate while we have an AND conjoining two terms
    while (global_next && global_next->value == TT_AND) {
        scan();
        result = term1();
        if (result == PARSE_ERROR) { return result; }

        child0 = NULL;
        child1 = NULL;

        // pop off two children (arguments to AND)
        child1 = pop(&global_rule_stack);
        if (child1 == NULL) { 
#if DEBUG
            printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
            return PARSE_ERROR;
        }
        child0 = pop(&global_rule_stack);
        if (child0 == NULL) { 
#if DEBUG
            printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
            return PARSE_ERROR;
        }

        // create AND node
        node = create_and_node(child0, child1);

        // push AND node onto stack
        push(&global_rule_stack, node);

#if DEBUG
        printf("stack: AND\n");
#endif

    }
    return PARSE_OK;
}

int term1(void) {
    int result;
    RuleToken* term_args[2];
    RulesTreeNode* node = NULL;

#if DEBUG
    printf("entering term1\n");
#endif
    switch (global_next->value) {
        case TT_KEY:
            term_args[0] = global_next;
            scan();
            if (global_next == NULL) {
#if DEBUG
                printf("stack: key_is_filled %s\n", term_args[0]->contents);
#endif

                // create key_is_filled node
                node = create_key_is_filled_node(term_args[0]->contents);

                // put node on the stack
                push(&global_rule_stack, node);
                return PARSE_OK;
            }

            switch (global_next->value) {
                case TT_EQ:
                    scan();
                    if (global_next == NULL) {
                        // Parse Error
#if DEBUG
                        printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
                        return PARSE_ERROR;
                    }
                    
                    term_args[1] = global_next;
                    switch (global_next->value) { 
                        case TT_KEY: 
                            // create node for key_eq_key and put it on the stack
#if DEBUG
                            printf("stack: key_eq_key %s == %s\n", 
                                   term_args[0]->contents, 
                                   term_args[1]->contents);
#endif

                            // create key_eq_key node
                            node = create_key_eq_key_node(term_args[0]->contents,
                                                          term_args[1]->contents);

                            // put node on the stack
                            //print_rules_node(node);
                            push(&global_rule_stack, node);

                            scan();
                            break;

                        case TT_KEYVAL:
                            // create node for key_eq_keyval and put it on the stack
#if DEBUG
                            printf("stack: key_eq_keyval %s == %s\n", 
                                   term_args[0]->contents, 
                                   term_args[1]->contents);
#endif

                            // create key_eq_keyval node
                            node = create_key_eq_keyval_node(term_args[0]->contents,
                                                             term_args[1]->contents);
                            // put node on the stack
                            //print_rules_node(node);
                            push(&global_rule_stack, node);

                            // Advance lexer
                            scan();
                            break;
                        default:
                            // parse error
#if DEBUG
                            printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
                            return PARSE_ERROR;
                            break;
                    }
                    break;

                case TT_NEQ:
                    // Advance lexer
                    scan();
                    if (global_next == NULL) {
                        // Parse Error
#if DEBUG
                        printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
                        return PARSE_ERROR;
                    }
               
                    term_args[1] = global_next;
                    switch (global_next->value) { 
                        case TT_KEY: 
                            // create node for key_neq_key and put it on the stack
#if DEBUG
                            printf("stack: key_neq_key %s != %s\n", 
                                   term_args[0]->contents, 
                                   term_args[1]->contents);
#endif
 
                            // create key_neq_key node
                            node = create_key_neq_key_node(term_args[0]->contents,
                                                           term_args[1]->contents);
                            // put node on the stack
                            push(&global_rule_stack, node);

                            // Advance lexer
                            scan();
                            break;

                        case TT_KEYVAL:
                            // create node for key_neq_keyval and put it on the stack
#if DEBUG
                            printf("stack: key_neq_keyval %s == %s\n", 
                                   term_args[0]->contents, 
                                   term_args[1]->contents);
#endif
                            // create key_neq_keyval node
                            node = create_key_neq_keyval_node(term_args[0]->contents,
                                                              term_args[1]->contents);
                            // put node on the stack
                            push(&global_rule_stack, node);

                            // Advance lexer
                            scan();
                            break;
                        default:
                            // parse error
#if DEBUG
                            printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
                            return PARSE_ERROR;
                            break;
                    }
                    break;
                default:
                    // we assume this is a key filled point, the parse may 
                    // break later
#if DEBUG
                    printf("stack: key_is_filled %s\n", term_args[0]->contents);
#endif

                    // create key_is_filled node
                    node = create_key_is_filled_node(term_args[0]->contents);

                    // put node on the stack
                    push(&global_rule_stack, node);
    
            } // end level 2 switch
            break;

        case TT_NOT:
            // Advance lexer
            scan();
            if (global_next == NULL) {
                // Parse Error
#if DEBUG
                printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
                return PARSE_ERROR;
            }
    
            switch (global_next->value) {
                case TT_KEY:
                    // create node for key_is_empty, put it on the stack
#if DEBUG
                    printf("stack: key_is_empty !%s\n", global_next->contents);
#endif

                    // create key_is_empty node
                    node = create_key_is_empty_node(global_next->contents);

                    // put node on the stack
                    push(&global_rule_stack, node);
        
                    // Advance Lexer
                    scan();
                    break;
                default:
                    // parse error
#if DEBUG
                    printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
                    return PARSE_ERROR;
                    break;
            }
            break;


        case TT_LPAREN:
            // Advance Lexer
            scan();
            result = expr();
            if (global_next == NULL || result == PARSE_ERROR) {
                // Parse error
#if DEBUG
                printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
                delete_stack(&global_rule_stack);
                return PARSE_ERROR;
            }
            switch (global_next->value) {
                case TT_RPAREN:
                    // No need to put anything on the stack as grammar
                    // does this inherently
                    // Advance Lexer
                    scan();
                    break;
                default:
                    // parse error
#if DEBUG
                    printf("Parse Error %s %d\n", __FILE__, __LINE__);
#endif
                    return PARSE_ERROR;
                    break;
            }
            break;
    } 
    return PARSE_OK;
} // end term1()

RulesTreeNode* parse_rule(char* rulestr) {
    int result;
    RulesTreeNode* node = NULL;

    global_current_token = 0;
    // Tokenize Rule
    global_num_tokens = tokenize_rule_string(rulestr, global_rule_tokens);

#if DEBUG
    int tok;
    printf("Rule: %s\n", rulestr);
    for (tok = 0; tok < global_num_tokens; tok++) {
        printf("Token %d: %s %d\n", tok, global_rule_tokens[tok].contents, global_rule_tokens[tok].value);
    }

#endif

    // Parse Rule and create Rule Tree
    // get first token
    scan();

    // parse
    result = expr();

    // Check for bad parse and store rules
    node = NULL;
    if (result == PARSE_ERROR || global_rule_stack.top == NULL || is_stack_empty(&global_rule_stack)) {
        printf("Badly formed rule: %s\n", rulestr);
        exit(1);
    }
    node = pop(&global_rule_stack);

    if (!is_stack_empty(&global_rule_stack)) {
        printf("Badly formed rule: %s\n", rulestr);
        exit(1);
    }
    return node;

}

int tokenize_rule_string(char* str, RuleToken tokens[]) {
     /* Tokenize rule string, 
      * store tokens in passed in array
      * return total number of tokens in the rule
      */

    int offset = 0;
    int n = 0;
    char* strend;

    compile_regex();

    offset = 0;
    n = 0;

    strend = str + strlen(str);
    for ( ; str < strend && offset != TT_UNKNOWN; str += offset) {
        
        offset = get_next_token(str, &tokens[n]);
        n++;
    }
    return n;
}

int get_next_token(char* str, RuleToken* token) {
    int offset = 0;
    size_t no_sub;
    regmatch_t result[100];
    int eflag = 0;
    int len;
    char* contents;

    /* Skip leading whitespace */
    if (str == NULL || *str == '\0') { return TT_UNKNOWN; }
        while (*str == ' ' || *str == '\t' || *str == '\n') {
            str++;
            offset++;
        }

    no_sub = key_re.re_nsub + 1;
    eflag = 0;

    if (regexec(&key_re, str, no_sub, result, eflag) != REG_NOMATCH) {
        token->value = TT_KEY;
    } else if (regexec(&keyval_re, str, no_sub, result, eflag) != REG_NOMATCH) {
        token->value = TT_KEYVAL;
    } else if (regexec(&lparen_re, str, no_sub, result, eflag) != REG_NOMATCH) {
        token->value = TT_LPAREN;
    } else if (regexec(&rparen_re, str, no_sub, result, eflag) != REG_NOMATCH) {
        token->value = TT_RPAREN;
    } else if (regexec(&and_re, str, no_sub, result, eflag) != REG_NOMATCH) {
        token->value = TT_AND;
    } else if (regexec(&or_re, str, no_sub, result, eflag) != REG_NOMATCH) {
        token->value = TT_OR;
    } else if (regexec(&eq_re, str, no_sub, result, eflag) != REG_NOMATCH) {
        token->value = TT_EQ;
    } else if (regexec(&neq_re, str, no_sub, result, eflag) != REG_NOMATCH) {
        token->value = TT_NEQ;
    } else if (regexec(&not_re, str, no_sub, result, eflag) != REG_NOMATCH) {
        token->value = TT_NOT;
    } else {
        token->value = TT_UNKNOWN;
        return TT_UNKNOWN;
    }


    len = result[0].rm_eo - result[0].rm_so + 1;

    /* Copy contents of matched regular expression into token contents */
    contents = malloc(sizeof(char) * len);
    memcpy(contents, str + result[0].rm_so, len);
    contents[len-1] = '\0';
    token->contents = contents;

    offset += result[0].rm_so;
    offset += result[0].rm_eo;
    return offset;

} // end get_next_token()



int compile_regex() { 
   static int compiled = 0;
   int reti;

   /* Compile regular expressions */

   if (compiled) { return(1); }

   /* KEY regular expression - of the form [Key].[Subkey] */
   reti = regcomp(&key_re,  "^[[][_[:alnum:]-]+]([.][[][_[:alnum:]-]+])*", REG_EXTENDED);
   if( reti ){ fprintf(stderr, "Could not compile regex key_re\n"); return(1); }

   /* KEYVAL regular expression - a string literal form "string"*/
   reti = regcomp(&keyval_re,  "^\"[_[:alnum:]-]+\"", REG_EXTENDED);
   if( reti ){ fprintf(stderr, "Could not compile regex keyval_re\n"); return(1); }

   /* LPAREN regular expression*/
   reti = regcomp(&lparen_re,  "^(", 0);
   if( reti ){ fprintf(stderr, "Could not compile regex lparen_re\n"); return(1); }

   /* RPAREN regular expression*/
   reti = regcomp(&rparen_re,  "^)", 0);
   if( reti ){ fprintf(stderr, "Could not compile regex rparen_re\n"); return(1); }

   /* AND regular expression*/
   reti = regcomp(&and_re,  "^AND", 0);
   if( reti ){ fprintf(stderr, "Could not compile regex and_re\n"); return(1); }

   /* OR regular expression*/
   reti = regcomp(&or_re,  "^OR", 0);
   if( reti ){ fprintf(stderr, "Could not compile regex or_re\n"); return(1); }

   /* EQ regular expression*/
   reti = regcomp(&eq_re,  "^==", REG_EXTENDED);
   if( reti ){ fprintf(stderr, "Could not compile regex eq_re\n"); return(1); }

   /* NEQ regular expression*/
   reti = regcomp(&neq_re,  "^!=", REG_EXTENDED);
   if( reti ){ fprintf(stderr, "Could not compile regex neq_re\n"); return(1); }

   /* NOT regular expression*/
   reti = regcomp(&not_re,  "^!", 0);
   if( reti ){ fprintf(stderr, "Could not compile regex not_re\n"); return(1); }

   compiled = 1;

   return 0;
}

