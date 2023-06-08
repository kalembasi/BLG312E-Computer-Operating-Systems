#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void processTree(int depth) {
    
    int left, right, curr_pid;
    int status; // to hold the exit status of child processes
    
    if (depth == 0){
        exit(0);
    }

    if(depth == 1){ //avoid to create right child at final depth
        left = fork();
       
        if(left == 0){  //if left child pid
            exit(0);    //return parent
        }

        curr_pid = getpid();

        printf("parent: %d -> child: %d\n", curr_pid, left);
        
        fflush(stdout); //flush the output buffer

        waitpid(left, &status, 0); //wait for the left child to complete
    } 
    
    if(depth > 1){  //if depht is not final, create left and right child
        
        left = fork();  //left child
        
        if(left == 0){
            exit(0);    //return parent
        }

        right = fork(); //right child

        if(right == 0){
            processTree(depth - 1); //call recursive function with depth-1
            exit(0);    //return parent
        }

        curr_pid = getpid();

        printf("parent: %d -> child: %d\n", curr_pid, left);
        printf("parent: %d -> child: %d\n", curr_pid, right);
        
        fflush(stdout); //flush the output buffer

        waitpid(left, &status, 0); //wait for the left child to complete
        waitpid(right, &status, 0); //wait for the right child to complete
    }
}

int main(){
    
    int N=0;
    printf("enter depth N: ");
    scanf("%d", &N);

    processTree(N+1);

    return EXIT_SUCCESS;
}
