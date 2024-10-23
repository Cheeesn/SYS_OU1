/*
 * Kurs:5DV088 Systemnäraprogrammering
 *
 * Obligatorisk uppgift 2
 * Author: Jon Sellgren (hed22jsn@cs.umu.se).
 *
 * Version information:
 *   2024-10-03: v1.0.
 *   2024-11-01: v2.0
 */
#include "parser.h"
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>

/**
 * read_makefile() - Parses the makefile and returns a makefile structure.
 * @fp: File pointer to the opened makefile.
 *
 * This function reads the makefile, parses its contents, and creates the
 * internal makefile structure. If parsing fails, it prints an error and exits.
 *
 * Returns: A pointer to the parsed makefile structure, exits on failure.
 */
makefile* read_makefile(FILE *fp);
/**
 * process_target() - Processes a target by recursively building prerequisites.
 * @make: The parsed makefile structure.
 * @target: The target to process.
 * @s_flag: Silent flag, controls command printing.
 * @B_flag: Force-build flag.
 * @f_flag: Custom makefile flag.
 *
 * This function processes a target by first recursively building its prerequisites.
 * Once prerequisites are built, it checks if the target needs to be rebuilt and
 * initiates the build process if necessary.
 *
 * Returns: void.
 */
void process_target(makefile *make, const char* target,  int s_flag, int B_flag, int f_flag);
/**
 * process_targets_from_args() - Processes command line arguments to build specified targets.
 * @make: A pointer to the parsed makefile structure.
 * @argv: The argument vector containing command line arguments.
 * @s_flag: An integer flag that indicates if the command should be executed silently (1) or not (0).
 * @B_flag: An integer flag that indicates whether to force a rebuild of all targets (1) or not (0).
 * @f_flag: An integer flag that indicates if a custom makefile is being used (1) or the default is being used (0).
 *
 * This function processes the targets specified in the command line arguments. 
 * If no target is provided, it defaults to the first target defined in the makefile.
 * The function ensures that all prerequisites of the specified targets are built 
 * recursively by calling the appropriate processing function for each target.
 *
 * Returns: void.
 */
void handle_targets(makefile *make, char *argv[], int s_flag, int B_flag, int f_flag);

/**
 * check_modification_time() - Checks if a target's prerequisites are newer than the target.
 * @target: The name of the target whose modification time needs to be checked.
 * @prereqs: A list of prerequisite file names.
 *
 * This function compares the modification times of the target and its prerequisites.
 * If any prerequisite has a more recent modification time than the target, the target
 * needs to be rebuilt.
 *
 * Returns: 1 if a prerequisite is newer than the target, 0 otherwise.
 */
int check_modification_time(const char *target, const char **prereqs);
/**
 * parse_flags() - Parses command-line flags and opens the specified makefile.
 * @argc: The argument count from the command line.
 * @argv: The argument vector containing command line arguments.
 * @B_flag: Pointer to the force-rebuild flag.
 * @s_flag: Pointer to the silent flag.
 * @f_flag: Pointer to the flag for custom makefile.
 * @fp: Pointer to the file pointer that will hold the makefile stream.
 *
 * This function handles parsing flags from the command line. It sets the flags
 * and opens the correct makefile, either from the command line argument or
 * defaults to "mmakefile".
 *
 * Returns: void, exits on error.
 */
void parse_flags(int argc, char **argv, int *B_flag, int *s_flag, int *f_flag, FILE **fp);
/**
 * open_makefile() - Opens the makefile for reading.
 * @filename: The name of the makefile to open.
 *
 * This function attempts to open the specified makefile for reading. If the file
 * cannot be opened, it prints an error and exits the program.
 *
 * Returns: A file pointer to the opened makefile, exits on failure.
 */
FILE* open_makefile(const char *filename);
/**
 * fork_and_execute() - Forks a process to execute a command.
 * @cmd: The command to be executed, passed as an array of strings.
 *
 * This function forks a new process and attempts to execute the command in the child
 * process using `execvp()`. The parent process continues normally, while the child 
 * replaces its image with the specified command.
 *
 * Returns: void, exits the child process if execution fails.
 */
void fork_and_execute(char **cmd);
/**
 * print_function() - Prints a command to the console.
 * @cmd: The command to be printed, passed as an array of strings.
 *
 * This function prints the command to the console, formatting it as a space-separated
 * string. It is used to print the command before it is executed, in non-silent mode.
 *
 * Returns: void.
 */
void print_function(char **cmd);
/**
 * check_and_build_target() - Checks if a target needs rebuilding and executes the build.
 * @rules: The rule associated with the target.
 * @target: The target to check and build.
 * @B_flag: Force-build flag.
 * @s_flag: Silent flag, controls command printing.
 * @prereqs: A list of prerequisite file names.
 *
 * This function checks if the target should be rebuilt based on the existence of the target,
 * the force flag, or if any prerequisites have been modified. If any of these conditions are
 * met, the target's build command is executed.
 *
 * Returns: void.
 */
void check_and_build_target(rule *rules, const char* target, int B_flag,int s_flag, const char ** prereqs);
/**
 * wait_for_child() - Waits for a child process to complete.
 *
 * This function waits for any child processes that were forked to finish their execution.
 * If the child process exits with a non-zero status, the parent process exits with failure.
 *
 * Returns: void, exits if the child terminates abnormally.
 */
void wait_for_child(void);