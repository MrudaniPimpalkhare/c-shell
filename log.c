#include "log.h"
#include "tokenise.h"

#define MAX_LINE_LENGTH 8192
#define MAX_LINES 15

int line_num = 0;
int offset = 0;
char words[MAX_LEN][MAX_LEN];
int count = 0;
int line_count;

void log_cmd(char *input)
{
    int flag = 1;
    char *saveptr;
    char copy_input[MAX_LEN];
    strcpy(copy_input, input);
    // printf("%s\n", input);
    char *token = strtok_r(copy_input, " ", &saveptr);
    while (token != NULL)
    {
        strcpy(words[count], token);
        if (strcmp(words[count++], "log") == 0)
        {
            flag = 0;
            break;
        }
        token = strtok_r(NULL, " ", &saveptr);
    }
    if (flag)
    {
        // printf("%s\n", input);
        write_to_line(input);
    }
}

void print_log()
{
    char buffer[MAX_LINE_LENGTH];
    FILE *file = fopen(path_to_log, "r");
    if (file != NULL)
    {
        while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL)
        {
            // Remove newline character if present
            buffer[strcspn(buffer, "\n")] = 0;
            printf(YELLOW "%s\n" RESET, buffer);
        }
        fclose(file);
    }
}

void purge()
{
    FILE *out_file = fopen(path_to_log, "w");
    if (out_file == NULL)
    {
        perror("Error opening file for writing");
        return;
    }
    fclose(out_file);
}

void execute(int number)
{
    char buffer[MAX_LINE_LENGTH];
    int line_count = 0;
    FILE *file = fopen(path_to_log, "r");
    if (file != NULL)
    {
        while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL && line_count < number)
        {
            buffer[strcspn(buffer, "\n")] = 0;
            line_count++;
            if (line_count == number)
            {
                pid_t shell_pgid = getpgrp();
                strncpy(input, buffer, MAX_LEN);
                tcsetpgrp(STDIN_FILENO, shell_pgid);
                tokenise();
                kill(shell_pgid,SIGTSTP);
                break;
            }
        }
        fclose(file);
    }
    if (number > line_count)
    {
        printf(RED "not enough log statements to print\n" RESET);
        return;
    }
}

void write_to_line(char *input_string)
{
    // Array to store lines from the file
    char *lines[MAX_LINES];
    char buffer[MAX_LINE_LENGTH];
    line_count = 0;
    int l = 0;
    // Open the file to read existing lines
    FILE *file = fopen(path_to_log, "r");
    if (file != NULL)
    {
        while (fgets(buffer, MAX_LINE_LENGTH, file) != NULL && line_count < MAX_LINES)
        {
            // Remove newline character if present
            buffer[strcspn(buffer, "\n")] = 0;
            lines[line_count] = strdup(buffer);
            line_count++;
        }
        fclose(file);
    }

    // Add the new input string at the end
    //printf("%d\n", line_count);
    if (line_count==0 || (line_count> 0 && strcmp(lines[line_count - 1], input_string) != 0))
    {
        if (line_count < MAX_LINES)
        {
            lines[line_count] = strdup(input_string);
            line_count++;
        }
        else
        {
            printf("%d\n", line_count);
            for(int i = 1;i<line_count;i++)
            {
               strcpy(lines[i-1] , lines[i]);
            }
            strcpy(lines[line_count-1] ,input_string);
            // return;
        }
    }

    // Open the file in write mode to clear it and write the updated lines
    FILE *out_file = fopen(path_to_log, "w");
    if (out_file == NULL)
    {
        perror("Error opening file for writing");
        return;
    }

    for (int i = 0; i < line_count; i++)
    {
        fprintf(out_file, "%s\n", lines[i]);
        free(lines[i]); // Free the duplicated line memory
    }

    fclose(out_file);

    // Update the global line_num
    line_num = line_count;
}

void logg()
{
    if (size == 1)
    {
        print_log();
    }
    else
    {
        if (strcmp(tokens[1], "execute") == 0)
        {

            execute(line_num - atoi(tokens[2])+1);
        }
        else if (strcmp(tokens[1], "purge") == 0)
        {
            purge();
        }
        else
        {
            printf("not a valid flag.");
        }
    }
    return;
}
