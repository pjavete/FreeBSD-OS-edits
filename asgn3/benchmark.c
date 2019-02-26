#include <stdlib.h>

int main() {

	void *m = malloc(1024*1024*1024);
    for(int i = 0; i < 10000; i++) {
    	printf("******LOOP %d********\n", i);
        int r = rand();
        
    }
    free(m);
    return 0;

}