#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

typedef struct {
    char **files;        // Array of file paths
    int total_files;     // Total number of files
    int current_index;   // Current index for file processing
    int num_files;       // Number of command-line files
    char **arguments;    // Original command-line arguments
    pthread_mutex_t *mutex; 
    sem_t *semaphore;    
    int *sizes;
    int exit_code;          // Sizes of files
} ThreadData;
typedef struct {
    ThreadData *thread_data;
    int thread_index;
    int *size; 
} ThreadArgs;

/**
 * initialize_resources() - Initializes mutex and semaphore for multithreading.
 * @mutex: A pointer to the mutex to be initialized.
 * @semaphore: A pointer to the semaphore to be initialized.
 * @threads: The number of threads to be used.
 * @thread_data: A pointer to the ThreadData structure holding thread-related data.
 *
 * Returns: None.
 */
void add_directory_files(const char *path, ThreadData *thread_data);

void initialize_resources(pthread_mutex_t *mutex, sem_t *semaphore, int threads, ThreadData *thread_data);
ThreadData *init_struct(void);
/**
 * destroy_resources() - Destroys mutex and semaphore resources.
 * @mutex: A pointer to the mutex to be destroyed.
 * @semaphore: A pointer to the semaphore to be destroyed.
 *
 * Returns: None.
 */
void destroy_resources(pthread_mutex_t *mutex, sem_t *semaphore);

/**
 * thread_calculate_size() - Thread function that calculates the size of files.
 * @arg: A pointer to the ThreadArgs structure containing thread-specific data.
 *
 * Returns: NULL.
 */
void* thread_calculate_size(void* arg);

/**
 * thread_handler() - Creates threads 
 * @threads: The number of threads to use for calculation.
 * @thread_data: A pointer to the ThreadData structure holding file information.
 *
 * Returns: A pointer to an array of integers containing sizes of files.
 */
int thread_handler(int threads, ThreadData *thread_data);

/**
 * calculate_size() - Calculates the size of a single file.
 * @path: A pointer to the string containing the file path.
 *
 * Returns: The size of the file in 512-byte blocks. Returns 0 if the file is unreadable.
 */
int calculate_size(const char *path);

/**
 * destroy_struct() - Frees dynamically allocated resources in ThreadData.
 * @thread_data: A pointer to the ThreadData structure to clean up.
 *
 * Returns: None.
 */
void destroy_struct(ThreadData *thread_data);

/**
 * collect_file_paths() - Collects file paths from command-line arguments or directories.
 * @argc: The number of command-line arguments.
 * @argv: The command-line arguments array.
 * @threads: A pointer to store the number of threads.
 * @exit_code: A pointer to store the exit code in case of errors.
 *
 * Returns: A pointer to a ThreadData structure containing file paths and metadata.
 */
ThreadData *collect_file_paths(int argc, char *argv[], int *threads, int *exit_code,ThreadData * thread_data) ;

/**
 * store_file_paths() - Recursively stores file paths in the ThreadData structure.
 * @path: A pointer to the string containing the directory path to be traversed.
 * @thread_data: A pointer to the ThreadData structure to store file paths.
 * @exit_code: A pointer to store the exit code in case of errors.
 *
 * Returns: None.
 */
void store_file_paths(const char *path, ThreadData *thread_data, int *exit_code);

/**
 * check_arguments() - Validates command-line arguments and returns file paths.
 * @argv: The command-line arguments array.
 * @argc: The number of command-line arguments.
 * @threads: A pointer to store the number of threads.
 * @num_files: A pointer to store the total number of files specified.
 *
 * Returns: A pointer to an array of strings containing valid file paths.
 */
ThreadData *check_arguments(char *argv[], int argc, int *threads, ThreadData * thread_data);

/**
 * print_function() - Prints the total size of files for each argument.
 * @thread_data: A pointer to the ThreadData structure containing file information.
 * @sizes: A pointer to an array of integers representing file sizes.
 *
 * Returns: None.
 */
void print_function(ThreadData *thread_data, int total_size);