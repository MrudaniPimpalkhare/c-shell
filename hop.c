#include "hop.h"
#include "valid.h"

#define MAX_PATH 4096

void init_prev()
{
    strncpy(path_copy,path,MAX_LEN);
    if (getcwd(previous_dir, sizeof(previous_dir)) == NULL)
    {
        perror("Error getting current directory");
        return;
    }    
}

void tilde(char *full_path)
{
    if (strncmp(full_path, home, strlen(home)) == 0)
    {
        // Shift the path to replace home prefix with "~"
        snprintf(full_path, MAX_PATH, "~%s", full_path + strlen(home));
    }
}

void initialize_home_dir()
{
    if (getcwd(home, sizeof(home)) == NULL)
    {
        perror("Error getting current directory");
        return;
    }
    strncpy(path,home,MAX_LEN);
    //printf("%s\n\n",home);
}

int hop(char *p)
{
    char current_dir[MAX_PATH];
    char new_path[MAX_PATH];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL)
    {
        perror("Error getting current directory");
        return 0;
    }
    if (p == NULL || strlen(p) == 0)
    {
        // No argument, hop to home directory
        printf("%s\n\n", path_copy );
        return 1;
    }
    else if (strcmp(p, "-") == 0)
    {
        char temp[MAX_LEN];
        strncpy(temp,previous_dir,MAX_PATH);
        strncpy(previous_dir, path_copy, MAX_PATH);
        //printf("entered -\n");
        strncpy(path, temp, MAX_PATH);
        strncpy(path_copy,path,MAX_LEN);
        printf("%s\n\n",path_copy);
        tilde(path);
        return 1;
    }
    else if (strcmp(p, "~") == 0)
    {
        strncpy(path, home, MAX_PATH);
        strncpy(previous_dir, path_copy, MAX_PATH);
        strncpy(path_copy,path,MAX_LEN);
        printf("%s\n\n",path_copy);
        return 1;
    }
    else
    {
        // Absolute or relative path
        strncpy(previous_dir, path_copy, MAX_PATH);
        chdir(path_copy);
        realpath(p, new_path);
        //printf("%s\n", path);
        if (chdir(p) == 0)
        {
            //printf("chdir\n");
            strncpy(path, new_path, MAX_PATH);
            strncpy(path_copy,path,MAX_LEN);
            tilde(path);
            printf("%s\n", path_copy);
            return 1;
        }
        else
        {
            fprintf(stderr, "hop: %s: %s\n", path, strerror(errno));
            strncpy(path, previous_dir, MAX_PATH);
            printf(RED "error %s\n\n"RESET, path_copy );
            return 0;
        }
        //printf("current path:%s\nprevious path:%s\n",path,previous_dir);
        return 0;
    }
}

void hop_multiple(int count)
{
    for (int i = 1; i < count; i++)
    {
        //printf("hi\n");
        hop(tokens[i]);
    }
}
