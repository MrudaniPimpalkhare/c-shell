#include "pipes.h"
#include "valid.h"
#include "redirect.h"

void execute_pipe()
{
    int ind = 0;
    int has_pipe = 0;
    int std_in = -1;
    int std_in_1 = dup(STDIN_FILENO);
    int std_out = dup(STDOUT_FILENO);

    for (int i = 0; i < temp_size; i++)
    {
        if (std_in != -1)
        {
            dup2(std_in, STDOUT_FILENO); // Restore the original stdout before processing the next command
            close(std_in);
            std_in = -1;
        }
        if (strcmp(temp_tokens[i], "|") != 0)
        {
            strcpy(tokens[ind++], temp_tokens[i]); // Collect the command before the pipe
        }
        else
        {
            has_pipe = 1;
            size = ind;

            int pipefd[2];
            if (pipe(pipefd) == -1)
            {
                perror("pipe");
                return;
            }

            int pid = fork();
            if (pid == -1)
            {
                perror("fork");
                return;
            }
            else if (pid == 0)
            {
                close(pipefd[0]);               // Close the read end in the child process
                dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to write end of pipe
                close(pipefd[1]);
                run_command(); // Execute the command before the pipe
                exit(0);
            }
            else
            {
                int status;
                close(pipefd[1]);              // Close the write end in the parent process
                dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to read end of pipe
                close(pipefd[0]);
                waitpid(pid, &status, 0);
                ind = 0; // Reset the index for the next command after the pipe
            }
        }
    }

    size = ind;
    run_command();

    dup2(std_in_1, STDIN_FILENO);
    dup2(std_out, STDOUT_FILENO);
    close(std_in_1);
    close(std_out);
}

int check_pipe()
{
    for (int i = 0; i < size; i++)
    {
        if ((strcmp(tokens[i], ">") == 0) || (strcmp(tokens[i], ">>") == 0) || (strcmp(tokens[i], "<") == 0))
        {
            return 1;
        }
    }
    return 0;
}

void run_command()
{
    int c = check_pipe();
    if (c == 0)
    {
        has_op = 1;
        if (!is_valid_command())
        {
            printf(RED "Invalid command %s\n" RESET, tokens[0]);
            return;
        }
    }
    else
    {
        redirect();
    }
}
