#ifndef VALID_H
#define VALID_H

#include "header.h"

extern int is_valid_command();
extern void sigchld_handler();
extern void handle_background_process(pid_t pid);
extern void add_process(pid_t pid,int bg);
extern void update_process(pid_t pid, char* state);
extern int cmp_process(const void* a, const void* b);
extern int is_command_executable(char *command);
extern int find_alias();
extern int find_func();
extern int bg_pids[100]; // To store background process PIDs
extern char bg_cmd[100][MAX_LEN];

typedef struct Process
{
    char command[100][MAX_LEN];
    pid_t pid;
    char state[15];
    int command_size;
    int bg;
}Process;

extern Process;

extern Process processes[MAX_PROCESSES];

#endif