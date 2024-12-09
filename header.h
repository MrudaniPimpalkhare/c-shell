#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <dirent.h> 
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h> // For time()
#include <termios.h>
#include <signal.h> // For signal handling
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>


#define MAX_LEN 4096 
#define MAX_LEN_2 8192
#define MAX_TOKENS 100
#define MAX_ARGS 100
#define BUFFER_SIZE 4096
#define HOST "man.he.net"
#define PORT "80"
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define DIR_COLOR "\033[34m"      // Blue for directories
#define EXEC_COLOR "\033[32m"     // Green for executables
#define FILE_COLOR "\033[37m"     // Default color for regular files
#define MAX_PROCESSES 100

extern char path[MAX_LEN];
extern char home[MAX_LEN];
extern char input[MAX_LEN];
extern char tokens[100][MAX_LEN];
extern char temp_tokens[100][MAX_LEN];
extern char path_copy[MAX_LEN];
extern char bash[MAX_LEN_2];
extern char path_to_log[MAX_LEN];
extern char previous_dir[MAX_LEN]; 
extern char fg[MAX_LEN_2];
extern char alias[MAX_LEN_2];
extern char copy[100][MAX_LEN_2];
extern char commands_pipe[100][100][MAX_LEN];
extern char out[MAX_LEN];
extern int size_array[100];
extern int al_size;
extern int size;
extern int temp_size;
extern int bg;
extern int proc_count;
extern int bg_count;
extern int fg_process_pid;
extern pid_t shell_pgid;
extern int has_op;


#endif
