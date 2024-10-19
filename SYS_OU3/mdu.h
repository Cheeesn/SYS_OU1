#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>

typedef struct {
    char **files;              // Array of file paths
    int *sizes;               // Array to store calculated sizes
    pthread_mutex_t *mutex;   // Pointer to the mutex for shared access
    sem_t *semaphore;         // Pointer to the semaphore for limiting concurrent access
    int num_files;            // Total number of files to be processed
    int current_index;        
    int total_files;      
} ThreadData;

// Function prototypes
void* thread_calculate_size(void* arg);
void initialize_resources(pthread_mutex_t *mutex, sem_t *semaphore, int threads);
void destroy_resources(pthread_mutex_t *mutex, sem_t *semaphore);
int *calculate_sizes(char **files, int num_files, int threads, ThreadData *thread_data,pthread_mutex_t *mutex, sem_t *semaphore);
int calculate_size(const char *path);
void cleanup_resources(char **files, int num_files);
char **collect_file_paths(int argc, char *argv[], int *num_files, int *threads, int *exit_code,ThreadData *thread_data);
void store_file_paths(const char *path, char ***files, int *num_files, int *exit_code, int *total_size,ThreadData *thread_data);
char** check_arguments(char *argv[], int argc, int *threads, int *num_files);