#include "header.h"


void seek_check();
void seek_recursive(const char *base_dir, const char *current_dir, const char *pattern, int dir_only, int file_only, int execute, int *match_count, char *last_match);
