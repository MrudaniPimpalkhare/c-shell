#include "header.h"
void get_info(pid_t pid) {
    char proc_status;
    int proc_group;
    long unsigned int v_mem;
    char exec_path[MAX_LEN];
    char path[MAX_LEN];

    // Construct the path to the /proc/[pid]/stat file
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *stat_file = fopen(path, "r");
    if (stat_file == NULL) {
        perror("Error opening stat file");
        return;
    }

    // Variables to temporarily hold skipped values
    int dummy_int;
    unsigned long dummy_ulong;
    long int dummy_long;
    unsigned long long d_ull;

    // Read and discard the fields as necessary
    fscanf(stat_file, "%d", &dummy_int);  // PID
    fscanf(stat_file, "%s", exec_path);   // Executable name
    fscanf(stat_file, " %c", &proc_status); // Process status
    fscanf(stat_file, "%d", &dummy_int);  // Skipped field
    fscanf(stat_file, "%d", &proc_group); // Process group ID
    for(int i=0;i<3;i++)
    {
        fscanf(stat_file, "%d", &dummy_int);
    }
    //fscanf(stat_file, "%u", &proc_group);
    // Skip the next 6 fields
    //fscanf(stat_file, "%*d %*s %c %*d %d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld %*ld %*ld %*ld %*ld %*llu %lu", &proc_status, &proc_group, &v_mem);
    fclose(stat_file);
    for (int i = 0; i < 6; i++) {
        fscanf(stat_file, "%lu", &dummy_ulong);
    }
    for(int i=0;i<6;i++)
    {
        fscanf(stat_file, "%lu", &dummy_long);
    }
    fscanf(stat_file,"%llu", &d_ull);
    fscanf(stat_file, "%lu", &v_mem);
    fclose(stat_file);

    // Get the executable path using readlink
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t len = readlink(path, exec_path, sizeof(exec_path) - 1);
    if (len == -1) {
        perror("Error reading executable path");
        return;
    }
    exec_path[len] = '\0';

    // Print the gathered information
    printf(WHITE"pid: %d\n" RESET, pid);
    printf(CYAN "Process status: %c\n\n" RESET, proc_status);
    printf(MAGENTA "Process group: %d\n\n" RESET, proc_group);
    printf(YELLOW "Virtual memory: %lu\n\n" RESET, v_mem);
    printf(GREEN "Executable path: %s\n\n" RESET, exec_path);
}


void proclore()
{
    if(size>=2)
    {
        get_info(atoi(tokens[1]));
    }else{
        get_info(getpid());
    }
}