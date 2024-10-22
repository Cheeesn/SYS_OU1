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
        

        pthread_mutex_t mutex;
        sem_t semaphore;

        // Collect file paths and initialize thread data
        ThreadData *thread_data = init_struct();
        thread_data = check_arguments(argv, argc, &threads, thread_data);
        
        initialize_resources(&mutex, &semaphore, threads, thread_data);
        
        // Calculate sizes for all files (multithreaded if threads > 1)
        
        int sizes = thread_handler(threads, thread_data);
        
        print_function(thread_data, sizes);
        if (thread_data->exit_code == 1) {
            exit(EXIT_FAILURE);
        }
        // Cleanup allocated resources
        destroy_struct(thread_data);
        destroy_resources(&mutex, &semaphore);


        
        return 0;
    }
ThreadData *init_struct(void){
    ThreadData *thread_data = malloc(sizeof(ThreadData));
    thread_data->files = NULL;
    thread_data->arguments = NULL;
    thread_data->num_files = 0;
    thread_data->total_files = 0;
    thread_data->sizes = 0;
    thread_data->arguments = 0;
    thread_data->current_index = 0;
    thread_data->exit_code = 0;
    return thread_data;
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
    int thread_size = 0; // Local size for this thread

    while (1) {
        int file_index; // Declare file_index outside the lock

        pthread_mutex_lock(data->mutex);
        
        // Check if all files have been processed
        if (data->current_index >= data->total_files) {
            pthread_mutex_unlock(data->mutex);
            break;
        }

        // Get the next file to process
        file_index = data->current_index++; // Use the current index
        pthread_mutex_unlock(data->mutex);
        
        struct stat path_stat;
        if (lstat(data->files[file_index], &path_stat) != 0) {
            fprintf(stderr, "Warning: Failed to get stats for '%s': ", data->files[file_index]);
            perror("lstat");
            continue;
        }

        // Calculate the size and accumulate it in thread_size
        thread_size += calculate_size(data->files[file_index]);
        
        // If it's a directory, add its files to the queue for future processing
        if (S_ISDIR(path_stat.st_mode)) {
            add_directory_files(data->files[file_index], data);
        }
    }

    // Store the calculated size in the thread_data structure
    pthread_mutex_lock(data->mutex);
    data->sizes[thread_args->thread_index] = thread_size; // Save this thread's size
    pthread_mutex_unlock(data->mutex);

    return NULL;
}


 int thread_handler(int threads, ThreadData *thread_data) {
    int total_size = 0;

    // Allocate memory for sizes for each thread
    thread_data->sizes = malloc(sizeof(int) * threads);
    if (!thread_data->sizes) {
        perror("Failed to allocate memory for sizes");
        exit(EXIT_FAILURE);
    }

    pthread_t thread_ids[threads];
    ThreadArgs thread_args[threads];
    
    for (int i = 0; i < threads; i++) {
        thread_args[i].thread_data = thread_data;
        thread_args[i].thread_index = i; // Set thread index for identification
        thread_args[i].size = &thread_data->sizes[i]; // Pass the pointer to the thread's size

        // Create thread to calculate sizes
        
        if (pthread_create(&thread_ids[i], NULL, thread_calculate_size, &thread_args[i]) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
        
    }

    // Wait for all threads to complete and aggregate sizes
    for (int i = 0; i < threads; i++) {
        if (pthread_join(thread_ids[i], NULL) != 0) {
            perror("Thread join failed");
            exit(EXIT_FAILURE);
        }
        total_size += thread_data->sizes[i];  // Sum up sizes from each thread
    }

    free(thread_data->sizes); // Free the allocated memory for sizes
    return total_size; // Return the total size calculated
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
        for(int i = 0; i < thread_data->num_files;i++){
            free(thread_data->arguments[i]);
        }
        free(thread_data->files);
        free(thread_data->arguments);
        free(thread_data);
        
    }

void add_directory_files(const char *path, ThreadData *thread_data) {
    DIR *dir;
    struct dirent *entry;
    char fullpath[PATH_MAX];

    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "Failed to open directory '%s'\n", path);
        pthread_mutex_lock(thread_data->mutex);
        thread_data->exit_code = 1;
        pthread_mutex_unlock(thread_data->mutex);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        
        pthread_mutex_lock(thread_data->mutex);
        thread_data->files = realloc(thread_data->files, (thread_data->total_files + 1) * sizeof(char *));
        if (!thread_data->files) {
            perror("Failed to allocate memory for files");
            exit(EXIT_FAILURE);
        }
        thread_data->files[thread_data->total_files] = strdup(fullpath); // Duplicate the path
        thread_data->total_files++;
        pthread_mutex_unlock(thread_data->mutex);
    }

    closedir(dir);
}

    ThreadData *check_arguments(char *argv[], int argc, int *threads, ThreadData * thread_data) {
        int opt;
        *threads = 1;

        int total_files = 0;

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
                thread_data->files = realloc(thread_data->files, (total_files +1) * sizeof(char *));
                thread_data->arguments = realloc(thread_data->arguments, (total_files +1) * sizeof(char *));
                if (thread_data->files == NULL) {
                    perror("Failed to reallocate memory");
                    exit(EXIT_FAILURE);
                }

            thread_data->files[total_files] = malloc(strlen(argv[i])+1);
            thread_data->arguments[total_files] = malloc(strlen(argv[i])+1);
            strcpy(thread_data->files[total_files], argv[i]);
            strcpy(thread_data->arguments[total_files], argv[i]);
            
            total_files++;
            
        }
        thread_data->num_files = total_files;
        thread_data->total_files = total_files;
    
        return thread_data;
    }
void print_function(ThreadData *thread_data, int total_size) {
    for (int i = 0; i < thread_data->num_files; i++) {
        printf("%d %s\n", total_size, thread_data->arguments[i]);
    }
}