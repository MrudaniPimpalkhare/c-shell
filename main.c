#include "header.h"
#include "display.h"
#include "log.h"
#include "tokenise.h"
#include "valid.h"
#include "hop.h"
#include "signals.h"

void clear_log()
{
    snprintf(path_to_log, 8192, "%s/log.txt", home);
    int fd = open(path_to_log, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 1)
    {
        perror("open");
        return;
    }
    close(fd);
}

void make_bash()
{
    snprintf(bash, 8192, "%s/.myshrc", home);
    int fd = open(bash, O_WRONLY | O_CREAT, 0644);
    if (fd < 1)
    {
        perror("open");
        return;
    }
    close(fd);
}


void output()
{
    snprintf(out, 8192, "%s/stdout.txt", home);
    int fd = open(out, O_WRONLY | O_CREAT| O_TRUNC, 0644);
    if (fd < 1)
    {
        perror("open");
        return;
    }
    close(fd);
}


void kill_processes()
{
    for (int i = 0; i < proc_count; i++)
    {
        if (strcmp(processes[i].state, "running") == 0)
        {
            // printf("heheheh\n");
            if (kill(processes[i].pid, SIGSTOP) == -1)
            {
                perror("kill"); // Debugging output in case of error
            }
        }
    }
}

int main()
{
    proc_count = 0;
    initialize_home_dir();
    make_bash();
    output();
    fg_process_pid = -1; // Initialize foreground process PID

    if (chdir(home) != 0)
    {
        printf("Error in switching directory\n");
        return 0;
    }

    init_prev();
    clear_log();
    bg_count = 0;
    strcpy(fg, "");

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    sigset_t sigset;
    sigprocmask(SIG_BLOCK, NULL, &sigset);

    if (sigismember(&sigset, SIGCHLD))
    {
        printf("SIGCHLD is blocked!\n");
    }

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    while (1)
    {
        //printf("proc count %d\n", proc_count);
        //tcsetpgrp(STDIN_FILENO, getpgrp());
        getdisplay();
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            if (feof(stdin))
            {
                fflush(stdout);

                kill_processes();

                printf("\n");
                fflush(stdout);
                exit(0);

            }
            else if (ferror(stdin))
            {
                perror("fgets");
                clearerr(stdin);
            }
            continue;
        }
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n')
        {
            input[len - 1] = '\0';
        }

        log_cmd(input);
        tokenise();
        //printf("hehehe\n");
    }
    return 0;
}
