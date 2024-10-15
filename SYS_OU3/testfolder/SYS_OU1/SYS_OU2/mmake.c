#include "mmake.h"
/*
 * Kurs:5DV088 Systemnäraprogrammering
 *
 * Obligatorisk uppgift 2
 * Author: Jon Sellgren (hed22jsn@cs.umu.se).
 *
 * Version information:
 *   2024-10-03: v1.0.
 */
int main(int argc, char *argv[]){
    makefile* make;
    FILE *fp = NULL;
    int B_flag = 0;
    int s_flag = 0;
    int f_flag = 0;
    char opt;
    while((opt = getopt(argc,argv,"f:Bs")) != -1){//Checking what flags are present
        switch(opt){
            case 'f':
                f_flag = 1; 
                if((fp = fopen(optarg,"r")) == NULL){
                    perror(optarg);
                    exit(EXIT_FAILURE);
                }
            break;

            case 'B':
                B_flag = 1;
            break;

            case 's':
                s_flag  = 1;
            break;

            default:
            fprintf(stderr,"No flags set");
            exit(EXIT_FAILURE);
        }
        
    }
    if(f_flag == 0){
        if((fp = fopen("mmakefile","r")) == NULL){//No file provided use mmakefile
            perror(optarg);
            exit(EXIT_FAILURE);
        }
    }
    
    make = read_makefile(fp);
    build_prerequisites(make,argv,s_flag,B_flag, f_flag);
    makefile_del(make);
    fclose(fp);
    return 0;
}

makefile* read_makefile(FILE *fp){
    makefile* make;
    make = parse_makefile(fp);
    if(make == NULL){
        perror("mmakefile");
        exit(EXIT_FAILURE);
    }

    return make;
}
void build_prerequisites(makefile *make,  char *argv[], int s_flag, int B_flag, int f_flag) {

    char * target = argv[optind];
    
    if (target == NULL){//No target specified use default
        build_target(make, makefile_default_target(make), s_flag, B_flag, f_flag);
    }
    else{
        while (target != NULL){
            build_target(make,target, s_flag, B_flag, f_flag);
            optind++;
            target = argv[optind];
        }
    }

}

void build_target(makefile *make, const char* target, int s_flag, int B_flag, int f_flag){
    const char **prereqs;
    rule *rules = makefile_rule(make,target);
    if(rules == NULL){//Checking if there not rules if there is no target error otherwise return
        if (access(target, F_OK) != 0){
            perror("mmake");
            exit(EXIT_FAILURE);
        }
    return; 
    }
    prereqs = rule_prereq(rules);
    
    for (int i = 0; prereqs[i] != NULL; i++) {//Looping through every prereq
        build_target(make, prereqs[i], s_flag, B_flag, f_flag);
    }
    //Checking force flag, or target doesnt exist/acessable or if it has been modified
    if(access(target, F_OK) == -1 ||B_flag == 1 || check_modification_time(target,prereqs)){
        char **cmd = rule_cmd(rules);
        pid_t pid;
        
        if(s_flag == 0){//Not silent print out commands 
            
            for(int i = 0; cmd[i] != NULL;i++){
                if(i != 0){
                    printf(" ");
                }
                printf("%s", cmd[i]);
            }
        printf("\n");
        }

        if((pid = fork()) == -1){
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
            if(execvp(*cmd, cmd) < 0){
                perror(*cmd);
                exit(EXIT_FAILURE);
            }   
            
        }
        //waiting for children
        int wait_check;
        wait(&wait_check);
        if (wait_check > 0){
            exit(EXIT_FAILURE);
        }
    }

}
int check_modification_time(const char *target, const char **prereqs) {
    struct stat file_info;
    
    if (stat(target, &file_info) < 0) {//checking if stat returns info correctly
       
        perror(target);
        exit(EXIT_FAILURE); 
    }
    time_t prereq_time;
    time_t target_time = file_info.st_mtime;
    
    for (int i = 0; prereqs[i] != NULL; i++) {//checking every prereq for modification time
        if (stat(prereqs[i], &file_info) < 0) {
        }
        prereq_time = file_info.st_mtime;
        
        if (prereq_time > target_time) {//If the prereqs were modified later than the target rebuild
            return 1;
        }
    }

    return 0;
}