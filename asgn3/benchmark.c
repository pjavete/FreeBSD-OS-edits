

int main() {

    for(int i = 0; i < 10000; i++) {
        void *m = malloc(1024*1024);
        memset(m,0,1024*1024);
    }

    return 0;

}