#ifndef __RULES_GRAMMAR_H__
#define __RULES_GRAMMAR_H__

/****************************************************************************
 * rules_grammar.h
 *
 * function and variable definitions used for parsing the exception rules
 * in a task file
 ***************************************************************************/

#include "rules.h"
#ifdef WIN
#include <windows.h>
#endif
#include <regex.h>
#include <string.h>

#define TT_KEY     257
#define TT_KEYVAL  258
#define TT_AND     259
#define TT_OR      260
#define TT_EQ      261
#define TT_NEQ     262
#define TT_NOT     263
#define TT_LPAREN  264
#define TT_RPAREN  265
#define TT_UNKNOWN -1
#define PARSE_OK      0
#define PARSE_ERROR  -1

#define MAX_NUM_TOKENS 128

typedef int   rules_tokenval_t;
typedef char* rules_tokenstr_t;

// Structure for representing a token of a rule string
typedef struct RuleToken {
    rules_tokenval_t value;
    rules_tokenstr_t contents;
} RuleToken;

/* Regular expression variables */
regex_t key_re, keyval_re, lparen_re, rparen_re, 
        and_re, or_re, eq_re, neq_re, not_re, whitespace_re;

int tokenize_rule_string(char* str, RuleToken tokens[]);
int get_next_token(char* str, RuleToken* token);

/* parses string into exception rule tree and returns the top node */
RulesTreeNode* parse_rule(char* rulestr);

/* function for advancing to the next token */
void scan(void);


#endif
