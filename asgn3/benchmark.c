int main() {

    for(int i = 0; i < 5000; i++) {
        void *m = malloc(1024*1024);
        memset(m,0,1024*1024);
    }

    return 0;

}
//malloc outside for loop 
//and randomly access the memory
//randomly generate a number within the range of the malloc and call a read/write on that address