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
#include "mmake.h"

int main(int argc, char *argv[]) {
    makefile *make;
    FILE *fp = NULL;
    int B_flag = 0, s_flag = 0, f_flag = 0;

    // Parse flags and open the file accordingly
    parse_flags(argc, argv, &B_flag, &s_flag, &f_flag, &fp);


    make = read_makefile(fp);
    handle_targets(make, argv, s_flag, B_flag, f_flag);

    // Cleanup
    makefile_del(make);
    fclose(fp);

    return 0;
}
void parse_flags(int argc, char **argv, int *B_flag, int *s_flag, int *f_flag, FILE **fp) {
    char opt;

    while((opt = getopt(argc, argv, "f:Bs")) != -1) {
        switch (opt) {
            case 'f':
                *f_flag = 1;
                *fp = open_makefile(optarg);  // Open file passed via -f
                break;

            case 'B':
                *B_flag = 1;
                break;

            case 's':
                *s_flag = 1;
                break;

            default:
                fprintf(stderr, "Invalid option\n");
                exit(EXIT_FAILURE);
        }
    }

    if(*f_flag == 0) {
        *fp = open_makefile("mmakefile");
    }
}
FILE* open_makefile(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    return fp;
}
makefile* read_makefile(FILE *fp){
    makefile* make;
    make = parse_makefile(fp);

    if(make == NULL){
        fprintf(stderr, "mmakefile: Could not parse makefile\n");
        exit(EXIT_FAILURE);
    }

    return make;
}
void handle_targets(makefile *make,  char *argv[], int s_flag, int B_flag, int f_flag) {
    char * target = argv[optind];
    
    if (target == NULL){//No target specified use default
        process_target(make, makefile_default_target(make), s_flag, B_flag, f_flag);
    }
    else{
        while (target != NULL){
            process_target(make,target, s_flag, B_flag, f_flag);
            optind++;
            target = argv[optind];
        }
    }
}
void process_target(makefile *make, const char* target, int s_flag, int B_flag, int f_flag){
    const char **prereqs;
    rule *rules = makefile_rule(make,target);

    if(rules == NULL){//Checking if there not rules if there is no target error otherwise return
        if (access(target, F_OK) != 0){
            fprintf(stderr, "mmake: No rule to make target '%s'.\n", target);
            exit(EXIT_FAILURE);
        }
        return; 
    }
    prereqs = rule_prereq(rules);
    
    for (int i = 0; prereqs[i] != NULL; i++) {//Looping through every prereq
        process_target(make, prereqs[i], s_flag, B_flag, f_flag);
    }

    check_and_build_target(rules,target,B_flag,s_flag,prereqs);
}
void check_and_build_target(rule *rules, const char* target, int B_flag,int s_flag, const char ** prereqs){
    //Checking force flag, or target doesnt exist/acessable or if it has been modified
    if(access(target, F_OK) == -1 ||B_flag == 1 || check_modification_time(target,prereqs)){
        char **cmd = rule_cmd(rules);
        if(s_flag == 0){//Not silent print out commands 
            print_function(cmd);
        }
        fork_and_execute(cmd);
    }
}
int check_modification_time(const char *target, const char **prereqs) {
    struct stat file_info;
    
    if (stat(target, &file_info) < 0) {
        perror(target);
        exit(EXIT_FAILURE); 
    }
    time_t prereq_time;
    time_t target_time = file_info.st_mtime;
    
    for (int i = 0; prereqs[i] != NULL; i++) {//checking every prereq for modification time
        if (stat(prereqs[i], &file_info) < 0) {
            perror(target);
            exit(EXIT_FAILURE);
        }
        prereq_time = file_info.st_mtime;
        
        if (prereq_time > target_time) {//If the prereqs were modified later than the target rebuild
            return 1;
        }
    }

    return 0;
}
void fork_and_execute(char **cmd){
    pid_t pid;

    if((pid = fork()) == -1){
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    if(pid == 0){//Check if child
        if(execvp(*cmd, cmd) < 0){
            perror(*cmd);
            exit(EXIT_FAILURE);
        }   
    }
}
void wait_for_child(void){
    int status;

    if(wait(&status) < 0){
        perror("wait");
        exit(EXIT_FAILURE);
    }
    if(status != 0){
        exit(EXIT_FAILURE);
    }
}
void print_function(char **cmd){
    for(int i = 0; cmd[i] != NULL;i++){
            if(i != 0){
                printf(" ");
            }
            printf("%s", cmd[i]);
        }
    printf("\n");
}