/*
 * Kurs:5DV088 Systemnäraprogrammering
 *
 * Obligatorisk uppgift 3
 * Author: Jon Sellgren (hed22jsn@cs.umu.se).
 *
 * Version information:
 *   2024-10-22: v1.0.
 */
#include "mdu.h"

int main(int argc, char *argv[]) {
    int threads = 1;
    int exit_code = 0;

    pthread_mutex_t mutex;
    sem_t semaphore;

    // Collect file paths and initialize thread data
    ThreadData *thread_data = collect_file_paths(argc, argv, &threads, &exit_code);
    
    initialize_resources(&mutex, &semaphore, threads, thread_data);

    // Calculate sizes for all files (multithreaded if threads > 1)
    int *sizes = thread_handler(threads, thread_data);
    print_function(thread_data, sizes);
    
    // Cleanup allocated resources
    destroy_struct(thread_data);
    destroy_resources(&mutex, &semaphore);
   
    free(sizes); // Free sizes array

    if (exit_code == 1) {
        exit(EXIT_FAILURE);
    }
    return 0;
}


void initialize_resources(pthread_mutex_t *mutex, sem_t *semaphore, int threads, ThreadData *thread_data) {
    pthread_mutex_init(mutex, NULL);
    sem_init(semaphore, 0, threads); // Initialize semaphore with thread count
    thread_data->mutex = mutex;
    thread_data->semaphore = semaphore;
}


void destroy_resources(pthread_mutex_t *mutex, sem_t *semaphore) {
    pthread_mutex_destroy(mutex);
    sem_destroy(semaphore);
}


void* thread_calculate_size(void* arg) {
    ThreadArgs *thread_args = (ThreadArgs*)arg;
    ThreadData *data = thread_args->thread_data;
    int *sizes = thread_args->size;

    while (1) {
        sem_wait(data->semaphore);  // Wait for a resource to be available
        pthread_mutex_lock(data->mutex); 
        
        // Check if all files have been processed
        if (data->current_index >= data->total_files) {
            pthread_mutex_unlock(data->mutex);
            sem_post(data->semaphore); 
            break; 
        }

        int file_index = data->current_index++; 
        pthread_mutex_unlock(data->mutex); 

        // Calculate size of the file
        sizes[file_index] = calculate_size(data->files[file_index]);
        sem_post(data->semaphore); // Release the semaphore after processing the file
    }

    return NULL;
}

int* thread_handler(int threads, ThreadData *thread_data) {
    int *sizes = malloc(thread_data->total_files * sizeof(int));
    if (!sizes) {
        perror("Failed to allocate memory for sizes");
        exit(EXIT_FAILURE);
    }

    pthread_t thread_ids[threads];
    ThreadArgs thread_args[threads];

    for (int i = 0; i < threads; i++) {
        thread_args[i].thread_data = thread_data;
        thread_args[i].size = sizes;

        // Create thread to calculate sizes
        if (pthread_create(&thread_ids[i], NULL, thread_calculate_size, &thread_args[i]) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < threads; i++) {
        if(pthread_join(thread_ids[i], NULL) != 0){
            perror("Thread join failed");
            exit(EXIT_FAILURE);
        }
    }

    return sizes; 
}


int calculate_size(const char *path) {
    struct stat file_stat;
    if (lstat(path, &file_stat) != 0) {
        fprintf(stderr, "Warning: Failed to get stats for '%s': ", path);
        perror("lstat");
        return 0; // Return 0 size for unreadable files
    }

    return file_stat.st_blocks; // Return size in 512-byte blocks
}


void destroy_struct(ThreadData *thread_data) {
    for (int i = 0; i < thread_data->total_files; i++) {
        free(thread_data->files[i]);
    }
    free(thread_data->files);
    for (int i = 0; i < thread_data->num_files; i++) {
        free(thread_data->arguments[i]);
    }
    free(thread_data->arguments);
    free(thread_data);
}


ThreadData *collect_file_paths(int argc, char *argv[], int *threads, int *exit_code) {
    ThreadData *thread_data = malloc(sizeof(ThreadData));
    thread_data->current_index = 0;
    thread_data->total_files = 0;

    char **files = check_arguments(argv, argc, threads, &thread_data->num_files);
    if (thread_data->num_files == 0) {
        fprintf(stderr, "No files or directories specified.\n");
        *exit_code = 1;
        return thread_data;
    }

    thread_data->arguments = malloc(thread_data->num_files * sizeof(char *));
    if (!thread_data->arguments) {
        perror("Failed to allocate memory for arguments");
        exit(EXIT_FAILURE);
    }

    // Store the file paths for calculation
    thread_data->files = malloc(thread_data->total_files * sizeof(char *));
    if (!thread_data->files) {
        perror("Failed to allocate memory for file paths");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < thread_data->num_files; i++) {
        thread_data->arguments[i] = strdup(files[i]); // Save the original argument
        store_file_paths(files[i], thread_data, exit_code); // Recursively store the file paths
        free(files[i]);
    }
    free(files);
    
    return thread_data;
}


void store_file_paths(const char *path, ThreadData *thread_data, int *exit_code) {
    DIR *dir;
    struct dirent *entry;
    char fullpath[PATH_MAX];

    thread_data->files = realloc(thread_data->files, (thread_data->total_files + 1) * sizeof(char *));
    if (!thread_data->files) {
        perror("Failed to reallocate memory for file paths");
        exit(EXIT_FAILURE);
    }
    thread_data->files[thread_data->total_files] = strdup(path); 
    thread_data->total_files++;

    if ((dir = opendir(path)) == NULL) {
        if (errno == EACCES) {
            fprintf(stderr, "du: cannot read directory '%s': Permission denied\n", path);
            *exit_code = 1;
        }
        return;  // Skip unreadable directories
    }
    
    // Traverse directory contents
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        // If it's a directory, traverse it recursively
        if (entry->d_type == DT_DIR) {
            store_file_paths(fullpath, thread_data, exit_code);
        } else {
            thread_data->files = realloc(thread_data->files, (thread_data->total_files + 1) * sizeof(char *));
            if (!thread_data->files) {
                perror("Failed to reallocate memory for file paths");
                exit(EXIT_FAILURE);
            }
            thread_data->files[thread_data->total_files] = strdup(fullpath);
            thread_data->total_files++;
        }
    }
    closedir(dir);
}


char** check_arguments(char *argv[], int argc, int *threads, int *num_files) {
    int opt;
    *threads = 1;
    int file_array_size = 4;
    int total_files = 0;

    char **files = malloc(file_array_size * sizeof(char *));
    if (!files) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "j:")) != -1) {  
        switch (opt) {
            case 'j':
                *threads = atoi(optarg);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    for (int i = optind; i < argc; i++) {
        if (total_files >= file_array_size) {
            file_array_size *= 2;
            files = realloc(files, file_array_size * sizeof(char *));
            if (!files) {
                perror("Failed to reallocate memory");
                exit(EXIT_FAILURE);
            }
        }

        files[total_files] = malloc((strlen(argv[i]) + 1) * sizeof(char));
        strcpy(files[total_files], argv[i]);
        total_files++;
        *num_files = total_files;
    }
   
    return files;
}


void print_function(ThreadData *thread_data, int *sizes) {
    for (int i = 0; i < thread_data->num_files; i++) {
        int total_size = 0;

        for (int j = 0; j < thread_data->total_files; j++) {
            if (strncmp(thread_data->arguments[i], thread_data->files[j], strlen(thread_data->arguments[i])) == 0) {
                total_size += sizes[j];
            }
        }

        printf("%d %s\n", total_size, thread_data->arguments[i]);
    }
}
