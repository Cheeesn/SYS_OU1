#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <dirent.h>
#include <errno.h>


#define BLOCK_SIZE 512
char** check_arguments(char *argv[], int argc,int* threads, int* num_files);
int traverse_directory(const char *path, int *exit_code);
void kill_function_arr(int amount_of_index, char **array);