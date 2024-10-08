
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define Maximum_allowed_char 1024

int reading_command_lines(FILE *fp, char **command_buffer, int* command_args);
void temp_print_function(char** command_arguments, int number_of_args);
void kill_function_arr(int* amount_of_index, char **command_buffer);
void create_children(FILE *fp,char **command_buffer, int command_lines,int* command_args);
void child_process(FILE *fp,char **command_buffer, int command_lines, int fd[][2],int current_index, int childnr,int* command_args);
int tokenize_command(char **command_buffer, char *read_buffer, int index);
void close_pipes(int pipeamount, int fd[][2]);