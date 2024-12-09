#include "alias.h"
#include "valid.h"
#include "hop.h"
#include "seek.h"
#include "log.h"
#include "reveal.h"
#include "proclore.h"
#include "tokenise.h"



#define MAX_ARG_SIZE 100

int find_command()
{
    char *argv[MAX_ARG_SIZE];
    char *list[] = {"hop", "log", "reveal", "proclore", "seek", NULL};
    for (int i = 0; list[i] != NULL; i++)
    {
        if (strcmp(list[i], tokens[0]) == 0)
        {
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
            return 1;
        }
    }

    int is_background = 0;
    if (strcmp(tokens[size - 1], "&") == 0)
    {
        is_background = 1;
        size--; // Remove the '&' from the argument list
    }

    // Prepare argv for execvp
    for (int ind = 0; ind < size; ind++)
    {
        //printf("doing this\n");
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
        // Child process
        if (execvp(argv[0], argv) == -1)
        {
            // fprintf(stderr, "Invalid command: %s\n", argv[0]);
            exit(127);
        }
        exit(0);
    }
    else
    {
        if (is_background)
        {
            // Background process: Handle and return immediately
            handle_background_process(pid);
        }
        else
        {
            // Foreground process: Wait for it to complete
            // printf("foreground\n");
            int status;
            time_t start_time = time(NULL);
            waitpid(pid, &status, 0); // Wait for the specific child process
            time_t end_time = time(NULL);
            double elapsed_time = difftime(end_time, start_time);

            // Print process name and time if it took more than 2 seconds
            if (elapsed_time > 2.0)
            {
                // printf("Process %s took %.0f seconds.\n", argv[0], elapsed_time);
                snprintf(fg, MAX_LEN_2, "%s: %.0fs", tokens[0], elapsed_time);
            }

            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            {
                // If execvp failed in the child, exit the parent process
                fprintf(stderr, "Command failed with exit status %d\n", WEXITSTATUS(status));
                return 0;
            }
            sigchld_handler();
        }
    }
}
