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

#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
static int wp_num = NR_WP;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
		wp_pool[i].expr = NULL;
		wp_pool[i].value = 0;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char *exp, word_t valu) {
	if (wp_num == 0) {
		printf("\033[31mNo available watchpoints!\033[0m\n");
		return NULL;
	}
	WP *p = head;
	head = free_;
	free_ = free_->next;
	head->next = p;
	wp_num--;
	head->expr = strdup(exp);
	head->value = valu;
	printf("Add a new watchpoint. %d\n", head->NO);
	return head;
}

void free_wp(WP *wp) {
	if (wp == NULL) return;	
	if (wp == head) {
		head = head->next;
	} else {
		WP *r = head;
	    while (r->next != wp) {
			r = r->next;
			}
		r->next = wp->next;
		}
	wp->next = free_;
	free_ = wp;
	if (wp->expr != NULL) {
        free(wp->expr);
        wp->expr = NULL;
    }
	wp->value = 0;
	wp_num++;
}

void scan_watchpoints() {
	WP *p = head;
	while (p) {
		bool success = true;
		word_t new_value = expr(p->expr, &success);
		if (p->value != new_value) {
			nemu_state.state = NEMU_STOP;
			p->value = new_value;
			printf("\033[32mWatchpoint %d has been changed , it becomes %ld, program stops.\33[0m\n", p->NO, p->value);
		}
	p = p->next;
	}
}

void print_watchpoints() {
	WP *p = head;
	if (p == NULL) printf("No any watchpoints.\n");
	else {
		while (p) {
			printf("\033[35mWatchpoint \033[33m%d\033[35m : \033[33m%s \033[35mis \033[33m%ld\033[0m\n", p->NO, p->expr, p->value);
			p = p->next;
		}
	}
}

void delete_watchpoints(int num) {
	WP *p = head;
	if (p == NULL) printf("\033[31mThere is no watchpoints.\033[0m\n");
	else {
		while (p) {
			if (p->NO == num) {
				free_wp(p);
			  printf("The watchpoint %d has been deleted.\n", num);
			  break;
			}
			p = p->next;
		}
	}
}

