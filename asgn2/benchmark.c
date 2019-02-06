void benchmark1(){
    int pid;
    pid = fork();
    if(pid < 0){
        perror("Error: fork did not complete");
		exit(0);
    }else if(pid == 0){

    }else{
        wait();
    }
}
//we can have it write to file
void benchmark2(){
    int pid;
    pid = fork();
    if(pid < 0){
        perror("Error: fork did not complete");
		exit(0);
    }else if(pid == 0){

    }else{
        wait();
    }
}

int main(){
    benchmark1();
    benchmark2();
}