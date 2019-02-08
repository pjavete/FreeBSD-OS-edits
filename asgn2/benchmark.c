#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>

int main(int argc, char *argv[]){

    if (argv[1] == NULL){
        printf("Usage: benchmark <nice>\n");
        exit(0);
    }

    int nice = atoi(argv[1]);
    if (nice >= -20 && nice <= 19){
        setpriority(PRIO_PROCESS, 0, nice);
    } else {
        printf("Error: Nice value out of range\n");
        exit(0);
    }
    
    for(int i = 0; i < 1000000; i++){
        // :)
    }

}