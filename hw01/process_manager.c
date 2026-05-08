#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

#define INPUT_CAPACITY 128
#define TOKEN_DELIMS " \t\r\n"

enum process_state {
	PROCESS_NEW = 0,
	PROCESS_READY,
	PROCESS_RUNNING,
	PROCESS_BLOCKED,
	PROCESS_TERMINATED,
};

struct process {
	int pid;
	char name[32];
	enum process_state state;
	struct list_head node;
};

struct process_manager {
	int next_pid;
	struct list_head new_list;
	struct list_head ready_list;
	struct list_head running_list;
	struct list_head blocked_list;
	struct list_head terminated_list;
};

static const char *process_state_name(enum process_state state)
{
	switch (state) {
	case PROCESS_NEW:
		return "NEW";
	case PROCESS_READY:
		return "READY";
	case PROCESS_RUNNING:
		return "RUNNING";
	case PROCESS_BLOCKED:
		return "BLOCKED";
	case PROCESS_TERMINATED:
		return "TERMINATED";
	default:
		return "UNKNOWN";
	}
}

static void process_manager_init(struct process_manager *manager)
{
	manager->next_pid = 1;
	INIT_LIST_HEAD(&manager->new_list);
	INIT_LIST_HEAD(&manager->ready_list);
	INIT_LIST_HEAD(&manager->running_list);
	INIT_LIST_HEAD(&manager->blocked_list);
	INIT_LIST_HEAD(&manager->terminated_list);
}

static struct list_head *process_manager_state_head(struct process_manager *manager,
						    enum process_state state)
{
	switch (state) {
	case PROCESS_NEW:
		return &manager->new_list;
	case PROCESS_READY:
		return &manager->ready_list;
	case PROCESS_RUNNING:
		return &manager->running_list;
	case PROCESS_BLOCKED:
		return &manager->blocked_list;
	case PROCESS_TERMINATED:
		return &manager->terminated_list;
	default:
		return NULL;
	}
}

static struct process *find_process_in_list(struct list_head *head, int pid)
{
	struct process *process;

	list_for_each_entry(process, head, node) {
		if (process->pid == pid)
			return process;
	}

	return NULL;
}

static struct process *process_manager_find(struct process_manager *manager,
					    int pid,
					    enum process_state *state_out)
{
	struct process *process;

	process = find_process_in_list(&manager->new_list, pid);
	if (process != NULL) {
		if (state_out != NULL)
			*state_out = PROCESS_NEW;
		return process;
	}

	process = find_process_in_list(&manager->ready_list, pid);
	if (process != NULL) {
		if (state_out != NULL)
			*state_out = PROCESS_READY;
		return process;
	}

	process = find_process_in_list(&manager->running_list, pid);
	if (process != NULL) {
		if (state_out != NULL)
			*state_out = PROCESS_RUNNING;
		return process;
	}

	process = find_process_in_list(&manager->blocked_list, pid);
	if (process != NULL) {
		if (state_out != NULL)
			*state_out = PROCESS_BLOCKED;
		return process;
	}

	process = find_process_in_list(&manager->terminated_list, pid);
	if (process != NULL) {
		if (state_out != NULL)
			*state_out = PROCESS_TERMINATED;
		return process;
	}

	return NULL;
}

static bool parse_pid(const char *token, int *pid_out)
{
	char *end = NULL;
	long value;

	if (token == NULL) {
		printf("error: missing pid\n");
		return false;
	}

	value = strtol(token, &end, 10);
	if (token[0] == '\0' || end == NULL || *end != '\0') {
		printf("error: invalid pid '%s'\n", token);
		return false;
	}

	if (value <= 0 || value > INT_MAX) {
		printf("error: pid out of range '%s'\n", token);
		return false;
	}

	*pid_out = (int)value;
	return true;
}

static bool ensure_no_extra_args(char *extra)
{
	if (extra != NULL) {
		printf("error: too many arguments\n");
		return false;
	}

	return true;
}

static bool process_manager_create(struct process_manager *manager, const char *name)
{
	struct process *process;
	size_t name_len = strlen(name);

	if (name_len == 0) {
		printf("error: empty process name\n");
		return false;
	}

	if (name_len >= sizeof(process->name)) {
		printf("error: process name too long (max %zu characters)\n",
		       sizeof(process->name) - 1U);
		return false;
	}

	process = malloc(sizeof(*process));
	if (process == NULL) {
		printf("error: out of memory\n");
		return false;
	}

	process->pid = manager->next_pid++;
	memcpy(process->name, name, name_len + 1U);
	process->state = PROCESS_NEW;
	INIT_LIST_HEAD(&process->node);
	list_add_tail(&process->node, &manager->new_list);

	printf("created pid=%d name=%s state=%s\n",
	       process->pid,
	       process->name,
	       process_state_name(process->state));
	return true;
}

static bool process_manager_transition_pid(struct process_manager *manager,
					   int pid,
					   enum process_state from,
					   enum process_state to)
{
	struct process *process;
	struct list_head *target_head;
	enum process_state current_state;

	process = process_manager_find(manager, pid, &current_state);
	if (process == NULL) {
		printf("error: pid %d not found\n", pid);
		return false;
	}

	if (current_state != from) {
		printf("error: pid %d is in %s, expected %s\n",
		       pid,
		       process_state_name(current_state),
		       process_state_name(from));
		return false;
	}

	target_head = process_manager_state_head(manager, to);
	if (target_head == NULL) {
		printf("error: invalid target state\n");
		return false;
	}

	list_move_tail(&process->node, target_head);
	process->state = to;
	printf("pid=%d moved %s -> %s\n",
	       process->pid,
	       process_state_name(from),
	       process_state_name(to));
	return true;
}

static struct process *process_manager_running(struct process_manager *manager)
{
	if (list_empty(&manager->running_list)) {
		printf("error: no running process\n");
		return NULL;
	}

	return list_first_entry(&manager->running_list, struct process, node);
}

static bool process_manager_dispatch(struct process_manager *manager)
{
	struct process *process;

	if (!list_empty(&manager->running_list)) {
		process = list_first_entry(&manager->running_list, struct process, node);
		printf("error: pid %d is already running\n", process->pid);
		return false;
	}

	if (list_empty(&manager->ready_list)) {
		printf("error: no ready process to dispatch\n");
		return false;
	}

	process = list_first_entry(&manager->ready_list, struct process, node);
	list_move_tail(&process->node, &manager->running_list);
	process->state = PROCESS_RUNNING;
	printf("pid=%d moved READY -> RUNNING\n", process->pid);
	return true;
}

static bool process_manager_move_running(struct process_manager *manager,
					 enum process_state to)
{
	struct process *process = process_manager_running(manager);
	struct list_head *target_head;

	if (process == NULL)
		return false;

	target_head = process_manager_state_head(manager, to);
	if (target_head == NULL) {
		printf("error: invalid target state\n");
		return false;
	}

	list_move_tail(&process->node, target_head);
	printf("pid=%d moved RUNNING -> %s\n",
	       process->pid,
	       process_state_name(to));
	process->state = to;
	return true;
}

static size_t free_process_list(struct list_head *head)
{
	struct process *process;
	struct process *next;
	size_t freed = 0;

	list_for_each_entry_safe(process, next, head, node) {
		list_del(&process->node);
		free(process);
		freed++;
	}

	return freed;
}

static void process_manager_reap(struct process_manager *manager)
{
	size_t reaped = free_process_list(&manager->terminated_list);

	printf("reaped %zu terminated process%s\n",
	       reaped,
	       reaped == 1U ? "" : "es");
}

static void print_state_list(const char *label, struct list_head *head)
{
	struct process *process;

	printf("%-11s (%zu):", label, list_count_nodes(head));
	if (list_empty(head)) {
		printf(" <empty>\n");
		return;
	}

	list_for_each_entry(process, head, node)
		printf(" [%d:%s]", process->pid, process->name);

	printf("\n");
}

static void process_manager_show(struct process_manager *manager)
{
	print_state_list("NEW", &manager->new_list);
	print_state_list("READY", &manager->ready_list);
	print_state_list("RUNNING", &manager->running_list);
	print_state_list("BLOCKED", &manager->blocked_list);
	print_state_list("TERMINATED", &manager->terminated_list);
}

static void process_manager_destroy(struct process_manager *manager)
{
	(void)free_process_list(&manager->new_list);
	(void)free_process_list(&manager->ready_list);
	(void)free_process_list(&manager->running_list);
	(void)free_process_list(&manager->blocked_list);
	(void)free_process_list(&manager->terminated_list);
}

static void print_help(void)
{
	puts("commands:");
	puts("  create <name>  - create a process in NEW");
	puts("  admit <pid>    - move NEW -> READY");
	puts("  dispatch       - move the next READY process -> RUNNING");
	puts("  yield          - move RUNNING -> READY");
	puts("  block          - move RUNNING -> BLOCKED");
	puts("  wake <pid>     - move BLOCKED -> READY");
	puts("  finish         - move RUNNING -> TERMINATED");
	puts("  reap           - free all TERMINATED processes");
	puts("  show           - print all state lists");
	puts("  help           - print this help");
	puts("  quit           - exit the program");
}

int main(void)
{
	struct process_manager manager;
	char input[INPUT_CAPACITY];

	process_manager_init(&manager);
	print_help();

	for (;;) {
		char *command;
		char *arg1;
		char *arg2;
		int pid;

		printf("pm> ");
		if (fgets(input, sizeof(input), stdin) == NULL) {
			printf("\n");
			break;
		}

		command = strtok(input, TOKEN_DELIMS);
		if (command == NULL)
			continue;

		arg1 = strtok(NULL, TOKEN_DELIMS);
		arg2 = strtok(NULL, TOKEN_DELIMS);

		if (strcmp(command, "create") == 0) {
			if (arg1 == NULL) {
				printf("error: create requires a name\n");
				continue;
			}
			if (!ensure_no_extra_args(arg2))
				continue;
			(void)process_manager_create(&manager, arg1);
			continue;
		}

		if (strcmp(command, "admit") == 0) {
			if (!parse_pid(arg1, &pid) || !ensure_no_extra_args(arg2))
				continue;
			(void)process_manager_transition_pid(&manager,
							 pid,
							 PROCESS_NEW,
							 PROCESS_READY);
			continue;
		}

		if (strcmp(command, "dispatch") == 0) {
			if (!ensure_no_extra_args(arg1))
				continue;
			(void)process_manager_dispatch(&manager);
			continue;
		}

		if (strcmp(command, "yield") == 0) {
			if (!ensure_no_extra_args(arg1))
				continue;
			(void)process_manager_move_running(&manager, PROCESS_READY);
			continue;
		}

		if (strcmp(command, "block") == 0) {
			if (!ensure_no_extra_args(arg1))
				continue;
			(void)process_manager_move_running(&manager, PROCESS_BLOCKED);
			continue;
		}

		if (strcmp(command, "wake") == 0) {
			if (!parse_pid(arg1, &pid) || !ensure_no_extra_args(arg2))
				continue;
			(void)process_manager_transition_pid(&manager,
							 pid,
							 PROCESS_BLOCKED,
							 PROCESS_READY);
			continue;
		}

		if (strcmp(command, "finish") == 0) {
			if (!ensure_no_extra_args(arg1))
				continue;
			(void)process_manager_move_running(&manager, PROCESS_TERMINATED);
			continue;
		}

		if (strcmp(command, "reap") == 0) {
			if (!ensure_no_extra_args(arg1))
				continue;
			process_manager_reap(&manager);
			continue;
		}

		if (strcmp(command, "show") == 0) {
			if (!ensure_no_extra_args(arg1))
				continue;
			process_manager_show(&manager);
			continue;
		}

		if (strcmp(command, "help") == 0) {
			if (!ensure_no_extra_args(arg1))
				continue;
			print_help();
			continue;
		}

		if (strcmp(command, "quit") == 0) {
			if (!ensure_no_extra_args(arg1))
				continue;
			break;
		}

		printf("error: unknown command '%s'\n", command);
	}

	process_manager_destroy(&manager);
	return 0;
}
