#include "tokenise.h"
#include "valid.h"
#include "pipes.h"
#include "redirect.h"

void tokenise()
{
    size = 0;
    char *saveptr;
    char *cmd = strtok_r(input, ";", &saveptr); // Tokenize input using ;
    // printf("input is %s\n", input);
    while (cmd != NULL)
    {
        // Skip leading spaces
        while (*cmd == ' ')
        {
            cmd++;
        }
        // printf("Command: %s\n", cmd); // Print the command after ; tokenization
        tokenise_and(cmd); // Further tokenize the command using &
        cmd = strtok_r(NULL, ";", &saveptr);
    }
}

void tokenise_and(char *cmd)
{
    // printf("before and ", cmd);
    char token_list[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int token_count = 0;

    char *token_start = cmd;
    char *current = cmd;

    while (*current != '\0' && token_count < MAX_TOKENS)
    {
        // Find the next '&' or the end of the string
        while (*current != '&' && *current != '\0')
        {
            current++;
        }

        // Calculate token length including '&' if it's present
        size_t token_length = current - token_start;

        // If '&' is found, include it in the current token
        if (*current == '&')
        {
            token_length++;
        }

        // Copy the token to the array and ensure it is null-terminated
        strncpy(token_list[token_count], token_start, token_length);
        token_list[token_count][token_length] = '\0'; // Null-terminate the string

        // Trim leading spaces of the token
        char *trim_start = token_list[token_count];
        while (*trim_start == ' ')
        {
            trim_start++;
        }
        if (trim_start != token_list[token_count])
        {
            memmove(token_list[token_count], trim_start, strlen(trim_start) + 1);
        }

        token_count++;

        // Move to the start of the next token
        token_start = current;
        // Skip the '&' and spaces for the next token
        if (*current == '&')
        {
            token_start++;
        }
        current = token_start;
        while (*token_start == ' ')
        {
            token_start++;
            current++;
        }
    }
    // Print the tokens
    for (int i = 0; i < token_count; i++)
    {
        // printf("Token %d: \"%s\"\n", i, token_list[i]);
        tokenise_space(token_list[i]);
    }
}

void tokenise_space(char *cmd)
{
    char *saveptr;
    // printf("before space tokens: ", cmd);
    char *token = strtok_r(cmd, " ", &saveptr);
    temp_size = 0;
    while (token != NULL)
    {
        strcpy(temp_tokens[temp_size], token);
        // printf("size %d , %s\n",temp_size,temp_tokens[temp_size]);
        temp_size++;
        token = strtok_r(NULL, " ", &saveptr);
    }
    execute_pipe();
    //printf("hello after pipe\n");
}
