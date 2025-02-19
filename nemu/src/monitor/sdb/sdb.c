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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <stdlib.h>
#include <ctype.h>
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args) {
  if (args == NULL) {
		cpu_exec(1);
	}
	else {
		int n = atoi(args);
		if (n <= 0) {
			printf("\033[31mPlease choose an integer(>0) as your choice!\033[0m\n");
			}
		else {
			cpu_exec(n);
		}
	}
	return 0;
}

static int cmd_info(char *args) {
	if (args[0] == 'r') {
		isa_reg_display();
	}
	else if (args[0] == 'w') {
		print_watchpoints();
	}
	else {
		printf("\033[31mYou should choose 'r' or 'w' as your option!\033[0m\n");
	}
	return 0;
}

static int cmd_x(char *args) {
	char *token1 = strtok(args, " ");
	if (token1 == NULL) {
    printf("\033[31mPlease choose an integer(>0) as your choice!\033[0m\n");
		return 0;
	}
	else {
		char *token2 = strtok(NULL, " ");
		if (token2 == NULL) {
			printf("\033[31mYou should choose a hexadecimal as your option!\033[0m\n");
			return 0;
		}
		else {
			int n = atoi(token1);
			if (token2[0] == '0' && (token2[1] == 'X' || token2[1] == 'x')) {
				token2+=2;
			}
			vaddr_t addr = strtol(token2, NULL, 16);
			if (n <= 0 || addr < 0 || addr > 0x07ffffff) {
				printf("\033[31mInvalid arguments: N should be positive, EXPR should be a valid address!\033[0m\n");
				return 0;
			}
			else {
				printf("\033[1mStarting from 0x%lx to read %d addresses' information:\033[0m\n", addr, n);
				for (int i = 0; i < n; i++) {
					if (i % 5 == 0)
						printf("\n");
					word_t data = vaddr_read(addr + i * 4 + CONFIG_MBASE, 4);
					printf("\033[33m0x%08lx\033[0m\t", data);
				}
			}
			printf("\n");
		}
	}
	return 0;
}

static int cmd_p(char *args) {
	bool success = false;
	word_t value = expr(args, &success);
  if (success == false) {
		printf("\033[31mSorry, can't calculate the expression, please try to change format!\033[0m\n");
	}
	else {
	printf("\n\033[35mExpression's result is \033[0m\033[1;36m%ld\033[0m\n", value);
	}	
	return 0;
}

static int cmd_w(char *args) {
  if (args == NULL) printf("\033[31mPlease choose an expression as your choice!\033[0m\n");
  else {
    bool success = false;
	  word_t value = expr(args, &success);
    if (success == false) {
      printf("\033[31mSorry, can't calculate the expression, please try to change format!\033[0m\n");
      }
	  else {
      new_wp(args, value);
    }
  }
	return 0;
}

static int cmd_d(char *args) {
	if (args == NULL) {
		printf("\033[31mPlease choose an integer as your choice!\033[0m\n");cpu_exec(1);
	}
	else {
		int n = atoi(args);
		delete_watchpoints(n);
	}
	return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
	{ "si", "Execute N instructions, 'N' is optional as a number", cmd_si },
	{ "info", "Print registers' status(r) or watchpoints' information(w), 'SUBCMD' is optional as 'r' or 'w'", cmd_info },
	{ "x", "Print N consecutive 4-bytes starting addresses from the result of EXPR in hex", cmd_x },
	{ "p", "Print EXPR's value", cmd_p },
	{ "w", "The program will stop if EXPR changes", cmd_w },
	{ "d", "Delete the watchpoint:N", cmd_d }

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
