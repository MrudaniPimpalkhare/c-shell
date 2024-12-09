#include "header.h"

void handle_sigint(int sig);
void handle_sigtstp(int sig);
void push_to_background(pid_t pid);
void handle_ctrl_d();
void handle_fg_process(pid_t pid);
void bg_to_fg(pid_t pid);
void run_bg(pid_t pid);