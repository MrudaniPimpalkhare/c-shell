#include "valid.h"
#include "hop.h"
#include "alias.h"
#include "seek.h"
#include "log.h"
#include "reveal.h"
#include "proclore.h"
#include "tokenise.h"
#include "redirect.h"
#include "ping.h"
#include "signals.h"
#include "iman.h"
#include "func.h"
#include "neonate.h"

int is_valid_command();
void sigchld_handler();
void handle_background_process(pid_t pid);
void add_process(pid_t pid,int bg);
void update_process(pid_t pid, char* state);
int cmp_process(const void* a, const void* b);
int is_command_executable(char *command);
int find_alias();
int find_func();
int bg_pids[100]; // To store background process PIDs
char bg_cmd[100][MAX_LEN];
Process processes[MAX_PROCESSES];

#define MAX_ARG_SIZE 100
int has_log = 0;
int custom = 0;
char *argv[MAX_ARG_SIZE];

int compare_processes(const void *a, const void *b)
{
    Process *procA = (Process *)a;
    Process *procB = (Process *)b;
    return strcmp(procA->command[0], procB->command[0]);
}

void add_process(pid_t pid, int bg)
{
    if (proc_count < MAX_PROCESSES)
    {
        // printf("heree %s\n", argv[0]);
        processes[proc_count].command_size = size;
        for (int i = 0; i < size; i++)
        {
            strcpy(processes[proc_count].command[i], argv[i]);
            // printf("hello, %s\n", processes[proc_count].command[i]);
        }
        processes[proc_count].pid = pid;
        processes[proc_count].bg = bg;
        strcpy(processes[proc_count].state, "running");
        proc_count++;
        // printf("%d\n", proc_count);
    }
}

void update_process(pid_t pid, char *state)
{
    for (int i = 0; i < proc_count; i++)
    {
        if (processes[i].pid == pid)
        {
            strcpy(processes[i].state, state);
            break;
        }
    }
}

void print_process_list()
{
    qsort(processes, proc_count, sizeof(Process), compare_processes);
    // printf("%d\n", proc_count);
    for (int i = 0; i < proc_count; i++)
    {
        if (strcmp(processes[i].state, "killed") == 0 || strcmp(processes[i].state, "exited") == 0 || strcmp(processes[i].state, "terminated") == 0)
        {
            continue;
        }
        int s = processes[i].command_size;
        printf(CYAN "%d:\t" RESET, processes[i].pid);
        for (int j = 0; j < s; j++)
        {
            printf("%s ", processes[i].command[j]);
        }
        printf("%s\n", processes[i].state);
    }
}

int find_alias()
{
    // printf("in find\n");
    char word[MAX_LEN];
    char line[MAX_LEN];
    char pattern[MAX_LEN_2];
    strcpy(word, tokens[0]);
    int found_alias = 0;
    snprintf(pattern, sizeof(pattern), "%s =", word);

    FILE *fp = fopen(bash, "r");
    if (fp == NULL)
    {
        perror("Error opening file");
        return 0;
    }

    while (fgets(line, MAX_LEN, fp) != NULL)
    {
        int commented = 0;
        if (strstr(line, pattern) != NULL)
        {
            for (int i = 0; i < strlen(line); i++)
            {
                if (line[i] == '#')
                {
                    commented = 1;
                    break;
                }
            }
            if (commented == 0)
            {
                found_alias = 1;
                // printf("%s\n", line);
                break;
            }
        }
    }

    fclose(fp);

    if (found_alias)
    {
        char *saveptr;
        char *token = strtok_r(line, "=", &saveptr);
        token = strtok_r(NULL, "=", &saveptr);

        // Trim leading spaces
        while (*token == ' ')
        {
            token++;
        }

        // Trim trailing newline
        size_t len = strlen(token);
        if (len > 0 && token[len - 1] == '\n')
        {
            token[len - 1] = '\0';
        }

        // Trim trailing spaces
        len = strlen(token);
        while (len > 0 && token[len - 1] == ' ')
        {
            token[len - 1] = '\0';
            len--;
        }
        int k = 0;
        char *ptr;
        char *sp_token = strtok_r(token, " ", &ptr);
        while (sp_token != NULL)
        {
            strcpy(tokens[k], sp_token);
            k++;
            sp_token = strtok_r(NULL, " ", &ptr);
        }
        return k;
    }

    return 0;
}

void sigchld_handler()
{
    int status;
    pid_t pid;
    // printf("here,handler\n");

    // Reap all terminated or stopped/continued children
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
    {
        for (int i = 0; i < proc_count; i++)
        {
            if (processes[i].pid == pid)
            {
                if (WIFEXITED(status))
                {
                    printf(GREEN "%s exited normally (%d)\n" RESET, processes[i].command[0], pid);
                    update_process(pid, "exited");
                }
                else if (WIFSIGNALED(status))
                {
                    int sig = WTERMSIG(status);
                    printf(GREEN "%s terminated (%d) by signal %d\n" RESET, processes[i].command[0], pid, sig);
                    update_process(pid, "terminated");
                }
                else if (WIFSTOPPED(status))
                {
                    printf(YELLOW "%s stopped (%d)\n" RESET, processes[i].command[0], pid);
                    update_process(pid, "stopped");
                }
                else if (WIFCONTINUED(status))
                {
                    printf(CYAN "%s continued (%d)\n" RESET, processes[i].command[0], pid);
                    update_process(pid, "running");
                }
            }
        }
    }
}

int is_command_executable(char *command)
{
    if (access(command, X_OK) == 0)
    {
        return 1;
    }
    char *path = getenv("PATH");
    char *path_copy = strdup(path);
    char *token = strtok(path_copy, ":");
    while (token != NULL)
    {
        char full_path[MAX_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", token, command);
        if (access(full_path, X_OK) == 0)
        {
            free(path_copy);
            return 1;
        }
        token = strtok(NULL, ":");
    }
    free(path_copy);
    return 0;
}

void handle_background_process(pid_t pid)
{
    if (bg_count < MAX_ARG_SIZE)
    {
        bg_pids[bg_count++] = pid;
        strcpy(bg_cmd[bg_count - 1], tokens[0]);
        printf(GREEN "Background process started with PID: %d\n" RESET, pid);
    }
    else
    {
        fprintf(stderr, "Maximum number of background processes reached.\n");
    }
}

int is_valid_command()
{

    custom = 0;
    char *list[] = {"hop", "log", "reveal", "proclore", "seek", "activities", "ping", "fg", "bg", "iMan", "neonate", NULL};
    for (int i = 0; list[i] != NULL; i++)
    {
        if (strcmp(list[i], tokens[0]) == 0)
        {
            custom = 1;
            if (strcmp(tokens[0], "hop") == 0)
            {
                hop_multiple(size);
            }
            else if (strcmp(tokens[0], "reveal") == 0)
            {
                reveal(size);
            }
            else if (strcmp(tokens[0], "log") == 0)
            {
                logg();
            }
            else if (strcmp(tokens[0], "proclore") == 0)
            {
                proclore();
            }
            else if (strcmp(tokens[0], "seek") == 0)
            {
                seek_check();
            }
            else if (strcmp(tokens[0], "activities") == 0)
            {
                print_process_list();
            }
            else if (strcmp(tokens[0], "ping") == 0)
            {
                if (size == 3)
                {
                    ping(atoi(tokens[1]), atoi(tokens[2]));
                }
                else
                {
                    return 0;
                }
            }
            else if (strcmp(tokens[0], "fg") == 0)
            {
                if (size == 2)
                {
                    bg_to_fg(atoi(tokens[1]));
                }
                else
                {
                    printf(RED "wrong paramters." RESET);
                }
            }
            else if (strcmp(tokens[0], "bg") == 0)
            {
                if (size == 2)
                {
                    run_bg(atoi(tokens[1]));
                }
                else
                {
                    printf(RED "wrong paramters\n" RESET);
                }
            }
            else if (strcmp(tokens[0], "iMan") == 0)
            {
                if (size >= 2)
                {
                    fetch_man_page(tokens[1]);
                }
                else
                {
                    printf(RED "wrong parameters\n" RESET);
                }
            }
            else if (strcmp(tokens[0], "neonate") == 0)
            {
                if (size == 3)
                {
                    if (strcmp(tokens[1], "-n") == 0)
                    {
                        neonate(atoi(tokens[2]));
                    }
                }
            }
            return 1;
        }
        char temp[100][MAX_LEN];
        if (size > 1)
        {
            for (int i = 1; i < size; i++)
            {
                strcpy(temp[i - 1], tokens[i]);
            }
        }
        int new_size = find_alias();
        if (new_size > 0)
        {
            int t = size - 1;
            for (int i = 0; i < t; i++)
            {
                strcpy(tokens[new_size + i], temp[i]);
            }
            size = new_size + t;
            if (!is_valid_command())
            {
                printf(RED "Error: invalid command : %s\n" RESET, tokens[0]);
                exit(0);
                return 0;
            }
            return 1;
        }
    }

    if (custom == 0)
    {
        if (!is_command_executable(tokens[0]))
        {
            return 0;
        }

        int is_background = 0;
        if (strcmp(tokens[size - 1], "&") == 0)
        {
            is_background = 1;
            size--;
        }

        for (int ind = 0; ind < size; ind++)
        {
            argv[ind] = tokens[ind];
        }
        argv[size] = NULL;

        int pid = fork();

        if (pid < 0)
        {
            perror("fork");
            return 0;
        }
        else if (pid == 0)
        {
            if (is_background)
            {
                // printf("hhdhd\n");
                setsid();

                int dev_null_fd = open("/dev/null", O_RDWR);
                dup2(dev_null_fd, STDIN_FILENO);
                dup2(dev_null_fd, STDOUT_FILENO);
                dup2(dev_null_fd, STDERR_FILENO);
                close(dev_null_fd);
            }
            if (execvp(argv[0], argv) == -1)
            {
                exit(127);
            }
            exit(0);
        }
        else
        {
            // setpgid(pid, 0);
            if (is_background)
            {
                add_process(pid, 1);
                handle_background_process(pid);
            }
            else
            {
                add_process(pid, 0);
                fg_process_pid = pid;
                int status;
                time_t start_time = time(NULL);
                waitpid(pid, &status, WUNTRACED);
                //printf("here!");
                fflush(stdout);
                time_t end_time = time(NULL);
                double elapsed_time = difftime(end_time, start_time);
                if (elapsed_time > 2.0)
                {
                    snprintf(fg, MAX_LEN_2, "%s: %.0fs", tokens[0], elapsed_time);
                }

                if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
                {
                    fprintf(stderr, "Command failed with exit status %d\n", WEXITSTATUS(status));
                    return 0;
                }

                if (WIFEXITED(status))
                {
                    update_process(pid, "exited");
                }
                else if (WIFSIGNALED(status))
                {
                    update_process(pid, "killed");
                }
                else if (WIFSTOPPED(status))
                {
                    update_process(pid, "stopped");
                }
                fg_process_pid = -1;
                //printf("hello\n");
                tcsetpgrp(STDIN_FILENO, getpgrp());

            }
        }
    }

    return 1;
}
