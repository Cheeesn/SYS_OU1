#include "parser.h"
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>

/**
 * read_makefile() - Reads and parses the makefile.
 * @fp: File pointer to the opened makefile.
 *
 * This function reads the makefile from the given file pointer and parses it into a
 * structure for further processing. It exits the program if the makefile cannot be read.
 *
 * Returns: A pointer to the parsed makefile structure.
 */
makefile* read_makefile(FILE *fp);
/**
 * build_target() - Builds a single target based on its rules and prerequisites.
 * @make: Pointer to the parsed makefile structure.
 * @target: The name of the target to build.
 * @s_flag: Integer flag indicating whether silent mode (-s) is enabled.
 * @B_flag: Integer flag indicating whether to force rebuild (-B).
 * @f_flag: Integer flag indicating whether a custom makefile was provided (-f).
 *
 * This function recursively builds a target by first ensuring that all its prerequisites
 * are up to date. It compares the modification times of the target and its prerequisites,
 * and executes the necessary commands if a rebuild is required.
 *
 * Returns: None.
 */
void build_target(makefile *make, const char* target,  int s_flag, int B_flag, int f_flag);
/**
 * build_prerequisites() - Builds the prerequisites for a given target.
 * @make: Pointer to the parsed makefile structure.
 * @argv: Command-line arguments array, including targets to build.
 * @s_flag: Integer flag indicating whether silent mode (-s) is enabled.
 * @B_flag: Integer flag indicating whether to force rebuild (-B).
 * @f_flag: Integer flag indicating whether a custom makefile was provided (-f).
 *
 * This function builds the prerequisites for the specified target(s) by recursively
 * processing them. If no targets are specified, it builds the default target.
 *
 * Returns: None.
 */
void build_prerequisites(makefile *make, char *argv[], int s_flag, int B_flag, int f_flag);

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
void parse_flags(int argc, char **argv, int *B_flag, int *s_flag, int *f_flag, FILE **fp);
FILE* open_makefile(const char *filename);