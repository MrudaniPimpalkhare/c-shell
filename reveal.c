#include "reveal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// ANSI escape codes for colors
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

int reveal_l = 0; // if details need to be displayed
int reveal_a = 0; // if hidden files need to be displayed
int arg = 0;

int alphasort(const struct dirent **a, const struct dirent **b)
{
    return strcmp((*a)->d_name, (*b)->d_name);
}

void permissions(mode_t mode)
{
    char perms[11];
    perms[0] = (S_ISDIR(mode)) ? 'd' : '-';
    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';
    printf("%s ", perms);
}

void reveal(int size)
{
    struct dirent **namelist;
    int n;
    struct stat file_stat;
    blkcnt_t total_blocks = 0; // Variable to store the total number of blocks

    // Build the path from the provided tokens
    char path_reveal[4096];
    if (strcmp(tokens[size - 1], "&") == 0)
    {
        if (size == 2)
        {
            printf(RED "invalid arguments\n" RESET);
            return;
        }
        if (strcmp(tokens[size - 2], "-") == 0)
        {
            strncpy(path_reveal, previous_dir, MAX_LEN);
        }
        else if (strcmp(tokens[size - 2], "~") == 0)
        {
            strncpy(path_reveal, home, MAX_LEN);
        }
    }
    else
    {
        if (strcmp(tokens[size - 1], "-") == 0)
        {
            strncpy(path_reveal, previous_dir, MAX_LEN);
        }
        else if (strcmp(tokens[size - 1], "~") == 0)
        {
            strncpy(path_reveal, home, MAX_LEN);
        }
        else
        {
            if (size == 1)
            {
                strncpy(path_reveal, path, MAX_LEN);
            }
        }
    }

    // Process flags
    if (size > 2)
    {
        for (int i = 1; i <= size - 1; i++)
        {
            int len = strlen(tokens[i]);
            if (tokens[i][0] == '-')
            {
                for (int j = 1; j < len; j++)
                {

                    if (tokens[i][j] == 'l')
                    {
                        reveal_l = 1;
                    }
                    else if (tokens[i][j] == 'a')
                    {
                        reveal_a = 1;
                    }
                    else
                    {
                        printf("%s is not a valid flag\n", tokens[i]);
                        break;
                    }
                }
            }
            else
            {
                strncpy(path_reveal, tokens[i], MAX_LEN);
                if (i != size - 1)
                {
                    printf(RED "invalid number of arguments\n" RESET);
                    break;
                }
            }
        }
    }

    // Check if path_reveal is a directory or a file
    if (stat(path_reveal, &file_stat) != 0)
    {
        printf(RED "File/directory not found\n" RESET);
        return;
    }

    // Handle directory
    if (S_ISDIR(file_stat.st_mode))
    {
        DIR *dr = opendir(path_reveal);
        if (dr == NULL)
        {
            perror("opendir");
            return;
        }

        // Scan the directory and sort the entries
        n = scandir(path_reveal, &namelist, NULL, alphasort);
        if (n < 0)
        {
            perror("scandir");
            closedir(dr);
            return;
        }

        // Calculate the total number of blocks
        for (int i = 0; i < n; i++)
        {
            if (strcmp(namelist[i]->d_name, ".") != 0 && strcmp(namelist[i]->d_name, "..") != 0)
            {
                char full_path[8192];
                snprintf(full_path, sizeof(full_path), "%s/%s", path_reveal, namelist[i]->d_name);
                if (stat(full_path, &file_stat) == 0)
                {
                    if (reveal_a == 1 || namelist[i]->d_name[0] != '.')
                    {
                        total_blocks += file_stat.st_blocks;
                    }
                }
            }
        }

        // Print the total number of blocks (only if -l flag is used)
        if (reveal_l == 1)
        {
            printf("total %ld\n", total_blocks);
        }

        // Print the directory contents
        for (int i = 0; i < n; i++)
        {
            if (strcmp(namelist[i]->d_name, ".") != 0 && strcmp(namelist[i]->d_name, "..") != 0)
            {
                char full_path[8192];
                snprintf(full_path, sizeof(full_path), "%s/%s", path_reveal, namelist[i]->d_name);
                if (stat(full_path, &file_stat) == 0)
                {
                    if (reveal_a == 1 || namelist[i]->d_name[0] != '.')
                    {
                        if (S_ISDIR(file_stat.st_mode))
                        {
                            printf(DIR_COLOR "%s\n" RESET, namelist[i]->d_name);
                        }
                        else if (S_ISREG(file_stat.st_mode))
                        {
                            if (file_stat.st_mode & S_IXUSR)
                            {
                                printf(EXEC_COLOR "%s\n" RESET, namelist[i]->d_name);
                            }
                            else
                            {
                                printf(WHITE "%s\n" RESET, namelist[i]->d_name);
                            }
                        }
                        else
                        {
                            printf(RED "%s (Other)\n" RESET, path_reveal);
                        }
                        if (reveal_l == 1)
                        {
                            permissions(file_stat.st_mode);
                            printf(GREEN "%ld " RESET YELLOW "%d " RESET CYAN "%d " RESET BLUE "%ld " RESET MAGENTA "%ld " RESET RED "%s\n" RESET,
                                   file_stat.st_nlink,
                                   file_stat.st_uid,
                                   file_stat.st_gid,
                                   file_stat.st_size,
                                   file_stat.st_mtime,
                                   namelist[i]->d_name);
                        }
                    }
                }
            }
            free(namelist[i]);
        }
        free(namelist);
        closedir(dr);
    }
    else
    {
        // Handle file
        printf(RED "%s\n" RESET, path_reveal);
        if (reveal_l == 1)
        {
            permissions(file_stat.st_mode);
            printf(GREEN "%ld " RESET YELLOW "%d " RESET CYAN "%d " RESET BLUE "%ld " RESET MAGENTA "%ld " RESET RED "%s\n" RESET,
                   file_stat.st_nlink,
                   file_stat.st_uid,
                   file_stat.st_gid,
                   file_stat.st_size,
                   file_stat.st_mtime,
                   path_reveal);
        }
    }

    // Reset flags
    reveal_a = 0;
    reveal_l = 0;
}
