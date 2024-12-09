#include "ping.h"
#include "valid.h"

int ping(pid_t pid, int s_num)
{
    s_num = (s_num%32);
    if (kill(pid, s_num) == -1)
    {
        perror("Error sending signal");
        return 0;
    }
    if (s_num == SIGTSTP)
    {
        printf("Sent signal %d (SIGTSTP) to process with pid %d\n", s_num, pid);
        update_process(pid, "stopped"); // Update status to "stopped"
    }
    else if (s_num == SIGCONT)
    {
        printf("Sent signal %d (SIGCONT) to process with pid %d\n", s_num, pid);
        update_process(pid, "running"); // Update status to "running"
    }
    else if (s_num == SIGINT)
    {
        printf("Sent signal %d (SIGINT) to process with pid %d\n", s_num, pid);
        update_process(pid, "exited");
    }else if(s_num == SIGTERM)
    {
        printf("Sent signal %d (SIGKILL) to process with pid %d\n", s_num, pid);
        update_process(pid, "terminated");

    }
    else
    {
        printf("Sent signal %d to process with pid %d\n", s_num, pid);
    }
    return 1;
}