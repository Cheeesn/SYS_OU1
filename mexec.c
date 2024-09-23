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

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    int command_lines = 0;
    int command_args = 0;
    char **command_buffer = malloc(sizeof(char*)*Maximum_allowed_char);
    
    if(command_buffer == NULL){
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    if(argc == 1) {
        fp = stdin;
    }
    else {
        fp = fopen(argv[1] ,"r");
    }
    if(argc > 2){

        fprintf(stderr,"Too many arguments");
        exit(EXIT_FAILURE);
    }
    
    if(fp == NULL){
        fprintf(stderr,"Could not open file\n");
        perror("File opening error:");
        exit(EXIT_FAILURE);
    }
    
    command_lines = reading_command_lines(fp, command_buffer, &command_args);
    
    create_children(fp,command_buffer,command_lines,&command_args);
    
    kill_function_arr(&command_args, command_buffer);
    
    
    if(fp != stdin){
        fclose(fp);
        fp = NULL;
    }
    
    return 0;
}
/**
 * reading_command_lines() - Reads lines from a file and stores command tokens.
 * @fp: A pointer to the file (or stdin) to read the command lines from.
 * @command_buffer: A double pointer to store the commands and arguments.
 * @command_args: A pointer to an integer that stores the total number of arguments.
 *
 * Returns: The total number of command lines read from the input.
 */
int reading_command_lines(FILE *fp, char **command_buffer,int* command_args) {
    
    char buffer[Maximum_allowed_char];
    int command_lines = 0;
    int i = 0;
    command_buffer[i] = NULL;
    while(fgets(buffer, Maximum_allowed_char, fp) != NULL){//Read each line
        buffer[strcspn(buffer, "\n")] = 0;//removes newline
        
        char *token = strtok(buffer," "); 
        command_buffer[i] = malloc(strlen(buffer));
        while(token != NULL){
            
            command_buffer[i] = realloc(command_buffer[i],strlen(token)+1);
            strcpy(command_buffer[i], token);
            
            
            
            i++;
            command_buffer[i] = NULL;
            token = strtok(NULL," ");
        }
        *command_buffer = realloc(*command_buffer,sizeof(char*)*i);
        
        
        i++;
        command_lines++;
        
        *command_args = i;
    }
    
    
    return command_lines;
}
/**
 * kill_function_arr() - Frees memory allocated for command buffer.
 * @amount_of_index: A pointer to the total number of arguments in the command buffer.
 * @command_buffer: A double pointer that holds the command and argument strings.
 *
 * Returns: Nothing. Frees allocated memory for each command in the buffer.
 */
void kill_function_arr(int* amount_of_index, char **command_buffer){
    for(int i = 0; *amount_of_index>i ;i++){
        free(command_buffer[i]);
    }
    
    free(command_buffer);
    return;
}
/**
 * create_children() - Forks child processes and sets up inter-process communication.
 * @fp: A pointer to the input file (or stdin).
 * @command_buffer: A double pointer that stores the commands and their arguments.
 * @command_lines: The total number of command lines.
 * @command_args: A pointer to the total number of arguments across all commands.
 *
 * Returns: Nothing. Creates child processes, pipes, and manages their execution.
 */
void create_children(FILE *fp,char **command_buffer, int command_lines,int* command_args){
    int fd[command_lines-1][2];
    int nullfound = 0;
    int command_index = 0;
    pid_t pid[command_lines];
    int pipeamount = command_lines-1;
    
    for(int i = 0; i < pipeamount; i++){
        
        if(pipe(fd[i]) == -1){
            perror("Pipe failed");
            exit(EXIT_FAILURE);
        }
    }
    
    for(int i = 0; i < command_lines; i++){
        if((pid[i] = fork()) == -1){
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        if(pid[i] == 0){
            
            nullfound = 0;
            command_index = 0;
            
            while(nullfound != i){
                if(command_buffer[command_index] == NULL){
                    nullfound++;
                }
                command_index++;
            }
            
            child_process(fp,command_buffer,command_lines,fd, command_index,i,command_args);
            return;
        }
        
    }
    for (int i = 0; i < command_lines - 1; i++)//Parent closing pipes
    {
        close(fd[i][0]);
        close(fd[i][1]);
    }
    for (int i = 0; i < command_lines; i++) {
        int status;
        waitpid(pid[i], &status, 0);
        if(status > 0){
            kill_function_arr(command_args,command_buffer);
        
            fclose(fp);
            exit(EXIT_FAILURE);
        }
    }
    
   return;
}
/**
 * child_process() - Executes the command for each child and sets up pipes.
 * @fp: A pointer to the input file (or stdin).
 * @command_buffer: A double pointer that holds the commands and their arguments.
 * @command_lines: The total number of command lines.
 * @fd: A 2D array representing pipes between processes.
 * @current_index: The index of the command to be executed by the child process.
 * @childnr: The current child process number in the chain.
 * @command_args: A pointer to the total number of arguments across all commands.
 *
 * Returns: Nothing. Executes the command using execvp() and redirects input/output as needed.
 */
void child_process(FILE *fp,char **command_buffer, int command_lines,int fd[][2],int current_index,int childnr,int* command_args){
    int pipeamount = command_lines-1;
    char *execbuff[Maximum_allowed_char];
    int j = 0;
    int i = current_index;
    
    while(command_buffer[i] != NULL){
        execbuff[j] = command_buffer[i];
        i++;
        j++;
    }
    execbuff[j] = NULL;
    
    if(command_lines > 1){
        if(childnr == 0){//Parent to child
            
            if(dup2(fd[childnr][1],STDOUT_FILENO) < 0){
                
                close(fd[childnr][1]);
                exit(EXIT_FAILURE);
            }
            
            
        }
        else if(childnr == pipeamount){//Child to stdout
            
            if(dup2(fd[childnr-1][0],STDIN_FILENO) < 0){
                
                close(fd[childnr-1][1]);
                exit(EXIT_FAILURE);
            }
            
        }

        else{//Child to child
            if(dup2(fd[childnr-1][0],STDIN_FILENO) < 0){
                
                close(fd[childnr-1][0]);
                exit(EXIT_FAILURE);
            }
            
            
            if(dup2(fd[childnr][1],STDOUT_FILENO) < 0){
                
                close(fd[childnr][1]);
                exit(EXIT_FAILURE);
            }
            
            
        }
    }
    
    for (int i = 0; i < command_lines - 1; i++)//Children closing pipes
    {
        close(fd[i][0]);
        close(fd[i][1]);
    }
    
    
    if(execvp(execbuff[0], execbuff) < 0){
        
        perror(execbuff[0]);
        kill_function_arr(command_args,command_buffer);
        
        fclose(fp);
        exit(EXIT_FAILURE);
        
    }
    
}
