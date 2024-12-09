#include "func.h"
#include "tokenise.h"
int is_empty_line(char *line) {
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {
            return 0;
        }
    }
    printf("empty line\n");
    return 1;
}

// Function to replace $1 with actual arguments in the line
void replace_arg_with_input(char *line) {
    char temp[MAX_LEN];
    char *pos;
    int i = 1;

    while ((pos = strstr(line, "$1")) != NULL && i < temp_size) {
        if (pos > line && *(pos - 1) == '"' && *(pos + 2) == '"') {
            *(pos - 1) = '\0';  // Terminate string before the quote
            snprintf(temp, sizeof(temp), "%s%s%s", line, temp_tokens[i], pos + 3); // Skip the closing quote
        } else {
            *pos = '\0';  // Terminate string at the start of "$1"
            snprintf(temp, sizeof(temp), "%s%s%s", line, temp_tokens[i], pos + 2);
        }
        strcpy(line, temp);
        i++;
    }
}

void parse_function() {
    FILE *file = fopen(bash, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LEN];
    char function_body[MAX_LEN] = "";
    int func_found = 0;

    while (fgets(line, sizeof(line), file)) {
        // Look for "func" keyword
        if (line[0] == '{' || line[0] == '}') {
            continue;
        }
        if (strncmp(line, "func", 4) == 0) {
            //printf("found!\n");
            char *fun = strtok(line + 5, " \n"); // Get the function name after "func"
            char *func_name = strtok(fun, "()");
            //printf("%s\n", func_name);
            if (func_name && strcmp(func_name, temp_tokens[0]) == 0) {
                func_found = 1;
                continue;
            }
        }

        // If function is found, concatenate non-empty lines into the function body
        if (func_found && !is_empty_line(line)) {
            //printf("line is :%s\n", line);
            char *clean_line = strtok(line, "\n"); // Remove newline from the line
            replace_arg_with_input(clean_line); // Replace $1 with arguments
            strcat(function_body, clean_line); // Append to function body
            strcat(function_body, ";");        // Add semicolon after each command
            //printf("function body %s\n", function_body);
        }

        // Stop when next "func" keyword or end of file is encountered
        if (func_found && (strncmp(line, "func", 4) == 0 || feof(file))) {
            break;
        }
    }

    fclose(file);
    strcpy(input,function_body);
    tokenise();
    printf("%s\n",function_body);
}
