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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define MAX_BUF 65534
#define MAX_PAR 3
#define MAX_REC 5

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int flags = 0;
static int parnum = 0;
static int renum = 0;
static bool zflag = false;
static char nonnull = 'a';

uint32_t choose(uint32_t n) {
	uint32_t random_num = (uint32_t) (rand() % n);
	printf("rec %d choose %d\n", renum, random_num);
	return random_num;
}

static void gen_num() {
	if (nonnull == ')') {
		printf("rec %d, have ) can't gen num, return!\n", renum);
		return;
	}
	int len = rand() % 4 + 1;
	if (flags < MAX_BUF - len - 1) {
		for (int i = 0; i < len; i++) {
			char random_char = (rand() % 10) + '0';
			if ((len > 1) && (i == 0) && (random_char == '0')) {
				random_char = '1';
				printf("0 head,it will add 1 to be %c\n", random_char);
			}
			if (zflag && len == 1 && random_char == '0') {
				random_char = '1';
				printf(" gen 0 but dev 0,add 1 to be %c\n", random_char);
			}	
			buf[flags] = random_char;
			nonnull = buf[flags];
			printf("rec %d, gen num %c\n", renum, buf[flags]);
			flags++;
			zflag = false;
		}
	}
	else {
		printf("rec %d, overflow\n", renum);
		return;
	}
}

static void gen(char par) {
	if (((nonnull == ')' || parnum >= MAX_PAR || flags >= MAX_BUF - 10) && par == '(') || (par == ')' && parnum <= 0)) {
		printf("rec %d want to gen (  or ) but parnum is %d or have ),return\n",renum, parnum);
		return;
	}
	else {
		if (flags < MAX_BUF) {
			if (par == '(') parnum++;
			else parnum--;
			buf[flags] = par;
			nonnull = buf[flags];
			printf("rec %d gen %c\n", renum, buf[flags]);
			flags++;
		}
		else {
			printf("rec %d overflow, return\n", renum);
			return;
		}
	}
}

static void gen_rand_op() {
	char sign[4] = {'+', '-', '*', '/'};
	if (flags > 0 && flags < MAX_BUF - 10) {
		buf[flags] = sign[rand() % 4];
		nonnull = buf[flags];
		printf("rec %d gen op %c\n", renum, buf[flags]);
		if (buf[flags] == '/') zflag = true;
		flags++;
	}
	else {
		printf("rec %d want to gen op but overflow or null\n", renum);
		return;
	}
}

static void gen_rand_expr() {
	renum++;
	if (choose(6) == 1 && flags < MAX_BUF) {
			buf[flags] = ' ';
			printf("gen %c\n", buf[flags]);
		  flags++;
	}
	else printf("so no gen     \n");
	if (renum >= MAX_REC) {
		printf("rec has max %d\n", renum);
		if (nonnull < '0' || nonnull > '9') gen_num();
		else return;
	}
	else {
    switch (choose(3)) {
			case 0: gen_num(); break;
			case 1: gen('('); gen_rand_expr(); gen(')'); break;
			default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
	  }
	}
}

int main(int argc, char *argv[]) {
  int seed = time(NULL);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
    buf[flags] = '\0';
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Wall -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
		memset(buf, 0, sizeof(buf));
		memset(code_buf, 0, sizeof(code_buf));
		renum = 0;
		flags = 0;
		zflag = false;
		nonnull = 'a';
  }
  return 0;
}
