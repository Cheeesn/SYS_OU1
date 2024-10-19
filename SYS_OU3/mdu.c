#include "mdu.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>



int main(int argc, char *argv[]) {
    int threads = 0;
    int num_files = 0;
    int exit_code = 0;

    // Create local mutex and semaphore variables
    pthread_mutex_t mutex;
    sem_t semaphore;
	ThreadData *thread_data = malloc(sizeof(ThreadData));
    // Collect file paths
    char **files = collect_file_paths(argc, argv, &num_files, &threads, &exit_code,thread_data);
    if (exit_code == 1) {
        cleanup_resources(files, num_files);
        exit(EXIT_FAILURE);
    }

    
    initialize_resources(&mutex, &semaphore, threads);
    
    // Calculate sizes for all files (multithreaded if threads > 1)
    int *sizes = calculate_sizes(files, num_files, threads, thread_data,&mutex,&semaphore);
	
    // Cleanup allocated resources
    free(sizes);
    cleanup_resources(files, num_files);

    // Destroy mutex and semaphore
    destroy_resources(&mutex, &semaphore);
    
    return 0;
}

// Initialize mutex and semaphore
void initialize_resources(pthread_mutex_t *mutex, sem_t *semaphore, int threads) {
    pthread_mutex_init(mutex, NULL);
    sem_init(semaphore, 0, threads); // Initialize semaphore with thread count
}

// Destroy mutex and semaphore
void destroy_resources(pthread_mutex_t *mutex, sem_t *semaphore) {
    pthread_mutex_destroy(mutex);
    sem_destroy(semaphore);
}

// Thread function to calculate the size of files
void* thread_calculate_size(void* arg) {
    ThreadData *data = (ThreadData*)arg;
	
    while (1) {
        // Lock the mutex to check and increment the index safely
		fprintf(stderr,"%d %d\n", data->current_index,data->total_files);
        pthread_mutex_lock(data->mutex);
		
        // Check if there are any unprocessed files left
        if (data->current_index >= data->total_files) {
            pthread_mutex_unlock(data->mutex);
			
            break; // Exit the loop if all files are processed
        }
		data->current_index++;
        // Get the current index to process
        int index = data->current_index; // Store the current index
        
        pthread_mutex_unlock(data->mutex); // Unlock after accessing the index

        // Wait for a free slot before calculating size
        sem_wait(data->semaphore);
        
        // Get size of the file
        int size = calculate_size(data->files[index]);
		
        // Lock before updating the shared array
        pthread_mutex_lock(data->mutex);
        data->sizes[index] = size; // Store the calculated size
        pthread_mutex_unlock(data->mutex);
		
        // Signal that this slot is free
        sem_post(data->semaphore);
    }

    return NULL;
}


// Calculate sizes for each file (multithreaded or single-threaded based on `threads`)
int *calculate_sizes(char **files, int num_files, int threads, ThreadData *thread_data,pthread_mutex_t *mutex, sem_t *semaphore) {
    int *sizes = malloc(num_files * sizeof(int));
    if (!sizes) {
        perror("Failed to allocate memory for sizes");
        exit(EXIT_FAILURE);
    }

    

    if (threads > 1) {
        pthread_t thread_ids[threads];
        
        thread_data->files = files;
        thread_data->sizes = sizes;
        thread_data->mutex = mutex;      // Assign mutex to thread data
        thread_data->semaphore = semaphore; // Assign semaphore to thread data
        thread_data->current_index = 0; // Start index for files
        thread_data->num_files = num_files; // Total number of files
		
        for (int i = 0; i < threads; i++) {
            // Create thread to calculate sizes
			
            if (pthread_create(&thread_ids[i], NULL, *thread_calculate_size, thread_data) != 0) {
                perror("Failed to create thread");
                exit(EXIT_FAILURE);
            }
			
			
        }

        // Wait for all threads to complete
        for (int i = 0; i < threads; i++) {
            pthread_join(thread_ids[i], NULL);
        }
    } 
    else {
        // Single-threaded calculation
        for (int i = 0; i < num_files; i++) {
            sizes[i] = calculate_size(files[i]);
        }
    }

    return sizes;
}

// Calculate the size of a single file
int calculate_size(const char *path) {
    struct stat file_stat;
    if (lstat(path, &file_stat) != 0) {
        fprintf(stderr, "Warning: Failed to get stats for '%s': ", path);
        perror("lstat");
        return 0; // Return 0 size for unreadable files
    }
	
    return file_stat.st_blocks; // Return size in 512-byte blocks
}

// Free the dynamically allocated file paths
void cleanup_resources(char **files, int num_files) {
    for (int i = 0; i < num_files; i++) {
        free(files[i]);
    }
    free(files);
}

// Collect file paths from command-line arguments or directories
char **collect_file_paths(int argc, char *argv[], int *num_files, int *threads, int *exit_code,ThreadData *thread_data) {
    char **files = check_arguments(argv, argc, threads, num_files);
    if (*num_files == 0) {
        fprintf(stderr, "No files or directories specified.\n");
        *exit_code = 1;
        return NULL;
    }

    // Initialize array to hold all files
    char **all_files = malloc(*num_files * sizeof(char *));
    if (!all_files) {
        perror("Failed to allocate memory for file paths");
        exit(EXIT_FAILURE);
    }

    // Store the files/directories provided in command-line arguments
    for (int i = 0; i < *num_files; i++) {
        all_files[i] = strdup(files[i]); // Duplicate the strings to avoid modifying the original array
    }

    // Accumulate the total size for each file or directory
    for (int i = 0; i < *num_files; i++) {
        int total_size = 0;  // Initialize total size for this file argument
        store_file_paths(files[i], &all_files, num_files, exit_code, &total_size,thread_data);
        // Print the total size for this file argument
        printf("%d\t%s\n", total_size, files[i]);
        if (*exit_code == 1) {
            break;
        }
    }

    cleanup_resources(files, *num_files); // Free the original files array
    return all_files; // Return the newly populated files
}

// Store file paths and accumulate total size
void store_file_paths(const char *path, char ***files, int *num_files, int *exit_code, int *total_size,ThreadData *thread_data) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char fullpath[PATH_MAX];

    // Get the stats for the file or directory
    if (lstat(path, &file_stat) != 0) {
        // Print a warning and continue
        fprintf(stderr, "Warning: Failed to get file stats for '%s': ", path);
        perror("lstat");
        return;  // Return early to not process this file or directory further
    }
	
    // Add the size of the current file or directory to the total size
    *total_size += file_stat.st_blocks; // Size in 512-byte blocks

    // If it's a directory, traverse it recursively
    if (S_ISDIR(file_stat.st_mode)) {
        if ((dir = opendir(path)) == NULL) {
            if (errno == EACCES) {
                *exit_code = 1;
                fprintf(stderr, "du: cannot read directory '%s': Permission denied\n", path);
            }
            return;  // Skip unreadable directories
        }

        // Traverse directory contents
        while ((entry = readdir(dir)) != NULL) {
            // Skip "." and ".." entries
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
			
            // Construct the full path for each entry
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
			thread_data->total_files++;
            // Recursively accumulate the size for nested files/directories
            store_file_paths(fullpath, files, num_files, exit_code, total_size,thread_data);
        }

        closedir(dir);
    }
}

// Check command-line arguments for valid inputs
char** check_arguments(char *argv[], int argc, int *threads, int *num_files) {
    int opt;
    *threads = 1;
    int file_array_size = 4;
    
    char **files = malloc(file_array_size * sizeof(char *));
    if (!files) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "j:")) != -1) {  // Checking for flags
        switch (opt) {
            case 'j':
                *threads = atoi(optarg);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        if (*num_files >= file_array_size) {
            file_array_size *= 2;
            files = realloc(files, file_array_size * sizeof(char *));
            if (!files) {
                perror("Failed to reallocate memory");
                exit(EXIT_FAILURE);
            }
        }
		 
        files[*num_files] = malloc((strlen(argv[i]) + 1) * sizeof(char));
        strcpy(files[*num_files], argv[i]);
        (*num_files)++;
    }
    return files;
}
