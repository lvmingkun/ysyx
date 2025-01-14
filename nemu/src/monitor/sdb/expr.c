/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
	TK_NUM,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
	{"-", '-'},
	{"\\*", '*'},
	{"/", '/'},
	{"\\(", '('},
	{"\\)", ')'},
  {"==", TK_EQ},        // equal
	{"[0-9]+", TK_NUM},			// number
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;
  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
					case TK_NOTYPE:
						break;
					case TK_NUM:
						tokens[nr_token].type = rules[i].token_type;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
					 	tokens[nr_token].str[substr_len] = '\0';
						nr_token++;
						break;
          default:
						tokens[nr_token].type = rules[i].token_type;
						nr_token++;
						break;
        }
        break;
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}

bool check_parentheses(int p, int q, bool *qs) {
	int n = 0;
	bool one = false;
	if (tokens[p].type == '(' && tokens[q].type != ')') {
		*qs = false;
		return false;
	}
	while (p <= q) {
		if (tokens[p].type == '(') {
			n++;
			if (!one) one = true;
		}
		if (tokens[q].type == ')') n--;
		p++;
	}
	if (one && n == 0) return true;
	if (!one) return false;
	*qs = false;
	return false;
}

int find_op(int p, int q) {
	int t = p + 1;
	int precedence[128];
	for (int i = 0; i < 128; i++) {
		precedence[i] = 0;
	}
	precedence['+'] = 2;
	precedence['-'] = 2;
	precedence['*'] = 1;
	precedence['/'] = 1;

	while (t < q) {
		if (precedence[tokens[p].type] <= precedence[tokens[t].type]) { 
			p = t;
		}
		t++;
	}
	return p;
}

word_t eval(int p, int q, bool *success) {
	if (p > q) {
		return 0;
	}
	else if (p == q) {
		if (tokens[p].type == TK_NUM) {
			*success = true;
			return atoi(tokens[p].str);
		}
		return 0;
	}
	bool qs = true;
	if (check_parentheses(p, q, &qs) == true) {
		Log("Matching () successfully!\n"); 
		return eval(p + 1, q - 1, success);
	}
	else {
		if (!qs) {
			return 0;
		}
		int op = find_op(p, q);
		bool success1 = false;
		bool success2 = false;
	  word_t val1 = eval(p, op - 1, &success1);
	  word_t val2 = eval(op + 1, q, &success2);
		if (!success2) return 0;
		*success = true;
		int op_type = tokens[op].type;
	  switch (op_type) {
			case '+': if (!success1) return val2;
									return val1 + val2;
			case '-': if (!success1) return -val2;
									return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			default: {
								 *success = false;
								 return 0;
							 }
		}
	}
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
	return eval(0, nr_token - 1, success);
}
