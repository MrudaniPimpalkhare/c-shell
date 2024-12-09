#include "neonate.h"

pid_t get_most_recent_pid()
{
    DIR *proc_dir;
    struct dirent *entry;
    pid_t latest_pid = 0;
    time_t latest_time = 0;

    proc_dir = opendir("/proc");
    if (proc_dir == NULL)
    {
        perror("Error opening /proc");
        return -1;
    }

    while ((entry = readdir(proc_dir)) != NULL)
    {
        if (isdigit(entry->d_name[0]))
        {
            pid_t pid = atoi(entry->d_name);
            char stat_path[256];
            struct stat st;

            snprintf(stat_path, sizeof(stat_path), "/proc/%d", pid);
            if (stat(stat_path, &st) == 0)
            {
                if (st.st_ctime > latest_time)
                {
                    latest_time = st.st_ctime;
                    latest_pid = pid;
                }
            }
        }
    }

    closedir(proc_dir);
    return latest_pid;
}

int key_pressed()
{
    struct termios old_termios, new_termios;
    int old_flags;
    int ch;

    // Get current terminal attributes
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;

    // Disable canonical mode and echo
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    // Set non-blocking mode
    old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

    // Check for a keypress
    ch = getchar();

    // Restore old terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    fcntl(STDIN_FILENO, F_SETFL, old_flags);

    // If a key was pressed, put it back into the input buffer and return 1
    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    // No keypress detected
    return 0;
}

void neonate(int time)
{
    char input;

    while (1)
    {
        pid_t recent_pid = get_most_recent_pid();
        if (recent_pid != -1)
        {
            printf("%d\n", recent_pid);
        }
        else
        {
            printf("Error getting most recent PID\n");
        }

        // Wait for specified time or until 'x' is pressed
        for (int i = 0; i < time* 10; i++)
        {
            usleep(100000); // Sleep for 0.1 seconds
            if (key_pressed())
            {
                input = getchar();
                if (input == 'x')
                {
                    return;
                }
            }
        }
    }
}
