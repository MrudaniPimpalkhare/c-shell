#include "signals.h"
#include "valid.h"
#include "ping.h"

void handle_sigint(int sig)
{
    for (int i = 0; i < proc_count; i++)
    {
        if ((strcmp(processes[i].state, "running") == 0) && processes[i].bg == 0)
        {
            ping(processes[i].pid, SIGINT);
            update_process(processes[i].pid, "exited");
        }
    }
}

void handle_sigtstp(int sig)
{
    if (fg_process_pid != -1)
    {
        killpg(getpgid(fg_process_pid), SIGTSTP);
        for (int i = 0; i < proc_count; i++)
        {
            if (processes[i].pid == fg_process_pid)
            {
                processes[i].bg = 1;
                update_process(fg_process_pid,"stopped");
                break;
            }
        }
        setpgid(fg_process_pid, fg_process_pid);
        printf(BLUE "\nStopped [%d]\n" RESET, fg_process_pid);
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(STDIN_FILENO, getpgrp());
        signal(SIGTTOU, SIG_DFL);

        fg_process_pid = -1;
    }
    fflush(stdout);
}

void bg_to_fg(pid_t pid)
{
    int found = 0;
    for (int i = 0; i, proc_count; i++)
    {
        if (processes[i].bg == 1 && processes[i].pid == pid)
        {
            found = 1;
            fg_process_pid = processes[i].pid;
            processes[i].bg = 0;
            int rc = fork();
            if (rc < 0)
            {
                perror("fork");
                return;
            }
            else if (rc == 0)
            {
                kill(fg_process_pid, SIGCONT);
                exit(0);
            }
            else
            {
                int status;
                if (waitpid(fg_process_pid, &status, WUNTRACED) == -1)
                {
                    perror("waitpid failed");
                    break;
                }

                if (WIFSTOPPED(status))
                {
                    printf(GREEN "Process %d stopped. Moving it back to background.\n" RESET, pid);
                    processes[i].bg = 1;
                }
                else if (WIFEXITED(status) || WIFSIGNALED(status))
                {
                    printf(GREEN "Process %d finished.\n" RESET, pid);
                    strcpy(processes[i].state, "exited");
                }
                fg_process_pid = -1;
                break;
            }
        }
    }
    if (!found)
    {
        printf(RED "No such process found\n" RESET);
    }
}

void run_bg(pid_t pid)
{
    int found = 0;
    for (int i = 0; i < proc_count; i++)
    {
        if (processes[i].pid == pid)
        {
            if (processes[i].bg == 1)
            {
                if (strcmp(processes[i].state, "stopped") == 0)
                {
                    found = 1;
                    kill(pid, SIGCONT);
                    handle_background_process(pid);
                    update_process(pid, "running");
                    break;
                }
            }
        }
    }
    if (found == 0)
    {
        printf(RED "no such process running in the background\n" RESET);
    }
}