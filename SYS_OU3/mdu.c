#include "mdu.h"

int main(int argc, char * argv[]){
	int threads;
	char** files;
	int num_files = 0;
	int size_of_file;
	int exit_code = 0;
	files = check_arguments(argv,argc, &threads,&num_files);

	
	for(int i = 0; i < num_files;i++){
		size_of_file = traverse_directory(files[i],&exit_code);
		printf("%d	%s\n", size_of_file,files[i]);
		if(exit_code == 1){
			kill_function_arr(num_files,files);
			exit(EXIT_FAILURE);
		}
	}
	
	kill_function_arr(num_files,files);
	return 0;
}


char** check_arguments(char *argv[], int argc,int* threads, int* num_files){
	int opt;
	*threads = 1;
	int file_array_size = 4; 
	
	char **files = malloc(file_array_size * sizeof(char *)); 

	while((opt = getopt(argc,argv,"j:")) != -1){//Checking what flags are present
		switch(opt){
			case 'j':
				*threads = atoi(optarg);
			break;

			default:
			exit(EXIT_FAILURE);
		}
		
	}
		for(int i = optind; i < argc; i++){
			if(*num_files >= file_array_size){
				file_array_size *= 2;
				files = realloc(files, file_array_size * sizeof(char *));
			}
			
			
			files[*num_files] = malloc((strlen(argv[i]) + 1));
			strcpy(files[*num_files], argv[i]);
			(*num_files)++;
		}
		return files;
}

int traverse_directory(const char *path,int * exit_code){
	DIR *dir;
	struct dirent *entry;
	struct stat file_stat;
	char fullpath[PATH_MAX];
	int total_size = 0;
	
	if(lstat(path,&file_stat) != 0){
		perror("Failed to get file stats");
		exit(EXIT_FAILURE);
	}
	if((dir = opendir(path)) == NULL){
		if(errno == EACCES){
			*exit_code = 1;
			fprintf(stderr, "du: cannot read directory '%s': Permission denied\n", path);
		}
		return total_size += (file_stat.st_blocks);
		
	}
	total_size += (file_stat.st_blocks);
	while((entry = readdir(dir)) != NULL){
		
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		strcpy(fullpath,path);
		strcat(fullpath,"/");
		strcat(fullpath,entry->d_name);
		
		if(lstat(fullpath,&file_stat) != 0){
			perror("Failed to get file stats");
			continue;
		}
		
		
		if(S_ISDIR(file_stat.st_mode)){
			total_size += traverse_directory(fullpath,exit_code);
		}
		else {
			total_size += (file_stat.st_blocks);
		}

	}
	
	closedir(dir);
	
	return total_size;
}
void kill_function_arr(int amount_of_index, char **array){
	
	for(int i = 0; amount_of_index > i ;i++){
		free(array[i]);
	}
	free(array);
	
	return;
}