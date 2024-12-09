#include "seek.h"
#include "hop.h"

// Global variables
char spec[MAX_LEN];
char curr_dir[MAX_LEN];
char target[MAX_LEN];
char pattern[MAX_LEN];

int flag_d = 0;
int flag_e = 0;
int flag_f = 0;

// Function declarations

void seek_check()
{
    int i = 1;

    // Reset flags and arguments
    flag_d = 0;
    flag_f = 0;
    flag_e = 0;
    char arg[2][MAX_LEN] = {"NONE", "NONE"};
    // printf("%s %s", arg[0], arg[1]);
    //  Parse flags and arguments
    for (i = 1; i < size; i++)
    {
        if (strcmp(tokens[i], "-d") == 0)
        {
            flag_d = 1;
        }
        else if (strcmp(tokens[i], "-f") == 0)
        {
            flag_f = 1;
        }
        else if (strcmp(tokens[i], "-e") == 0)
        {
            flag_e = 1;
        }
        else
        {
            if (flag_d == 1 && flag_f == 1)
            {
                printf(RED "Invalid flags! Cannot use -d and -f together.\n" RESET);
                return;
            }
            break; // End of flags
        }
    }

    // Handle pattern (target)
    if (i < size)
    {
        strcpy(arg[0], tokens[i++]);
        strncpy(target, arg[0], MAX_LEN - 1);
    }
    else
    {
        printf(RED "Invalid arguments! A target name is required.\n" RESET);
        return;
    }
    // Optional second argument for pattern matching (unused in this case)
    if (i < size)
    {
        strcpy(arg[1], tokens[i]);
        strncpy(pattern, arg[1], MAX_LEN - 1);
    }

    if (strcmp(arg[1], "NONE") == 0)
    {
        // printf("no pattern specified\n");
        strcpy(pattern, ".");
    }

    int match_count = 0;
    char last_match[MAX_LEN] = "";
    // printf("target : %s\n", target);
    // printf("pattern : %s\n", pattern);
    seek_recursive(pattern, "", target, flag_d, flag_f, flag_e, &match_count, last_match);
    if (match_count == 0)
    {
        printf("No matches found!\n");
    }

    // Handle exact match if `-e` flag is set
    if (flag_e && match_count == 1)
    {
        struct stat statbuf;
        if (stat(last_match, &statbuf) == 0)
        {
            if (S_ISDIR(statbuf.st_mode))
            {
                if (!hop(last_match))
                {
                    printf(RED "Missing permissions for task!" RESET);
                }
            }
            else if (S_ISREG(statbuf.st_mode))
            {
                FILE *file = fopen(last_match, "r");
                if (file != NULL)
                {
                    int c;
                    while ((c = fgetc(file)) != EOF)
                    {
                        putchar(c);
                    }
                    fclose(file);
                }
                else
                {
                    printf(RED "Missing permissions for task!\n" RESET);
                }
            }
        }
    }
}

void seek_recursive(const char *base_dir, const char *current_dir, const char *pattern, int dir_only, int file_only, int execute, int *match_count, char *last_match)
{
    char path[MAX_LEN];
    struct dirent *entry;
    struct stat statbuf;
    DIR *dir;

    // Construct the full path for the directory
    snprintf(path, sizeof(path), "%s%s%s", base_dir, (base_dir[strlen(base_dir) - 1] == '/' ? "" : "/"), current_dir);

    // Open the directory
    dir = opendir(path[0] ? path : ".");
    if (dir == NULL)
    {
        return;
    }

    // Read each entry in the directory
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip current and parent directory entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construct the full path of the current entry
        char full_path[MAX_LEN_2];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Get the status of the entry
        if (stat(full_path, &statbuf) == -1)
        {
            perror(RED "Error getting file status" RESET);
            continue;
        }

        // Use strstr for partial matches
        char name_copy[MAX_LEN];
        strcpy(name_copy, entry->d_name);
        int is_match = (strcmp(entry->d_name,target) == 0);
        if (!is_match)
        {
            char *saveptr;
            char *name = strtok_r(name_copy, ".", &saveptr);
            int is_match = (strcmp(name, target) == 0);
        }

        int should_print = is_match &&
                           ((S_ISDIR(statbuf.st_mode) && !file_only) ||
                            (S_ISREG(statbuf.st_mode) && !dir_only));

        // Print if it matches the criteria
        if (should_print)
        {
            (*match_count)++;
            strncpy(last_match, full_path, MAX_LEN - 1);

            // Print the matching files/directories
            char relative_path[MAX_LEN];

            // Prepend './' if the file is in a subdirectory
            if (current_dir[0] != '\0')
            {
                snprintf(relative_path, sizeof(relative_path), "./%s/%s",
                         current_dir, entry->d_name);
            }
            else
            {
                snprintf(relative_path, sizeof(relative_path), "%s", entry->d_name);
            }

            if (S_ISDIR(statbuf.st_mode))
            {
                printf(BLUE "%s\n" RESET, relative_path);
            }
            else
            {
                printf(GREEN "%s\n" RESET, relative_path);
            }
        }

        // Recursively search subdirectories
        if (S_ISDIR(statbuf.st_mode))
        {
            char new_current_dir[MAX_LEN];
            snprintf(new_current_dir, sizeof(new_current_dir), "%s%s%s",
                     current_dir, (current_dir[0] ? "/" : ""), entry->d_name);
            seek_recursive(base_dir, new_current_dir, pattern, dir_only, file_only, execute, match_count, last_match);
        }
    }

    // Close the directory
    closedir(dir);
}
