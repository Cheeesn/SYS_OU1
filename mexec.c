#include "mexec.h"
/*
 * Kurs:5DV088 Systemnäraprogrammering
 *
 * Obligatorisk uppgift 1
 * Author: Jon Sellgren (hed22jsn@cs.umu.se).
 *
 * Version information:
 *   2024-09-23: v1.0.
 */
int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    int command_lines = 0;
    int command_args = 0;
    char **command_buffer = malloc(sizeof(char*)*Maximum_allowed_char);
    
    if(command_buffer == NULL){
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    
    if(argc == 1) {//Checking if its stdin input or a file
        fp = stdin;
    }
    
    else {
        fp = fopen(argv[1] ,"r");
    }
    
    if(argc > 2){//Checking if its the right amount of arguments
        fprintf(stderr,"Too many arguments");
        exit(EXIT_FAILURE);
    }
    
    if(fp == NULL){//Checking if file is readable
        
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }
    
    command_lines = reading_command_lines(fp, command_buffer, &command_args);
    create_children(fp,command_buffer,command_lines,&command_args);
    kill_function_arr(&command_args, command_buffer);
    
    if(fp != stdin){//closing file if its not stdin
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
    
    char buffer[Maximum_allowed_char];//Buffer for reading the file
    int command_lines = 0;
    int i = 0;
    command_buffer[command_lines] = NULL;

    while(fgets(buffer, Maximum_allowed_char, fp) != NULL){
        
        buffer[strcspn(buffer, "\n")] = 0;//removes newline
        command_buffer[i] = malloc(strlen(buffer));//Allocating command_buffer
        if(command_buffer[i] == NULL){
            perror("Memory allocation error");
            exit(EXIT_FAILURE);
        }
        
        i = tokenize_command(command_buffer,buffer,i);
        
        
        command_lines++;//counts the amount of commands
        
        *command_args = i;//Returning argument amount for returning memory later
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
        free(command_buffer[i]);//freeing every index
    }
    free(command_buffer);//freeing the whole array
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
    
    for(int i = 0; i < pipeamount; i++){//Creating pipes equal to the amount of commands-1
        
        if(pipe(fd[i]) == -1){
            perror("Pipe failed");
            exit(EXIT_FAILURE);
        }
    }
    
    for(int i = 0; i < command_lines; i++){//Starting fork process
        if((pid[i] = fork()) == -1){
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        if(pid[i] == 0){//Making sure its a parent
            
            nullfound = 0;
            command_index = 0;
            
            while(nullfound != i){//Looking for NULL to find the start of each command 
                if(command_buffer[command_index] == NULL){
                    nullfound++;
                }
                command_index++;
            }
            
            child_process(fp,command_buffer,command_lines,fd, command_index,i,command_args);//Starting the child process
            return;
        }
        
    }

     close_pipes(pipeamount,fd);

    for (int i = 0; i < command_lines; i++) {//Waiting for children to finish
        int status;
        if(waitpid(pid[i], &status, 0) < 0){
            perror("");
            exit(EXIT_FAILURE);
        }
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
    
    while(command_buffer[i] != NULL){//Storing the command and arguments to exec later
        execbuff[j] = command_buffer[i];
        i++;
        j++;
    }
    execbuff[j] = NULL;
    
    if(command_lines > 1){
        if(childnr == 0){
            
            if(dup2(fd[childnr][1],STDOUT_FILENO) < 0){//Redirecting stdout to the current pipes write
                close(fd[childnr][1]);
                exit(EXIT_FAILURE);
            }
            
        }
        else if(childnr == pipeamount){

            if(dup2(fd[childnr-1][0],STDIN_FILENO) < 0){//Redirecting in stream to previous pipes read and checking if it fails
                close(fd[childnr-1][0]);
                exit(EXIT_FAILURE);
            }
        }

        else{
            
            if(dup2(fd[childnr-1][0],STDIN_FILENO) < 0){//Redirecting in stream to previous pipes read and checking if it fails
                close(fd[childnr-1][0]);
                exit(EXIT_FAILURE);
            }
            if(dup2(fd[childnr][1],STDOUT_FILENO) < 0){//Redirecting out stream to current pipes write and checking if it fails
                close(fd[childnr][1]);
                exit(EXIT_FAILURE);
            }
        }
    }
    
    close_pipes(pipeamount,fd);
    
    if(execvp(execbuff[0], execbuff) < 0){
        //Cleanup in case of faulty command
        perror(execbuff[0]);
        kill_function_arr(command_args,command_buffer);
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    
}
int tokenize_command(char **command_buffer, char *read_buffer, int index){
    
    char *token = strtok(read_buffer," "); 
    int i = index; 
    
    while(token != NULL){
            command_buffer = realloc(command_buffer,sizeof(char *) * (i + 1024));
            command_buffer[i] = realloc(command_buffer[i],strlen(token)+1);//Reallocating the buffer so it matches the size + NULL
            if(command_buffer[i] == NULL){
                perror("Memory allocation error");
                exit(EXIT_FAILURE);
            }
            strcpy(command_buffer[i], token);//Storing tokenb
            i++;
            command_buffer[i] = NULL;//NULL at the end of the command
            token = strtok(NULL," ");
        }
        
        i++;//Extra count because of added NULL
        

    return i;
}
void close_pipes(int pipeamount, int fd[][2]){

    for (int i = 0; i < pipeamount; i++){//Children closing pipes that are not in use
        close(fd[i][0]);
        close(fd[i][1]);
    }
}