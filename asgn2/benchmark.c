#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>

int main(int argc, char *argv[]){

    int nice = atoi(argv[1]);
    if (nice >= -20 && nice <= 19){
        setpriority(PRIO_PROCESS, 0, nice);
    }
    
    for(int i = 0; i < 1000000; i++){
        // :)
    }

}