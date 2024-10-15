#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
    int error;
    char *args[] = {"./test", NULL};
    error = execv("./test",args);
    if(error == -1) {
        perror("Bad file");
    }
return 0;
}