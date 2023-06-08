//omer malik kalembasi
//150180112

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void leftTree(int depth) {

    int pid;
    int curr_pid;
    int parent_pid;
    int status;

    if(depth == 1){ //M-1 step is enough
        exit(0);
    }

    if(depth > 0){

        pid = fork();

        if(pid == 0){   //left child

            curr_pid = getpid();
            parent_pid = getppid();

            printf("parent: %d -> child: %d\n", parent_pid, curr_pid);
            leftTree(depth - 1);    //call leftTree func with N-1 depth recursively
            exit(0);
        }
        else if(pid > 0){ //parent process
            wait(&status); //wait for child process to terminate
        }
        else{
            perror("fork"); //fork error check
            exit(EXIT_FAILURE);
        }
    }
}

void processTree(int depth, int M) {

    int left;
    int right;
    int curr_pid;
    int status;

    if(depth == 0){
        exit(0);
    }

    if(depth > 0){  //create both left and right children

        left = fork();

        if(left == 0){  //left child

            leftTree(M);    //call subtree function
            exit(0);    //terminate subtree
        }
        else if(left > 0){ //parent process

            if(depth == 1){

                curr_pid = getpid();

                printf("parent: %d -> child: %d\n", curr_pid, left);
                wait(&status); //wait for left child process to terminate
                exit(0);    //return parent
            }
            
            right = fork();

            if(right == 0){ //right child
                processTree(depth - 1, M);  ////call recursive function with depth-1
                exit(0);    //return parent
            }
            else if(right > 0){ //parent process

                curr_pid = getpid();

                printf("parent: %d -> child: %d\n", curr_pid, left);
                printf("parent: %d -> child: %d\n", curr_pid, right);

                wait(&status); // wait for left child process to terminate
                wait(&status); // wait for right child process to terminate
                exit(0);
            }
            else{   //check fork error
                perror("fork");
                exit(EXIT_FAILURE);
            }
        }
    }
}


int main(){
    
    int N, M;

    printf("enter depth N: ");
    scanf("%d", &N);
    printf("enter subtree depth M: ");
    scanf("%d", &M);

    processTree(N+1, M);

    return EXIT_SUCCESS;
}