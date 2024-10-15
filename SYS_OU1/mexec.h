
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define Maximum_allowed_char 1024

/**
 * reading_command_lines() - Reads commands from a file or stdin, tokenizes them, and stores them in a buffer.
 * @fp: A pointer to the file or stdin for reading the command lines.
 * @command_buffer: A double pointer that holds the commands and their arguments.
 * @command_args: A pointer to an integer that tracks the total number of command arguments.
 *
 * Returns: A double pointer containing the commands and their arguments.
 * The total number of commands read is stored in command_args.
 */
char **reading_command_lines(FILE *fp, char **command_buffer, int *command_args);

/**
 * kill_function_arr() - Frees allocated memory used for storing commands and arguments.
 * @amount_of_index: The total number of arguments in the command buffer.
 * @command_buffer: A double pointer to the command buffer that stores command arguments.
 *
 * Returns: Nothing. Frees memory allocated for each command and the command buffer.
 */
void kill_function_arr(int amount_of_index, char **command_buffer);

/**
 * create_children() - Forks child processes to execute commands and sets up piping between them.
 * @fp: A pointer to the input file or stdin for command reading.
 * @command_buffer: A double pointer containing the commands and their arguments.
 * @command_lines: The total number of command lines read.
 * @command_args: The total number of arguments stored in command_buffer.
 *
 * Returns: Nothing. Creates child processes, pipes for inter-process communication,
 * and manages process execution.
 */
void create_children(FILE *fp, char **command_buffer, int command_lines, int command_args);

/**
 * child_process() - Executes a specific command as a child process, handling input/output redirection.
 * @fp: A pointer to the input file or stdin.
 * @command_buffer: A double pointer storing commands and arguments.
 * @command_lines: The total number of command lines read.
 * @fd: An array of file descriptors for piping between processes.
 * @current_index: The index of the current command in the command buffer.
 * @childnr: The current child process number (for piping purposes).
 * @command_args: The total number of arguments across all commands.
 *
 * Returns: Nothing. Executes the command for the current child process with necessary input/output redirection.
 */
void child_process(FILE *fp, char **command_buffer, int command_lines, int fd[][2], int current_index, int childnr, int command_args);

/**
 * tokenize_command() - Splits a command line into tokens and stores them in the command buffer.
 * @command_buffer: A double pointer used to store tokens (commands and arguments).
 * @read_buffer: A string containing the command line to be tokenized.
 * @index: A pointer to an integer tracking the current position in the command buffer.
 *
 * Returns: A double pointer containing the tokenized command and updated buffer.
 * The index is updated with the number of arguments added.
 */
char **tokenize_command(char **command_buffer, char *read_buffer, int *index);

/**
 * close_pipes() - Closes all file descriptors used for pipes.
 * @pipeamount: The number of pipes to be closed.
 * @fd: An array of file descriptors representing the pipes between processes.
 *
 * Returns: Nothing. Closes all open pipes used for inter-process communication.
 */
void close_pipes(int pipeamount, int fd[][2]);

/**
 * create_pipes() - Creates pipes for inter-process communication.
 * @command_lines: The total number of commands, used to determine how many pipes are needed.
 * @fd: An array of file descriptors for piping between processes.
 *
 * Returns: Nothing. Initializes pipes based on the number of command lines.
 */
void create_pipes(int command_lines, int fd[][2]);