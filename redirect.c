#include "redirect.h"
#include "valid.h"

int saved_stdin = -1;

void copy_tokens()
{
    for (int i = 0; i < size; i++)
    {
        strcpy(copy[i], tokens[i]);
        // printf("copy: %s\n", copy[i]);
    }
}

void redirect()
{
    copy_tokens();
    char *in_file = NULL;
    char *out_file = NULL;
    int append = 0;
    int ind = 0;
    int f;
    has_op = 0;

    for (int i = 0; i < size; i++)
    {
        //printf("copy[%d] = %s\n", i, copy[i]);
        if (strcmp(copy[i], "<") == 0)
        {
            i++;
            if (i < size)
            {
                in_file = copy[i];
                has_op = 1;
            }
            else
            {
                printf(RED "Invalid arguments for input\n" RESET);
                return;
            }
        }
        else if (strcmp(copy[i], ">") == 0)
        {
            i++;
            if (i < size)
            {
                out_file = copy[i];
                append = 0;
                has_op = 0;
            }
            else
            {
                printf("Invalid arguments for output redirection b\n");
                return;
            }
        }
        else if (strcmp(copy[i], ">>") == 0)
        {
            i++;
            if (i < size)
            {
                out_file = copy[i];
                append = 1;
                has_op = 0;
            }
            else
            {
                printf("Invalid arguments for output redirection c\n");
                return;
            }
        }
        else
        {
            strcpy(tokens[ind++], copy[i]);  // Corrected string copy
        }
    }

    size = ind;
    int std_in = -1;
    int std_out = -1;

    if (in_file != NULL)
    {
        int fd = open(in_file, O_RDONLY);
        if (fd == -1)
        {
            perror("No such input file found!");
            return;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (out_file != NULL)
    {
        int fd;
        if (append)
        {
            fd = open(out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else
        {
            fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        if (fd == -1)
        {
            perror("Failed to open output file");
            return;
        }
        std_out = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    if (!is_valid_command())
    {
        printf(RED "Error: Invalid command\n" RESET);
    }

    if (std_in != -1)
    {
        dup2(std_in, STDIN_FILENO);
        close(std_in);
    }
    if (std_out != -1)
    {
        dup2(std_out, STDOUT_FILENO);
        close(std_out);
    }
    fflush(stdout);
}
