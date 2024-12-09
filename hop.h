#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "header.h"
#include <pwd.h>
#include <errno.h>
#include <sys/utsname.h>
#include <dirent.h>

void initialize_home_dir();
void initialize_proj_dir(char *dir);
int hop(char* path);
void hop_multiple(int count);
void tilde(char* full_path);
void init_prev();
