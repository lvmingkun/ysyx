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

#include <common.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "monitor/sdb/sdb.h"

#define MAX_LINE_LENGTH 256

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {

  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

/* test expr. */
  FILE *file;
	char line[MAX_LINE_LENGTH] = {};
	// char value[12] = {};
	int32_t value = 0;
	char expre[MAX_LINE_LENGTH - 12] = {};
	file = fopen("tools/gen-expr/input", "r");
	if (file == NULL) {
		perror("Failed to open file");
    return 1;
	}
	int success_num = 0;

	while (fgets(line, sizeof(line), file) != NULL) {
		line[strcspn(line, "\n")] = '\0';
		if (sscanf(line, "%d %[^\n]", &value, expre) == 2) {
			bool success = false;
			int32_t valid_value = expr(expre, &success);
			if (success && valid_value == value) {
				success_num++;
			} else {
				printf("\033[31mThe expression %s calculate unsuccessfully\033[0m\n", expre);
			}	
		} else {
			printf("\033[31mLine format incorrect: %s\033[0m\n", line);
		}
	}
	printf("\033[1mPass %d / 10158  expressions successfully\033[0m\n", success_num); 
  printf("Accuracy is  \033[1;35m%d %% \033[0m\n", success_num * 100 / 10159);
	fclose(file);

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
